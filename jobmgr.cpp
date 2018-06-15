#include "jobmgr.h"

#include <iostream>
#include <future>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <memory>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/mpi/intercommunicator.hpp>

namespace mpi = boost::mpi;
using namespace std;

size_t                     n_processes_per_spawn;
size_t                     n_concurrent_job_starter;
size_t                     max_concurrent_jobs;
boost::filesystem::path    mapper_cmd("valgrind mapper");
vector<string>             margv;

namespace {

/// helper to create c-style argument vector
unique_ptr<char const*[]> make_argv(vector<string> const& argv)
{
	unique_ptr<char const*[]> tmp { new const char*[argv.size() + 1] };

	tmp[argv.size()] = nullptr;
	for (size_t ii = 0; ii < argv.size(); ++ii) {
		tmp[ii] = argv[ii].data();
	}

	return tmp;
}

}

// class: JobManager
JobManager& JobManager::instance()
{
	static JobManager _instance;
	return _instance;
}


JobManager::~JobManager()
{
	printf("deleting JobManager singleton\n");
}


job_handle_t JobManager::createJob(ObjectStore&& os)
{
	job_handle_t const handle = handle_cnt++;

	// move object store into job
	auto job = std::make_shared<Job>(handle, std::move(os));

	// get exclusive access
	boost::unique_lock<boost::shared_mutex> lock(mRWLock);
	auto it = jobs().insert(job);
	lock.unlock();

	if (!it.second)
		throw runtime_error("couldn't create job");

	// set job state to UNINITIALIZED -> NEW
	elevate(handle);

	return handle;
}


Job const& JobManager::getJob(job_handle_t const handle)
{
	handle_iterator it;

	{
		// get shared read access
		boost::shared_lock<boost::shared_mutex> lock(mRWLock);

		it = jobs().find(handle);
	}

	if (it != jobs().end())
		return **it;

	throw runtime_error("couldn't find job");
}


void JobManager::modify_state(
	function<void (job_set_type::value_type&)> const f,
	job_handle_t const job)
{
	auto it = jobs().find(job);
	if (it == jobs().end())
		throw std::runtime_error("Job not found");

	jobs().modify(it, f);
}


bool JobManager::job_max_reached() const
{
	// no lock -> approx number (could be anything ;))
	return jobs().get<state_t>().count(RUNNING) > max_concurrent_jobs;
}

void JobManager::spawn(Job const& job)
{
	// append ester supplied argv to mapper argv
	vector<string> argvv(margv);
	for (auto const& arg : job.objectstore().argv())
		argvv.push_back(arg);

	// build const char** compatible argv
	auto argv = make_argv(argvv);

	// MPI on debian is not thread-safe and therefore needs protection
	// see: $ ompi_info | grep -i thread
	//lock_guard<mutex> lock(mSpawnLock);

	{
		// actually spawn new mpi environment
		mpi::intercommunicator comm(
			MPI::COMM_SELF.Spawn(
				mapper_cmd.string().data(),
				argv.get(),
				n_processes_per_spawn,
				MPI::INFO_NULL,
				0),
			mpi::comm_take_ownership);

		job.comm() = std::move(comm);
	}

	// set errorhandler
	MPI_Comm_set_errhandler(job.comm(), MPI::ERRORS_THROW_EXCEPTIONS);

	job.comm().send(MASTER_PROCESS, msg_data_package, job.objectstore());

	// async request results from mapping
	job.req() = job.comm().irecv(
		MPI::ANY_SOURCE,
		msg_finished_mapping,
		job.ret());
}


JobManager::JobManager()
{
	size_t n_concurrent_job_finisher = 1;

	for (auto ii = 0u; ii < n_concurrent_job_finisher; ++ii)
		mJobFinisher.emplace_back(thread(bind(&JobManager::find_finished_jobs, ref(*this))));

	for (auto ii = 0u; ii < n_concurrent_job_starter; ++ii)
		mJobStarter.emplace_back(thread(bind(&JobManager::start_new_jobs, ref(*this))));

	printf("starting JobManager (threads starter: %lu finisher : %lu)\n",
		   n_concurrent_job_starter,
		   n_concurrent_job_finisher);
}


void JobManager::start_new_jobs()
{
	auto& state_index = jobs().get<state_t>();
	state_iterator it;

	while (true) {

		boost::this_thread::sleep(boost::posix_time::milliseconds(20));

		{
			// get shared read access
			boost::upgrade_lock<boost::shared_mutex> lock(mRWLock);

			if ((it = state_index.find(NEW)) == state_index.end())
				continue;

			// get exclusive access
			boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

			// job is set to RUNNING state
			elevate((*it)->handle());
		}

		Job const& job = **it;

		printf("found job %d\n", job.handle());

		// try to spawn the mpi environments
		try {

			// block if approx number jobs in pipeline is above thresh
			if (job_max_reached()) {

				unique_lock<mutex> lock(mMaxNumberOfJobsLock);

				// check for spurious wakeups
				while (job_max_reached())
					mMaxNumberOfJobsCond.wait(lock);
			}

			// start spawn in it's own thread
			async(launch::async, std::bind(&JobManager::spawn, ref(*this), ref(job)));
			//async(launch::async, std::bind(&JobManager::spawn, ref(*this), placeholders::_1), job);
			//spawn(job);

		} catch (MPI::Exception& e) {
			printf("job starter caught: %s\n", e.Get_error_string());

			// get exclusive access
			boost::unique_lock<boost::shared_mutex> uniqueLock(mRWLock);

			set_state_to(job, ERROR);
			continue;
		}
	} // while(true)
}


void JobManager::find_finished_jobs()
{
	auto& state_index = jobs().get<state_t>();
	state_iterator it;

	while (true) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(40));

		{
			// get shared read access
			boost::upgrade_lock<boost::shared_mutex> lock(mRWLock);

			// continue if no running experiment has been finished finished
			if ((it = state_index.find(RUNNING)) == state_index.end()
				|| !(*it)->req()
				|| !(*it)->req()->test())
				continue;

			// do blocking I/O on response from mapping


			// get exclusive access
			boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

			// set Job state to FINISHED
			elevate(**it);
		}

		printf("ester: job %d returned %d\n", (*it)->handle(), (*it)->ret().error);

		// patch objectstore
		// TODO: copying could be avoided by deserialization to the right location.
		(*it)->objectstore() = *(*it)->ret().store;

		// eleveate Job state to COMPLETED
		elevate(**it);

		mMaxNumberOfJobsCond.notify_one();
	}
}


// static members
atomic<job_handle_t> JobManager::handle_cnt {0};

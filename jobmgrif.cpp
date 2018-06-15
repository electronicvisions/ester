#include "jobmgrif.h"

#include <iostream>

#include "jobmgr.h"

job_handle_t JobManagerInterface::createJob(ObjectStore os)
{
	std::cout << "got experiment from user with " << os.size()
	          << " PyNN commands." << std::endl;
	// create a new job
	JobManager& jm = JobManager::instance();

	// figure out cmd line
	job_handle_t job = jm.createJob(std::move(os));
	return job;
}

ObjectStore JobManagerInterface::getJob(job_handle_t const job)
{
	JobManager& jm = JobManager::instance();

	// TODO: add some timeout
	while (true) {
		Job const& j = jm.getJob(job);

		if (j.state() == COMPLETED)
			return j.objectstore();

		boost::this_thread::sleep(boost::posix_time::milliseconds(40));
	}
	return jm.getJob(job).objectstore();
}

state_t JobManagerInterface::state(job_handle_t handle)
{
	JobManager& jm = JobManager::instance();
	Job const& job = jm.getJob(handle);
	return job.state();
}

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <functional>
#include <condition_variable>
#include <memory>
#include <type_traits>

#include <boost/filesystem.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>

#include "job.h"
#include "euter/objectstore.h"

extern std::size_t                n_processes_per_spawn;
extern std::size_t                n_concurrent_job_starter;
extern std::size_t                max_concurrent_jobs;
extern boost::filesystem::path    mapper_cmd;
extern std::vector<std::string>   margv;

template<class Class, typename Type, Type (Class::*PtrToMemberFunction)()const>
struct uptr_const_mem_fun
{
	typedef typename std::remove_reference<Type>::type result_type;

	Type operator()(const Class& x) const
	{
		return (x.*PtrToMemberFunction)();
	}

	Type operator()(const std::shared_ptr<const Class>& x) const
	{
		return operator()(*x);
	}

	Type operator()(const std::shared_ptr<Class>& x) const
	{
		return operator()(*x);
	}
};


typedef boost::multi_index_container<
		std::shared_ptr<Job>,
		boost::multi_index::indexed_by<
			boost::multi_index::ordered_unique<
				boost::multi_index::tag<job_handle_t>,
				uptr_const_mem_fun<Job, job_handle_t, &Job::handle>>,
			boost::multi_index::ordered_non_unique<
				boost::multi_index::tag<state_t>,
				boost::multi_index::composite_key<
					Job,
					uptr_const_mem_fun<Job, state_t, &Job::state>,
					uptr_const_mem_fun<Job, job_handle_t, &Job::handle>
				>
			>
		>
	> job_set_type;

typedef typename job_set_type::index<job_handle_t>::type::iterator  handle_iterator;
typedef typename job_set_type::index<state_t>::type::iterator       state_iterator;


// MPI settings
class JobManager
{
public:
	// get singleton instance
	static JobManager& instance();

	JobManager(JobManager const&)             = delete;
	JobManager& operator= (JobManager const&) = delete;
	~JobManager();

	job_handle_t createJob(ObjectStore&& os);

	Job const& getJob(job_handle_t handle);

private:
	//! not thread safe
	void modify_state(
		std::function<void (job_set_type::value_type&)> f,
		job_handle_t job);

	//! not thread safe
	void elevate(job_handle_t job);

	//! not thread safe
	void set_state_to(
		job_handle_t job,
		state_t state);

	bool job_max_reached() const;

	void spawn(Job const& job);

	job_set_type&       jobs();
	job_set_type const& jobs() const;

	JobManager(); //! available singleton constructor

	void start_new_jobs();
	void find_finished_jobs();

	// members:

	static std::atomic<job_handle_t>  handle_cnt;

	//std::mutex                mSpawnLock; //! protects from concurrent calls to MPI::Spawn

	job_set_type              mJobs;
	boost::shared_mutex       mRWLock;    //! Reader/Writer lock for accessing jobs

	std::mutex                mMaxNumberOfJobsLock;
	std::condition_variable   mMaxNumberOfJobsCond;

	std::vector<std::thread> mJobFinisher;
	std::vector<std::thread> mJobStarter;
};



// implementaions

inline
job_set_type& JobManager::jobs()
{
	return mJobs;
}

inline
job_set_type const& JobManager::jobs() const
{
	return mJobs;
}

inline
void JobManager::elevate(job_handle_t const job)
{
	modify_state(
		[](job_set_type::value_type& j) {
			j->state() = state_t(j->state() + 1);
		}, job);
}


inline
void JobManager::set_state_to(job_handle_t const job,
							  state_t const state)
{
	modify_state(
		[state](job_set_type::value_type& j) {
			j->state() = state;
		}, job);
}

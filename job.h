#pragma once

#include <chrono>

#include "euter/objectstore.h"
#include "typedefs.h"
#include "mappingresult.h"
#include "mpi/config.h"

#include <boost/mpi/request.hpp>
#include <boost/mpi/intercommunicator.hpp>

class Job
{
public:
	// some typedefs
	typedef MappingResult return_type;

	struct MPIData;

	static char const*    state_name[];

	Job(job_handle_t handle, euter::ObjectStore && os);
	Job(Job const&) = delete;
	Job(Job&&)      = delete;
	Job()           = delete;
	~Job();

	euter::ObjectStore& objectstore() const;

	state_t&         state();
	state_t          state() const;
	job_handle_t     handle() const;

	/// @brief cast operator Job -> job_handle_t
	operator job_handle_t () const;

	/// @brief return reference to mpi intercomm used to communicate with the mapping
	///        process
	boost::mpi::intercommunicator&  comm() const;

	/// @brief asynchronous mpi request for mapping results
	boost::optional<boost::mpi::request>& req() const;

	/// @brief access to mapping result
	return_type&     ret() const;

private:
	// members:

	mutable euter::ObjectStore             mObjectStore;
	job_handle_t const                     mHandle;
	state_t                                mState;
	mutable boost::mpi::intercommunicator  mComm;
	mutable boost::optional<
		boost::mpi::request>               mReq;
	mutable return_type                    mRetval; //!< return value from mapping job

	std::chrono::time_point<std::chrono::system_clock> mSubmissionTime;
};


// implementation

inline
state_t& Job::state()
{
	return mState;
}

inline
state_t Job::state() const
{
	return mState;
}

inline
job_handle_t Job::handle() const
{
	return mHandle;
}

inline
Job::operator job_handle_t () const
{
	return mHandle;
}

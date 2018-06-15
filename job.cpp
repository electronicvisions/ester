#include "job.h"

namespace mpi = boost::mpi;

char const* Job::state_name[] = {
	"UNINITIALIZED",
	"NEW",
	"RUNNING",
	"FINISHED",
	"COMPLETED",
	"ERROR"
};


Job::Job(job_handle_t handle, ObjectStore && os) :
	mObjectStore(std::move(os)),
    mHandle(handle),
	mState(UNINITIALIZED),
	mComm(MPI::COMM_NULL, mpi::comm_take_ownership),
	mReq(),
	mRetval()
{}


Job::~Job()
{
	// TODO: is called right now because of copy construction of job into
	// multi_index container
	assert(!objectstore().size());

	printf("deleting job\n");
}


ObjectStore& Job::objectstore() const
{
	return mObjectStore;
}


mpi::intercommunicator& Job::comm() const
{
	return mComm;
}


boost::optional<mpi::request>& Job::req() const
{
	return mReq;
}


typename Job::return_type& Job::ret() const
{
	return mRetval;
}

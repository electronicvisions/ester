#pragma once

#include "typedefs.h"
#include "settings.h"

#include "euter/objectstore.h"

#include <RCF/Idl.hpp>
#include <RCF/RcfServer.hpp>
#include <RCF/TcpEndpoint.hpp>
#include <RCF/FilterService.hpp>
#include <RCF/ZlibCompressionFilter.hpp>
#include <RCF/SessionObjectFactoryService.hpp>

RCF_BEGIN(I_JobManagerInterface, "I_JobManagerInterface")
    RCF_METHOD_R1(job_handle_t, createJob, ObjectStore)
    RCF_METHOD_R1(ObjectStore, getJob, job_handle_t)
    RCF_METHOD_R1(state_t, state, job_handle_t)
RCF_END(I_Experiment)

struct JobManagerInterface
{
public:
	job_handle_t createJob(ObjectStore os);
	ObjectStore getJob(job_handle_t handle);
	// TODO das ist keine Gute idee, weil der Objectstore shared pointer hält...
	state_t state(job_handle_t handle);
};

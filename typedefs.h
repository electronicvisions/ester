#pragma once

#include <cstring>

enum state_t {
	UNINITIALIZED, /* Newly created Job, but not yet marked for execution */
	NEW,           /* Initialized Job, waiting for exection */
	RUNNING,       /* Job currently running (mapping or actual experiment) */
	FINISHED,      /* Experiment complete, but data not yet been sent to user */
	COMPLETED,     /* Experiment complete and data sent to user */
	ERROR          /* Job is in an error state */
};

std::size_t const NUM_JOB_STATES = ERROR + 1u;

typedef unsigned int job_handle_t;

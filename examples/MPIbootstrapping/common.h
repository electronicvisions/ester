#pragma once

#include <mpi/mpi.h>
#include <cstdio>

#define LOCAL_PROCESSES_TO_SPAWN 2

enum msg_tags { msg_intercomm, msg_slave_terminate, msg_bootstrap };

void bootstrap_others(MPI::Intracomm& cur_global, int clients_left, char* port);

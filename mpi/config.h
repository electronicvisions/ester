#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <mpi.h>
#pragma GCC diagnostic pop

#define MASTER_PROCESS 0

enum message_tags {
        msg_start_mapping,
		msg_finished_mapping,
        msg_data_package,
        msg_broadcast_data,
        msg_finished
};

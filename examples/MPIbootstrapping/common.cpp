#include "common.h"

void bootstrap_others(MPI::Intracomm& cur_global, int clients_left, char* port) {
	while (clients_left>0) {
		printf("accepting [global rank: %d; global size: %d]\n", cur_global.Get_rank(), cur_global.Get_size());
		MPI::Intercomm iec = cur_global.Accept(port, MPI::INFO_NULL, 0);
		cur_global = iec.Merge(0);

		--clients_left;
		if (cur_global.Get_rank() == 0) {
			iec.Send(&clients_left, 1, MPI::INT, 0, msg_bootstrap);
		}
	}
}

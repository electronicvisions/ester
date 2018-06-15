#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <sstream>
#include <ctime>

#include "common.h"

using namespace std;

void proc_name() {
	char proc_name[MPI::MAX_PROCESSOR_NAME];
	int len;
	MPI::Get_processor_name(proc_name, len);
	printf("%s, %d",proc_name, len);
}


int main(int argc,char *argv[]) {
	MPI::Init(argc, argv);
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	int rank = MPI::COMM_WORLD.Get_rank();

	// bootstrap MPI
	MPI::Intercomm ic = MPI::COMM_WORLD.Connect(argv[1], MPI::INFO_NULL, 0);
	MPI::Intracomm global = ic.Merge(1);

	// connecting and merging remaining clients
	int clients_left;
	ic.Recv(&clients_left, 1, MPI::INT, 0, msg_bootstrap);
	bootstrap_others(global, clients_left, "");

	//MPI::COMM_WORLD = global;
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	rank = MPI::COMM_WORLD.Get_rank();
	int global_rank = global.Get_rank();

	// debug
	cout << global.Get_size() << " " << global.Get_rank() << endl;

	MPI::Intracomm clients = global.Split(global.Get_rank()>0 ? 0 : 1, 0);
	clients.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	cout << clients.Get_size() << " " << clients.Get_rank() << endl;

	//global.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	try {
		//MPI::Intercomm intcomm = global.Spawn("spawn", MPI::ARGV_NULL, LOCAL_PROCESSES_TO_SPAWN, MPI_INFO_NULL, 0);
		MPI::Intercomm intcomm = clients.Spawn("spawn", MPI::ARGV_NULL, LOCAL_PROCESSES_TO_SPAWN, MPI_INFO_NULL, 0);
	} catch (...) {
		printf("caught error\n");
	}


	global.Barrier();
	MPI::Finalize();
	printf("[rank %d] end client\n", rank);
	return EXIT_SUCCESS;
}

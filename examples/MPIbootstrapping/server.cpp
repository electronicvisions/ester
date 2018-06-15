#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <sstream>
#include <ctime>

#include "common.h"

using namespace std;

char* port = new char[MPI::MAX_PORT_NAME];

int main(int argc,char *argv[]) {
	int NUM_CLIENTS = atoi(argv[1]);

	MPI::Init(argc, argv);
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	int rank = MPI::COMM_WORLD.Get_rank();

	// bootstrap MPI
	MPI::Open_port(MPI::INFO_NULL, port);
	cout << port << endl;

	MPI::Intracomm global(MPI::COMM_WORLD);
	bootstrap_others(global, NUM_CLIENTS, port);

	MPI::Close_port(port);

	// debug
	cout << global.Get_size() << " " << global.Get_rank() << endl;

	MPI::Intracomm clients = global.Split(global.Get_rank() ? 0 : 1, 0);

	global.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	//try {
		//MPI::Intercomm intcomm = global.Spawn("spawn", MPI::ARGV_NULL, LOCAL_PROCESSES_TO_SPAWN, MPI_INFO_NULL, 0);
	//} catch(...) {}


	sleep(1);
	global.Barrier();
	MPI::Finalize();
	printf("[rank %d] end server\n", rank);
	return EXIT_SUCCESS;
}

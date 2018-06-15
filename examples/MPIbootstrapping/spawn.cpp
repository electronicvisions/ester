#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <sstream>
#include <ctime>
#include <signal.h>

#include "common.h"

using namespace std;

MPI::Intercomm parent;

void err_seg() {
	int a = *static_cast<int*>(NULL);
}

void err_float() {
	float a = 42./0;
}


void sig_handler(int sig) {
	if (!MPI::Is_finalized()) {
		MPI::Finalize();
	}
	printf("singnal occured!!!\n");
	exit(-1);
}


int main(int argc,char *argv[]) {
	MPI::Init(argc, argv);
	MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);
	signal(SIGSEGV, sig_handler);
	int rank = MPI::COMM_WORLD.Get_rank();

	parent = MPI::COMM_WORLD.Get_parent();
	parent.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);

	err_seg();

	printf("[rank %d] end spawned process\n", rank);
	MPI::Finalize();
	return EXIT_SUCCESS;
}

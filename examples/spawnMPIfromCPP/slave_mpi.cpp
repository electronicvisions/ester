#include <cstdio>
#include <boost/mpi.hpp>
#include <boost/mpi/intercommunicator.hpp>
#include <iostream>
#include <cstdlib>
#include <exception>

using namespace std;
using namespace boost;

enum msg_tags { msg_intercomm, msg_slave_terminate };

extern int idx;

void err_nullptr() {
	int a = *static_cast<int*>(NULL);
}

void err_div0() {
	float a = 1/0;
}

#include <sys/time.h>
#include <cmath>
#include <sys/resource.h>
void err_bus() { // doesn't work
	struct rlimit rl;
	rl.rlim_cur = 1;
	rl.rlim_max = 1;
	setrlimit(RLIMIT_AS, &rl);
	int* b = static_cast<int*>(malloc(pow(1024,20) * sizeof(int)));
}

int main(int argc,char *argv[]) {
	//mpi::environment evn(argc,argv,false); // don't use, broken!
	MPI_Init(&argc, &argv);

	mpi::communicator world;

	MPI_Comm parent;
	MPI_Comm_set_errhandler(parent, MPI_ERRORS_RETURN);
	MPI_Comm_get_parent(&parent);
	mpi::intercommunicator incomm(parent, mpi::comm_duplicate);

	if(parent == MPI_COMM_NULL){
		throw std::exception();
	} else {
		printf("[rank %d] slave%d running\n", incomm.rank(), idx);
		if(world.rank() == 0){
			int a;
			incomm.recv(0,msg_intercomm,a);
			printf("[rank %d] slave%d received a=%d\n", incomm.rank(), idx, a);

			//cout << incomm.local_size() <<" "<< incomm.local_rank() << " " << incomm.remote_size() << endl;

			a = 23;
			incomm.send(0, msg_slave_terminate, a);
		}
	}
	err_div0();
	world.barrier();
	printf("[rank %d] end slave%d\n", world.rank(), idx);
	return 0;
}

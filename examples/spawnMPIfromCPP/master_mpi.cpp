#include <cstdio>
#include <boost/mpi.hpp>
#include <boost/mpi/intercommunicator.hpp>
#include <iostream>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <sstream>
#include <ctime>
using namespace boost;
using namespace std;

enum msg_tags { msg_intercomm, msg_slave_terminate };

#define NUM_SLAVES 2
#define PROCESSES_PER_SLAVE 5
#define ROOT_RANK 0

int main(int argc,char *argv[]) {
	//mpi::environment evn(argc,argv,false); // don't use, broken!
	MPI_Init(&argc,&argv);
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

	mpi::communicator world;

	try{
		if(world.rank() == ROOT_RANK){
			vector<mpi::intercommunicator> intcomm;
			vector<MPI_Comm>::iterator iter;

			for (int i=0; i<NUM_SLAVES; ++i) {
				static int cnt = 0;
				char fname[100];
				sprintf(fname, "slave%d\0", cnt);

				MPI_Comm everyone;

				// actually spawn the mpi environments
				MPI_Comm_spawn(fname, MPI_ARGV_NULL, PROCESSES_PER_SLAVE,
						MPI_INFO_NULL, ROOT_RANK, MPI_COMM_SELF, &everyone,
						MPI_ERRCODES_IGNORE);

				int a = 42;
				intcomm.push_back(mpi::intercommunicator(everyone, mpi::comm_duplicate));

				intcomm[cnt].isend(ROOT_RANK, msg_intercomm, a);
				//cout << intcomm[cnt].local_size() <<" "<< intcomm[cnt].remote_size() << endl;
				++cnt;
			}

			// wait for slaves to finish their business
			for (vector<mpi::intercommunicator>::iterator itin = intcomm.begin();
					itin < intcomm.end();
					++itin) {
				int a;
				itin->recv(0, msg_slave_terminate, a);
				printf("%d\n", a);
			}
		}
		world.barrier();
	} catch (...) {
		cout << "cought error" << endl;
	}

	//struct timespec ts, tt;
	//ts.tv_sec = 1;
	//nanosleep(&ts, &tt);
	sleep(1);

	printf("[rank %d] end master\n", world.rank());
	return EXIT_SUCCESS;
}

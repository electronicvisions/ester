#include <iostream>
#include <mpi/mpi.h>
#include <parmetis.h>

using namespace std;

template<typename T>
void printPartitioning(int rank, T* ad, int size) {
	printf("[rank %d] partitioning:", rank);
	for (int i=0; i<size; ++i) {
		printf(" %d",ad[i]);
	}
	printf("\n");
}

template<typename T>
void fill_array(T source, T target, int size) {
	for (int i=0; i<size; ++i) {
		target[i] = source[i];
	}
}

int main(int argc, char *argv[])
{
	MPI::Init(argc, argv);
	int rank = MPI::COMM_WORLD.Get_rank();
	int size = MPI::COMM_WORLD.Get_size();
	if (size!=3) {
		printf("start this programm with precisely 3 MPI processes\n");
		MPI::COMM_WORLD.Abort(-1);
	}

	idx_t* xadj       = new idx_t[100];    // vertices: distributed CSR
	idx_t* adjncy     = new idx_t[100];    // adjacency structure: distributed CSR
	idx_t vtxdist[]   = {0,5,10,15};       // distribution of nodes among cpus

	switch (rank) {
		case 0:
			{
				idx_t temp[] = {1,5,0,2,6,1,3,7,2,4,8,3,9,0,6};
				fill_array(temp, adjncy, 15);
			}{
				idx_t temp[] = {0,2,5,8,11,13};
				fill_array(temp,xadj, 6);
			}
			break;

		case 1:
			{
				idx_t temp[] = {0,6,10,1,5,7,11,2,6,8,12,3,7,9,13,4,8,14};
				fill_array(temp, adjncy, 18);
			}{
				idx_t temp[] = {0,3,7,11,15,18};
				fill_array(temp,xadj, 6);
			}
			break;

		case 2:
			{
				idx_t temp[] = {5,11,6,10,12,7,11,13,8,12,14,9,13};
				fill_array(temp, adjncy, 13);
			}{
				idx_t temp[] = {0,2,5,8,11,13};
				fill_array(temp,xadj, 6);
			}
			break;
	}


	idx_t wgtflag      = 0;        // whether edge and/or vertex weights
	idx_t* vwgt        = NULL;     // vertex weight
	idx_t* adjwgt      = NULL;     // edge weight
	idx_t numflag      = 0;        // always for c-style arrays
	idx_t ncon         = 1;        // number of constraints	
	idx_t nparts       = 3;        // number of partitions to partition the graph in
	real_t tpwgts[]    = {1./3, 1./3, 1./3};
	real_t ubvec       = 1.05;     // allowed imbalance measure
	idx_t options[3];              // options
	options[0]         = 0;        // use default options

	idx_t edgecut;                 // stores edge-cut
	idx_t* part        = (idx_t*)new idx_t[5]; // number of local vertices

	MPI_Comm comm = MPI_COMM_WORLD;

	int ret = ParMETIS_V3_PartKway(
			vtxdist,
			xadj,
			adjncy,
			vwgt, adjwgt, &wgtflag,
			&numflag,
			&ncon,
			&nparts,
			tpwgts,
			&ubvec,
			options,
			&edgecut,
			part,
			&comm);

	switch (ret) {
		case METIS_OK:
			break;
		case METIS_ERROR_INPUT:
		case METIS_ERROR_MEMORY:
		case METIS_ERROR:
			printf("error\n");
			break;
	}

	printf("edge-cut: %d\n", edgecut);

	printPartitioning(rank, part, 5);

	MPI::Finalize();
	return EXIT_SUCCESS;
}

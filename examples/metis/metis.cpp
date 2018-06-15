#include <iostream>
#include <metis.h>

using namespace std;

template<typename T>
void printPartitioning(T* ad, int size) {
	printf("partitioning:");
	for (int i=0; i<size; ++i) {
		printf(" %d",ad[i]);
	}
	printf("\n");
}

int main(int argc, const char *argv[])
{
	//idx_t xadj[]     = {0,2,5,8,11,13,16,20,24,28,31,33,36,39,42,44};
	//idx_t adjncy[]   = {1,5,0,2,6,1,3,7,2,4,8,3,9,\
							//0,6,10,1,5,7,11,2,6,8,\
							//12,3,7,9,13,4,8,14,5,11,\
							//6,10,12,7,11,13,8,12,14,9,13};

	//idx_t nvtxs      = 15;   // number of vertices
	//idx_t ncon       = 1;    // number of constraints	
	//idx_t nparts     = 3;    // number of partitions to partition the graph in



	idx_t xadj[]     = {0, 1, 2};
	idx_t adjncy[]   = {1, 0};

	idx_t nvtxs      = 2;   // number of vertices
	idx_t ncon       = 1;    // number of constraints	
	idx_t nparts     = 2;    // number of partitions to partition the graph in

	idx_t* vwgt      = NULL; // vertex weight
	idx_t* vsize     = NULL; // vertex weight
	idx_t* adjwgt    = NULL; // vertex weight
	real_t* tpwgts   = NULL; // vertex weight
	real_t* ubvec    = NULL; // vertex weight
	idx_t* options   = NULL; // vertex weight

	idx_t objval; // stores edge-cut
	idx_t* part = (idx_t*)new idx_t[nvtxs];

	int ret = METIS_PartGraphKway(
			&nvtxs,
			&ncon,
			xadj,
			adjncy,
			vwgt, vsize, adjwgt,
			&nparts,
			tpwgts, ubvec, options,
			&objval, part);

	switch (ret) {
		case METIS_OK:
			break;
		case METIS_ERROR_INPUT:
		case METIS_ERROR_MEMORY:
		case METIS_ERROR:
			printf("error\n");
			break;
	}

	printf("edge-cut: %d\n", objval);
	printPartitioning(part, nvtxs);

	return EXIT_SUCCESS;
}

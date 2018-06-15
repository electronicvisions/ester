#include <boost/mpl/greater.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/multiplies.hpp>
#include <boost/mpl/placeholders.hpp>
#include <cstdio>

#include "lut.h"

namespace mpl = boost::mpl;

template <typename x>
struct factorial :
	mpl::if_<mpl::greater<x, mpl::int_<1> >,
	    mpl::multiplies<x, factorial<typename x::prior> >,
	    mpl::int_<1>
	>::type
{};

GENERATE_LUT(factorial, int, FACTORIAL_TABLE, 12); // 13 fails?

int main(int argc, char ** argv) {
	// This should print '24:'
	printf("Result: %d.\n", FACTORIAL_TABLE[3]);
	return 0;

}



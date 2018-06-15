#include <string>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <cassert>

#include "logging.h"

// takes file handle
void redirectStdout(int const fd) {
	static int oldfd = -1;
	static fpos_t pos;
	static int wasFile;

	if (fd == -1)
		throw std::runtime_error("Invalid file handle");

	// flush stdout
	assert( fflush(stdout) != -1 );

	if (oldfd == -1) {
		// save pos and fd
		if ( fgetpos(stdout, &pos) != -1) { // if already redirected to file
			wasFile = 1;
		} else {
			wasFile = 0;
		}
		oldfd = dup(fileno(stdout));
		assert( oldfd != -1 );
		// redirect
		assert( dup2(fd, fileno(stdout)) != -1 );
	} else {
		// restore stdout
		assert( dup2(oldfd, fileno(stdout)) != -1 );
		assert( close(oldfd) != -1 );
		clearerr(stdout);
		if (wasFile)
			fsetpos(stdout, &pos);
		oldfd = -1;
	}
}

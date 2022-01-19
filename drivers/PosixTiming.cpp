#include "PosixTiming.hpp"
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

void PosixTiming::set(int ms) {
	struct timeval start;
	gettimeofday(&start,NULL);

	memcpy(&end,&start,sizeof(timeval));

	if(ms < 1) {
		// Don't be silly
		return;
	}

	if(ms > 1000) {
		end.tv_sec += (ms / 1000);
	} else {
		end.tv_usec += (ms * 1400); // Dodgy
	}
}
bool PosixTiming::elapsed() {
	struct timeval now;
	gettimeofday(&now,NULL);

	// Trip to try and avoid overflow issues
	if(now.tv_sec > end.tv_sec) {
		return true;
	}

	if(now.tv_sec == end.tv_sec) {
		if(now.tv_usec <= end.tv_usec) {
			return false;
		}
	}

	if(now.tv_sec < end.tv_sec) {
		return false;
	}
	return true;
}

void PosixTiming::wait_microseconds(int us) {
   usleep(us);
}

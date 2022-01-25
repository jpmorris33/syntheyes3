#include "PosixTiming.hpp"
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

static time_t diff_ms(struct timeval *start, struct timeval *end);

PosixTiming::PosixTiming() {
	gettimeofday(&start,NULL);
	duration=0;
}

void PosixTiming::set(int ms) {
	gettimeofday(&start,NULL);
	duration=0;
	if(ms < 1) {
		// Don't be silly
		return;
	}

	duration = ms;
}
bool PosixTiming::elapsed() {
	struct timeval now;
	gettimeofday(&now,NULL);

	time_t runtime = diff_ms(&start, &now);
	if(runtime > duration) {
		return true;
	}
	return false;
}

void PosixTiming::wait_microseconds(int us) {
	usleep(us);
}

static time_t diff_ms(struct timeval *start, struct timeval *end) {
	time_t ms = (end->tv_sec - start->tv_sec) * 1000;
	suseconds_t remainder = end->tv_usec - start->tv_usec;
	ms += (remainder / 1000);
	return ms;
}
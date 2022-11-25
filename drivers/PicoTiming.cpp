#include "PicoTiming.hpp"

void PicoTiming::set(int ms) {
	absolute_time_t start = get_absolute_time();
	end = start + (ms * 1000); // This will crash if the software is left running for more than 7 million years
}
bool PicoTiming::elapsed() {
	if(get_absolute_time() < end) {
		return false;
	}
	return true;
}

void PicoTiming::wait_microseconds(int us) {
   sleep_us(us);
}

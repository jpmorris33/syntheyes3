#include <time.h>
#include "Timing.hpp"

class PosixTiming : public Timing {
	public:
		PosixTiming();
		void set(int ms);
		bool elapsed();
		void wait_microseconds(int us);
	private:
		struct timeval start;
		time_t duration;
};

#include <time.h>
#include "Timing.hpp"

class PosixTiming : public Timing {
	public:
		void set(int ms);
		bool elapsed();
		void wait_microseconds(int us);
	private:
		struct timeval end;
};

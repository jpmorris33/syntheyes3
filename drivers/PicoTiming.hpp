#include "pico/stdlib.h"
#include "Timing.hpp"

class PicoTiming : public Timing {
	public:
		void set(int ms);
		bool elapsed();
		void wait_microseconds(int us);
	private:
		absolute_time_t end;
};

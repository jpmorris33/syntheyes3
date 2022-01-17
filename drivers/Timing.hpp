#ifndef __TIMING_HPP__
#define __TIMING_HPP__

class Timing {
	public:
		virtual void set(int ms);
		virtual bool elapsed();
		virtual void wait_microseconds(int us);
};

#endif
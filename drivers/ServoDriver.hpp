#ifndef __SERVODRIVER_HPP__
#define __SERVODRIVER_HPP__

#include <stdint.h>
#include "../gpio.hpp"
#include "Timing.hpp"

class ServoDriver {
	public:
		virtual void init(int defaultAngle, const char *param);
		void update();
		void setAngle(int angle);
		void seekAngle(int angle);
		void setDelay(int delay);
		int getAngle();
	protected:
		virtual void write(int angle);
		int curAngle;
		int targetAngle;
		int seekDelay;
		Timing *timer;
};

extern const char *getDriverParam(const char *string, const char *cmd);
extern int getDriverInt(const char *param);
extern const char *getDriverStr(const char *param);

#endif

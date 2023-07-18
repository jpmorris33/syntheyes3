#ifndef __SDLSENSORDRIVER_HPP__
#define __SDLSENSORDRIVER_HPP__

#include <stdint.h>
#include "../SensorDriver.hpp"

class SDLSensor : public SensorDriver {
	public:
		virtual void init(const char *param);
		virtual bool check();
		virtual bool isChannel(int channel);
	private:
		bool channelmap[8];
};

#endif

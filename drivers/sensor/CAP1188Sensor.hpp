#ifndef __CAP1188SENSORDRIVER_HPP__
#define __CAP1188SENSORDRIVER_HPP__

#include <stdint.h>
#include "../SensorDriver.hpp"

class CAP1188Sensor : public SensorDriver {
	public:
		CAP1188Sensor();
		virtual void init(const char *param);
		virtual bool check();
		virtual bool isChannel(int channel);
	private:
		unsigned char channels;
		int i2cHandle;
		unsigned char readReg(int reg);
		void writeReg(int reg, unsigned char val);
};

extern const char *getDriverParam(const char *string, const char *cmd);
extern int getDriverHex(const char *param);

#endif

#ifndef __SENSOR_HPP__
#define __SENSOR_HPP__

class SensorDriver {
	public:
		virtual void init(const char *param);
		virtual bool check();
		virtual bool isChannel(int channel);
};

#define MAX_SENSOR_CHANNELS 32

extern int registerSensorChannel(int channelID);
extern int sensorChannel[MAX_SENSOR_CHANNELS];
extern int sensorChannels;

#endif
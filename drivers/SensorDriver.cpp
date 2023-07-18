#include "SensorDriver.hpp"

int sensorChannel[MAX_SENSOR_CHANNELS];
int sensorChannels=0;

void SensorDriver::init(const char *param) {}
bool SensorDriver::check() {return false;}
bool SensorDriver::isChannel(int channel) {return false;}


int registerSensorChannel(int channelID) {
	
	if(channelID == -1) {
		// Don't register 'any'
		return -1;
	}

	for(int ctr=0;ctr<sensorChannels;ctr++) {
		if(sensorChannel[ctr] == channelID) {
			return ctr;
		}
	}

	if(sensorChannels >= MAX_SENSOR_CHANNELS) {
		return -1;
	}

	sensorChannel[sensorChannels] = channelID;
	sensorChannels++;

	return sensorChannels-1;
}

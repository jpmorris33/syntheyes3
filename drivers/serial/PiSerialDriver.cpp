#ifdef PLATFORM_PI

#include "PiSerialDriver.hpp"

#include <wiringPi.h>
#include <wiringSerial.h>
#include <string.h>

#define INVALID -1

PiSerialDriver::PiSerialDriver() {
	fd = INVALID;
}

PiSerialDriver::~PiSerialDriver() {
	close();
}

bool PiSerialDriver::open_read(const char *port, int baud) {

	if(fd != INVALID) {
		close();
	}

	fd=serialOpen(port, baud);
	if(fd == INVALID) {
		return false;
	}
	return true;
}

bool PiSerialDriver::open_write(const char *port, int baud) {
	// WiringPi serial driver is bidirectional
	return open_read(port,baud);
}

void PiSerialDriver::close() {
	if(fd != INVALID) {
		serialClose(fd);
		fd = INVALID;
	}
}

int PiSerialDriver::read(char *buffer, int maxlen) {

	int ret=0;
	char input[2];
	input[1]=0;
	buffer[0]=0;

	if(fd == INVALID) {
		return ret;
	}

	ret = serialDataAvail(fd);
	if(ret > 0) {
		while(serialDataAvail(fd) > 0) {
			input[0]=serialGetchar(fd);
			if(input[0] == '<') {
				break;
			}
		}

		// No luck
		if(input[0] != '<') {
			return 0;
		}

		while(serialDataAvail(fd) > 0) {
			input[0]=serialGetchar(fd);
			if(input[0] == '>') {
				return strlen(buffer);
			}
			strcat(buffer,input);
			if(strlen(buffer) >= maxlen-1) {
				return maxlen-1;
			}
		};

		return strlen(buffer);
	}

	return 0;	
}

int PiSerialDriver::write(const char *msg) {

	int len = strlen(msg);

	if(fd == INVALID) {
		return 0;
	}

	serialPutchar(fd,'<');
	for(int ctr=0;ctr<len;ctr++) {
		serialPutchar(fd,msg[ctr]);
	}
	serialPutchar(fd,'>');

	serialPuts(fd,msg);
	return strlen(msg)+2; // include start/stop codes

}
#endif

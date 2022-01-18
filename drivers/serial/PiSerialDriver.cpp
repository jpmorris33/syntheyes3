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

	if(fd == INVALID) {
		return ret;
	}

	ret = serialDataAvail(fd);
	if(ret > 0) {
		if(ret > maxlen) {
			ret=maxlen-1; // reserve for zero
		}
		memset(buffer,0,maxlen);
		for(int ctr=0;ctr<ret;ctr++) {
			buffer[ctr]=serialGetchar(fd);
		}
	}

	return ret;	
}

int PiSerialDriver::write(const char *msg) {
	if(fd == INVALID) {
		return 0;
	}

	serialPuts(fd,msg);
	return strlen(msg)+1; // include zero

}
#endif

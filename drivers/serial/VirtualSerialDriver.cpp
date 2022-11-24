#include "VirtualSerialDriver.hpp"
#include <string.h>

extern Platform *sys;

//
//  Hacky and not very reliable file-based serial emulator
//

VirtualSerialDriver::VirtualSerialDriver() {
}

VirtualSerialDriver::~VirtualSerialDriver() {
	close();
}

bool VirtualSerialDriver::open_read(const char *port, int baud) {

	FileIO *fp=sys->openSerial(port, "rb");
	if(!fp) {
		return false;
	}
	sys->closeFile(fp);

	strncpy(filename,port,1023);
	filename[1023]=0;

	return true;
}

bool VirtualSerialDriver::open_write(const char *port, int baud) {

	FileIO *fp=sys->openSerial(port, "wb");
	if(!fp) {
		return false;
	}
	sys->closeFile(fp);

	strncpy(filename,port,1023);
	filename[1023]=0;

	return true;
}

void VirtualSerialDriver::close() {
}

int VirtualSerialDriver::read(char *buffer, int maxlen) {

	int read=0;
	FileIO *fp=sys->openFile(filename, "rb");
	if(!fp) {
		return 0;
	}
	memset(buffer,0,maxlen);
	read=fp->read(buffer,maxlen-1);

	sys->closeFile(fp);
	if(read > 0) {
		write(""); // destroy contents
	}

	return read;

}

int VirtualSerialDriver::write(const char *msg) {
	int written=0;

	FileIO *fp=sys->openFile(filename, "wb");
	if(!fp) {
		return 0;
	}

	written=fp->write(msg,strlen(msg)+1);
	sys->closeFile(fp);

	return written;

}

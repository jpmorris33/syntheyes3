#include "VirtualSerialDriver.hpp"
#include <string.h>

//
//  Hacky and not very reliable file-based serial emulator
//

VirtualSerialDriver::VirtualSerialDriver() {
}

VirtualSerialDriver::~VirtualSerialDriver() {
	close();
}

bool VirtualSerialDriver::open_read(const char *port, int baud) {

	FILE *fp=fopen(port, "rb");
	if(!fp) {
		return false;
	}
	fclose(fp);

	strncpy(filename,port,1023);
	filename[1023]=0;

	return true;
}

bool VirtualSerialDriver::open_write(const char *port, int baud) {

	FILE *fp=fopen(port, "wb");
	if(!fp) {
		return false;
	}
	fclose(fp);

	strncpy(filename,port,1023);
	filename[1023]=0;

	return true;
}

void VirtualSerialDriver::close() {
}

int VirtualSerialDriver::read(char *buffer, int maxlen) {

	int read=0;
	FILE *fp=fopen(filename, "rb");
	if(!fp) {
		return 0;
	}
	memset(buffer,0,maxlen);
	read=fread(buffer,1,maxlen-1,fp);

	fclose(fp);
	if(read > 0) {
		write(""); // destroy contents
	}

	return read;

}

int VirtualSerialDriver::write(const char *msg) {
	int written=0;

	FILE *fp=fopen(filename, "wb");
	if(!fp) {
		return 0;
	}

	written=fwrite(msg,1,strlen(msg)+1,fp);
	fclose(fp);

	return written;

}

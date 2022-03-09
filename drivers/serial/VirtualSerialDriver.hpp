#ifndef __VirtualSerialDriver_HPP__
#define __VirtualSerialDriver_HPP__

#include "../../platforms/Platform.hpp"
#include "../SerialDriver.hpp"

#include <stdio.h>
#include <stdbool.h>

class VirtualSerialDriver : public SerialDriver {
	public:
		VirtualSerialDriver();
		~VirtualSerialDriver();
		bool open_read(const char *port, int baud);
		bool open_write(const char *port, int baud);
		void close();
		int read(char *buffer, int maxlen);
		int write(const char *msg);
	private:
		char filename[1024];
};

#endif
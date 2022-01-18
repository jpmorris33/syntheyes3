#ifndef __PISERIALDRIVER_HPP__
#define __PISERIALDRIVER_HPP__

#include "../SerialDriver.hpp"

#include <stdbool.h>

class PiSerialDriver : public SerialDriver {
	public:
		PiSerialDriver();
		~PiSerialDriver();
		bool open_read(const char *port, int baud);
		bool open_write(const char *port, int baud);
		void close();
		int read(char *buffer, int maxlen);
		int write(const char *msg);
	private:
		int fd;
};

#endif

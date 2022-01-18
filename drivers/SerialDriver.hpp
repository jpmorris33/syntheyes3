#ifndef __SERIALDRIVER_HPP__
#define __SERIALDRIVER_HPP__

#include <stdbool.h>

class SerialDriver {
	public:
		virtual bool open_read(const char *port, int baud);
		virtual bool open_write(const char *port, int baud);
		virtual void close();
		virtual int read(char *buffer, int maxlen);
		virtual int write(const char *msg);
};

#endif
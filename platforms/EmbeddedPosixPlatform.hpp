#ifndef __EMBEDDEDPOSIXPLATFORM_HPP__
#define __EMBEDDEDPOSIXPLATFORM_HPP__

#include "Platform.hpp"

class EmbeddedPosixPlatform : public Platform {
	public:
		EmbeddedPosixPlatform();
		~EmbeddedPosixPlatform();
		void *alloc(long amount);
		void free(void *ptr);
		void exit(int code);
		FileIO *openFile(const char *filename, const char *mode);
		FileIO *openSerial(const char *filename, const char *mode);
		void closeFile(FileIO *file);
		Timing *getTimer();
};

#endif
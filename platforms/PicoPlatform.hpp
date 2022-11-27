#ifndef __PICOPLATFORM_HPP__
#define __PICOPLATFORM_HPP__

#include "Platform.hpp"

class PicoPlatform : public Platform {
	public:
		PicoPlatform();
		~PicoPlatform();
		void *alloc(long amount);
		void free(void *ptr);
		void exit(int code);
		FileIO *openFile(const char *filename, const char *mode);
		FileIO *openSerial(const char *filename, const char *mode);
		bool access(const char *filename, int mode);
		void closeFile(FileIO *file);
		Timing *getTimer();
};

#endif
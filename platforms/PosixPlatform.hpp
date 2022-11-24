#ifndef __POSIXPLATFORM_HPP__
#define __POSIXPLATFORM_HPP__

#include "Platform.hpp"

class PosixFileIO : public FileIO {
	public:
		PosixFileIO();
		~PosixFileIO();
		bool open(const char *filename, const char *mode);
		void close();
		long read(void *buffer, long bytes);
		long write(const void *buffer, long bytes);
		char *readLine(char *buffer, long bytes);
		void seek(long offset);
		long tell();
		bool eof();
};

class PosixPlatform : public Platform {
	public:
		PosixPlatform();
		~PosixPlatform();
		void *alloc(long amount);
		void free(void *ptr);
		void exit(int code);
		FileIO *openFile(const char *filename, const char *mode);
		FileIO *openSerial(const char *filename, const char *mode);
		void closeFile(FileIO *file);
		Timing *getTimer();
};

#endif
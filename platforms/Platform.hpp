#ifndef __PLATFORM_HPP__
#define __PLATFORM_HPP__

#include "../drivers/Timing.hpp"

#define ACCESS_R_OK 1
#define ACCESS_W_OK 2
#define ACCESS_X_OK 4

class FileIO {
	public:
		virtual ~FileIO();
		virtual bool open(const char *filename, const char *mode);
		virtual void close();
		virtual long read(void *buffer, long bytes);
		virtual long write(const void *buffer, long bytes);
		virtual char *readLine(char *buffer, long bytes);
		virtual void seek(long offset);
		virtual long tell();
		virtual bool eof();
	protected:
		void *filehandle;
};

class Platform {
	public:
		virtual void *alloc(long amount);
		virtual void free(void *ptr);
		virtual void exit(int code);
		virtual FileIO *openFile(const char *filename, const char *mode);
		virtual FileIO *openSerial(const char *filename, const char *mode);
		virtual void closeFile(FileIO *file);
		virtual bool access(const char *filename, int mode);
		virtual Timing *getTimer();
};

#endif
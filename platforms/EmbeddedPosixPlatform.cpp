//
//	Experimental Embedded Posix platform wrapper
//

#include "EmbeddedPosixPlatform.hpp"
#include "../drivers/PosixTiming.hpp"
#include "TarFS.hpp"

#ifdef EMBEDDED_ENABLED
	#include "../embedded/testtar.h"	// imageBytes
#else
	unsigned char imageBytes[]="NO";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//  Simple wrapper around fopen
//


//
//	Basic library function wrappers
//


EmbeddedPosixPlatform::EmbeddedPosixPlatform() {
}

EmbeddedPosixPlatform::~EmbeddedPosixPlatform() {
}

void *EmbeddedPosixPlatform::alloc(long amount) {
	if(amount < 1) {
		return NULL;
	}
	return calloc(1,amount);
}

void EmbeddedPosixPlatform::free(void *ptr) {
	if(ptr) {
		::free(ptr);
	}
}

void EmbeddedPosixPlatform::exit(int code) {
	::exit(code);
}

FileIO *EmbeddedPosixPlatform::openFile(const char *filename, const char *mode) {
	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

FileIO *EmbeddedPosixPlatform::openSerial(const char *filename, const char *mode) {
	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

void EmbeddedPosixPlatform::closeFile(FileIO *file) {
	if(file) {
		delete file;
	}
}

bool EmbeddedPosixPlatform::access(const char *filename, int mode) {
 	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	bool ret = fp->open(filename,"rb");
	delete fp;
	return ret;
}

Timing *EmbeddedPosixPlatform::getTimer() {
	return new PosixTiming();
}
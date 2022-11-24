//
//	Experimental Embedded Posix platform wrapper
//

#include "EmbeddedPosixPlatform.hpp"
#include "../drivers/PosixTiming.hpp"
#include "TarFS.hpp"

#include "../embedded/testtar.h"	// imageBytes

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

Timing *EmbeddedPosixPlatform::getTimer() {
	return new PosixTiming();
}

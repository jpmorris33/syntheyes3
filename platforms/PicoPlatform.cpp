//
//	Raspberry Pico platform wrapper
//

#include "PicoPlatform.hpp"
#include "../drivers/PicoTiming.hpp"
#include "TarFS.hpp"
#include "../embedded/testtar.h"
//#include "../embedded/reserve.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//  Simple wrapper around fopen
//


//
//	Basic library function wrappers
//


PicoPlatform::PicoPlatform() {
}

PicoPlatform::~PicoPlatform() {
}

void *PicoPlatform::alloc(long amount) {
	if(amount < 1) {
		return NULL;
	}
	return calloc(1,amount);
}

void PicoPlatform::free(void *ptr) {
	if(ptr) {
		::free(ptr);
	}
}

void PicoPlatform::exit(int code) {
	::exit(code);
}

FileIO *PicoPlatform::openFile(const char *filename, const char *mode) {
 	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

FileIO *PicoPlatform::openSerial(const char *filename, const char *mode) {
 	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

void PicoPlatform::closeFile(FileIO *file) {
	if(file) {
		delete file;
	}
}

bool PicoPlatform::access(const char *filename, int mode) {
 	FileIO *fp = new TarFileIO(imageBytes,sizeof(imageBytes));
	bool ret = fp->open(filename,"rb");
	delete fp;
	return ret;
}


Timing *PicoPlatform::getTimer() {
	return new PicoTiming();
}

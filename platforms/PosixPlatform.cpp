//
//	Posix platform wrappers
//

#include "PosixPlatform.hpp"
#include "../drivers/PosixTiming.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//
//  Simple wrapper around fopen
//

PosixFileIO::PosixFileIO() {
	filehandle=NULL;
}

PosixFileIO::~PosixFileIO() {
	if(filehandle) {
		close();
	}
}

bool PosixFileIO::open(const char *filename, const char *mode) {
	if((!filename) || (!mode)) {
		return false;
	}
	FILE *fp = fopen(filename,mode);
	if(fp) {
		filehandle = (void *)fp;
		return true;
	}

	return false;
}

void PosixFileIO::close() {
	if(filehandle) {
		fclose((FILE *)filehandle);
		filehandle = NULL;
	}
}

long PosixFileIO::read(void *buffer, long bytes) {
	if(!filehandle) {
		return 0;
	}
	return fread(buffer,1,bytes,(FILE *)filehandle);
}

long PosixFileIO::write(const void *buffer, long bytes) {
	if(!filehandle) {
		return 0;
	}
	return fwrite(buffer,1,bytes,(FILE *)filehandle);
}

char *PosixFileIO::readLine(char *buffer, long bytes) {
	if((!filehandle) || (!buffer)) {
		return NULL;
	}
	memset(buffer,0,bytes);
	return fgets(buffer,bytes,(FILE *)filehandle);
}

void PosixFileIO::seek(long offset) {
	if(!filehandle) {
		return;
	}
	fseek((FILE *)filehandle,offset,SEEK_SET);
}

long PosixFileIO::tell() {
	if(!filehandle) {
		return 0;
	}
	return ftell((FILE *)filehandle);
}

bool PosixFileIO::eof() {
	if(!filehandle) {
		return true;
	}
	return feof((FILE *)filehandle);
}


//
//	Basic library function wrappers
//


PosixPlatform::PosixPlatform() {
}

PosixPlatform::~PosixPlatform() {
}

void *PosixPlatform::alloc(long amount) {
	if(amount < 1) {
		return NULL;
	}
	return calloc(1,amount);
}

void PosixPlatform::free(void *ptr) {
	if(ptr) {
		::free(ptr);
	}
}

void PosixPlatform::exit(int code) {
	::exit(code);
}

FileIO *PosixPlatform::openFile(const char *filename, const char *mode) {
	FileIO *fp = new PosixFileIO();
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

FileIO *PosixPlatform::openSerial(const char *filename, const char *mode) {
	FileIO *fp = new PosixFileIO();
	if(fp->open(filename,mode)) {
		return fp;
	}
	delete fp;
	return NULL;
}

void PosixPlatform::closeFile(FileIO *file) {
	if(file) {
		delete file;
	}
}

bool PosixPlatform::access(const char *filename, int mode) {
	int newmode = 0;
	if(mode & ACCESS_R_OK) {
		newmode |= R_OK;
	}
	if(mode & ACCESS_X_OK) {
		newmode |= X_OK;
	}

	return !::access(filename,newmode);
}

Timing *PosixPlatform::getTimer() {
	return new PosixTiming();
}

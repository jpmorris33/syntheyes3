//
//	Dummy platform wrappers
//

#include <stdlib.h>
#include "Platform.hpp"

FileIO::~FileIO() {}
bool FileIO::open(const char *filename, const char *mode) { return false; }
void FileIO::close() {}
long FileIO::read(void *buffer, long bytes) { return 0; }
long FileIO::write(const void *buffer, long bytes) { return 0; }
char *FileIO::readLine(char *buffer, long bytes) { return buffer; }
void FileIO::seek(long offset) {}
long FileIO::tell() { return 0; }
bool FileIO::eof() { return false; }

void *Platform::alloc(long amount) { return NULL; }
void Platform::free(void *ptr) {}
void Platform::exit(int code) {}
FileIO *Platform::openFile(const char *filename, const char *mode) { return NULL; }
FileIO *Platform::openSerial(const char *filename, const char *mode) { return NULL; }
void Platform::closeFile(FileIO *file) {}
bool Platform::access(const char *filename, int mode) { return false; }
Timing *Platform::getTimer() { return NULL; }
void Platform::background(void *(*thread)(void *), void *parm) {}


//
//	Simple RAM filesystem
//

#include "MemFS.hpp"

#include <string.h>

//
//  RAM-based Virtual Filesystem - this 'filesystem' represents a single
// file, to be provided by a higher-level layer, e.g. TarFS
//

MemFS::MemFS(unsigned char *src, long length) {
	filelen = length;
	filehandle = (void *)src;
	startptr = endptr = curptr = src;
	endptr += filelen;
}

MemFS::~MemFS() {
	filehandle = NULL;
}

bool MemFS::open(const char *filename, const char *mode) {
	if((!filename) || (!mode)) {
		return false;
	}
	curptr = startptr;
	return true;
}

void MemFS::close() {
	filehandle = NULL;
}

long MemFS::read(void *buffer, long bytes) {
	if((!filehandle) || bytes < 1) {
		return 0;
	}
	if(curptr >= endptr) {
		return 0;	// EOF
	}
	if(curptr + bytes > endptr) {
		bytes = (long)endptr - (long)curptr;
	}
	memcpy(buffer,curptr,bytes);
	curptr += bytes;
	return bytes;
}

long MemFS::write(const void *buffer, long bytes) {
	return 0; // No
}

char *MemFS::readLine(char *buffer, long bytes) {
	unsigned char *ptr;
	unsigned char *end;
	unsigned char *out;

	if((!filehandle) || (!buffer) || bytes < 1) {
		return NULL;
	}
	memset(buffer,0,bytes);
	out = (unsigned char *)buffer;
	ptr = curptr;
	end = curptr + (bytes-1);
	if(end > endptr) {
		end = endptr;
	}
	while(ptr < end) {
		if(*ptr == 0x0d || *ptr == 0x0a) {
			break;
		}
		*out++ = *ptr++;
	};
	// Skip CR/LF so the next line doesn't immediately bail on us
	if(*ptr == 0x0d) {
		ptr++;
	}
	if(*ptr == 0x0a) {
		ptr++;
	}
	curptr = ptr;
	return buffer;
}

void MemFS::seek(long offset) {
	if((!filehandle) || offset < 0) {
		return;
	}
	curptr = startptr + offset;
}

long MemFS::tell() {
	if(!filehandle) {
		return 0;
	}
	return (long)curptr-(long)startptr;
}

bool MemFS::eof() {
	if(!filehandle) {
		return true;
	}
	return curptr >= endptr;
}


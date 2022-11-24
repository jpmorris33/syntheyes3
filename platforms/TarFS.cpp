//
//	Posix platform wrappers
//

#include "PosixPlatform.hpp"
#include "TarFS.hpp"
#include "MemFS.hpp"
#include "../drivers/PosixTiming.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
//  TAR Virtual Filesystem
//

static MemFS *find_file(unsigned char *start, const char *filename, long len);

TarFileIO::TarFileIO(unsigned char *src, long length) {
	filehandle=NULL;
	datasrc = src;
	datalen = length;

}

TarFileIO::~TarFileIO() {
	if(filehandle) {
		close();
	}
}

bool TarFileIO::open(const char *filename, const char *mode) {
	if((!filename) || (!mode)) {
		return false;
	}
	
	MemFS *fp = find_file(datasrc, filename, datalen);
	if(fp) {
		filehandle = (void *)fp;
		return true;
	}

	return false;
}

void TarFileIO::close() {
	MemFS *fp = (MemFS *)filehandle;
	if(filehandle) {
		delete fp;
		filehandle = NULL;
	}
}

long TarFileIO::read(void *buffer, long bytes) {
	MemFS *fp = (MemFS *)filehandle;
	if(!filehandle) {
		return 0;
	}
	return fp->read(buffer, bytes);
}

long TarFileIO::write(const void *buffer, long bytes) {
	return 0;
}

char *TarFileIO::readLine(char *buffer, long bytes) {
	MemFS *fp = (MemFS *)filehandle;
	if((!filehandle) || (!buffer)) {
		return NULL;
	}
	return fp->readLine(buffer,bytes);
}

void TarFileIO::seek(long offset) {
	MemFS *fp = (MemFS *)filehandle;
	if(!filehandle) {
		return;
	}
	return fp->seek(offset);
}

long TarFileIO::tell() {
	MemFS *fp = (MemFS *)filehandle;
	if(!filehandle) {
		return 0;
	}
	return fp->tell();
}

bool TarFileIO::eof() {
	MemFS *fp = (MemFS *)filehandle;
	if(!filehandle) {
		return true;
	}
	return fp->eof();
}

//
//	Scan the TAR header to find the file, or else return NULL
//

MemFS *find_file(unsigned char *start, const char *filename, long len) {
	char block[512];
	char *name;
	long idx,flen,flen512;

	if(len < 512) {
		return NULL;
	}

	idx=0;

	while(idx<len) {
		memcpy(block,&start[idx],512);
		idx += 512; // Start of file (or next file)

		name = &block[0];

		// Length is in octal
		flen = strtol(&block[124], NULL, 8);

		// Round to nearest 512-byte block
		flen512 = flen;
		if(flen512 & 0x1ff) {
			flen512 &= ~0x1ff;
			flen512 += 0x200;
		}

		if(!strcmp(filename,name)) {
			return new MemFS(&start[idx],flen);
		}

		if(*name == '.') {
			name++;
		}
		if(*name == '/') {
			name++;
		}

		if(!strcmp(filename,name)) {
			return new MemFS(&start[idx],flen);
		}

		idx += flen512;
	};

	return NULL;

}
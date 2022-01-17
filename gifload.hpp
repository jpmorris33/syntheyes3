#ifndef __GIFLOAD_HPP__
#define __GIFLOAD_HPP__

struct GIFFRAME {
	unsigned char *imgdata;
	int delay;
};

struct GIFANIM {
	int w,h;
	int frames;
	GIFFRAME *frame;
	int keycolour;
};

extern GIFANIM *loadgif(const char *path);

#endif
#ifdef PLATFORM_PI

/**
 * Render to a Unicorn HD panel via SPI
 */

#include "Unicorn.hpp"
#include "../PosixTiming.hpp"

#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define REFRESH_MS 16 // approximately 1/60 second

unsigned char spioutputbuf[769];
extern bool transmitter;
extern uint32_t rainbow[16]; // Colour table
static PosixTiming refresh;
static unsigned char rainbowpattern[16][16];

//
//	Init the Unicorn HD driver
//
void Unicorn::init() {

	panelW = UNICORNPANEL_W;
	panelH = UNICORNPANEL_H;

	wiringPiSetup();
	wiringPiSPISetup(0,9000000);
	refresh.set(0);
}

//
//	Get driver capabilities
//

uint32_t Unicorn::getCaps() {
	return PANELCAPS_SPLIT|PANELCAPS_FIXED;
}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void Unicorn::draw() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &spioutputbuf[0];
	*outptr++=0x72;   // for addressing multiple panels, it's 0x73 + zero-based panel address

	// cheat and memcpy the whole lot
	memcpy(outptr,inptr,768);

	// The panel gets fussy if it's updated more than about 60Hz
	if(refresh.elapsed()) {
		refresh.set(REFRESH_MS);
		wiringPiSPIDataRW(0,spioutputbuf,769);
	}
}

//
//	Put the framebuffer onto the Unicorn HD panel backwards
//
void Unicorn::drawMirrored() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &spioutputbuf[0];
	*outptr++=0x72;   // for addressing multiple panels, it's 0x73 + zero-based panel address

	int windowwidth = panelW * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<panelH;ctr++)  {
		for(int xpos=windowwidth-3;xpos>=0;xpos-=3) {
			memcpy(outptr,&inptr[xpos],3);
			outptr+=3;
		}
		inptr += windowwidth;
	}

	// The panel gets fussy if it's updated more than about 60Hz
	if(refresh.elapsed()) {
		refresh.set(REFRESH_MS);
		wiringPiSPIDataRW(0,spioutputbuf,769);
	}
}


void Unicorn::updateRGB(unsigned char *img, int w, int h) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<panelW;x++) {
			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = in[0];
					out[1] = in[1];
					out[2] = in[2];
				}
				in +=3;
			}
			out+=3;
		}
	}
}

void Unicorn::updateRGB(unsigned char *img, int w, int h, uint32_t colour) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<panelW;x++) {
			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = r;
					out[1] = g;
					out[2] = b;
				}
				in +=3;
			}
			out+=3;
		}
	}
}

void Unicorn::updateRGBpattern(unsigned char *img, int w, int h, int offset) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;
	int index=0,xpos=0,ypos=0;
	unsigned char r,g,b;

	ypos=0;
	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		xpos=0;
		for(int x=0;x<panelW;x++) {
			index = (offset + (rainbowpattern[ypos][xpos]&0x0f))&0x0f;
			b=rainbow[index]&0xff;
			g=(rainbow[index]>>8)&0xff;
			r=(rainbow[index]>>16)&0xff;
			xpos++;
			xpos &= 0x0f; // Constrain to 16 pixels

			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = r;
					out[1] = g;
					out[2] = b;
				}
				in +=3;
			}
			out+=3;
		}

		ypos++;
		ypos &= 0x0f; // Constrain to 16 pixels

	}
}

//
//	Set the special effect pattern
//
void Unicorn::setPattern(unsigned char pattern[16][16]) {
	memcpy(rainbowpattern, pattern, 16*16);
}

void Unicorn::clear(uint32_t colour) {
	int len=panelW*panelH;
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	for(int ctr=0;ctr<len;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
	}
}

void Unicorn::clearV(int x, uint32_t colour) {
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	int offset = (panelW-1)*3;

	if(x<0 || x >= panelW) {
		return;
	}

	ptr += (x*3);  // Find the column

	for(int ctr=0;ctr<panelH;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
		ptr+=offset;
	}
}

void Unicorn::clearH(int y, uint32_t colour) {
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	if(y<0 || y >= panelH) {
		return;
	}

	ptr += ((y*panelW)*3);  // Find the row

	for(int ctr=0;ctr<panelW;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
	}
}

#endif

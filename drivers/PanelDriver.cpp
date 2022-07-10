#include "PanelDriver.hpp"

#include <string.h>

extern uint32_t rainbow[16]; // Colour table for gradients


//
//  Default stubs for the concrete implementations to override
//

void PanelDriver::init(const char *param) {}
void PanelDriver::draw() {}
void PanelDriver::drawMirrored() {}
int PanelDriver::getW() {return panelW;}
int PanelDriver::getH() {return panelH;}
uint32_t PanelDriver::getCaps() {return 0;};
void PanelDriver::setBrightness(int percentage) {}

//
//  Common code
//

void PanelDriver::updateRGB(unsigned char *img, int w, int h) {
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


void PanelDriver::updateRGB(unsigned char *img, int w, int h, uint32_t colour) {
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

void PanelDriver::updateRGBpattern(unsigned char *img, int w, int h, int offset) {
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
void PanelDriver::setPattern(unsigned char pattern[16][16]) {
	memcpy(rainbowpattern, pattern, 16*16);
}

void PanelDriver::clear(uint32_t colour) {
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

void PanelDriver::clearV(int x, uint32_t colour) {
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

void PanelDriver::clearH(int y, uint32_t colour) {
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

//
//	Flip an RGB framebuffer 180 degrees - can be overridden for other formats, e.g. RGBA or 4-byte alignment with padding
//

void PanelDriver::rotate180(unsigned char *buffer, int w, int h) {
	unsigned char *startptr = buffer;
	unsigned char *endptr = buffer + (w * h * 3);
	unsigned char r,g,b;

	int pixels=w*h;
	pixels >>= 1;	// Halve the total since we're doing both ends at once

	for(int ctr=0;ctr<pixels;ctr++) {
		r=startptr[0];
		g=startptr[1];
		b=startptr[2];
		endptr -= 3;
		startptr[0]=endptr[0];
		startptr[1]=endptr[1];
		startptr[2]=endptr[2];
		endptr[0]=r;
		endptr[1]=g;
		endptr[2]=b;
		startptr += 3;
	}
}
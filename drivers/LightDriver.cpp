#include "LightDriver.hpp"

#include <stdio.h>
#include <string.h>

extern char colourscale[256][256];


//
//  Default stubs for the concrete implementations to override
//

void LightDriver::init(int ledcount, const char *param) {}
void LightDriver::draw() {}


//
//  Common code
//

void LightDriver::setPattern(unsigned char pattern[16]) {
	memcpy(oldpattern, pattern, 16);
	setBrightness(brightness); // Rescale to current intensity
}

void LightDriver::setBrightness(int percentage) {
	if(percentage < 0) {
		percentage=0;
	}
	if(percentage > 100) {
		percentage=100;
	}

	brightness = percentage;
	bright256 = (percentage * 255)/100;
	bright256 &= 0xff;

	memcpy(curpattern,oldpattern,16);  

	// Rescale intensity table to currentl brightness
	for(int ctr=0;ctr<16;ctr++) {
		curpattern[ctr]=colourscale[oldpattern[ctr]][bright256];
	}
}

void LightDriver::setMode(int mode) {
	lightmode = mode;
}

void LightDriver::setColour(uint32_t colour) {
	b=colour&0xff;
	g=(colour>>8)&0xff;
	r=(colour>>16)&0xff;
}

void LightDriver::update() {
	if(!framebuffer) {
		return;
	}

	switch(lightmode) {
		case LIGHTMODE_UNISON:
			update_unison(curpattern[ledpos & 15]);
			break;
		case LIGHTMODE_STOP:
			update_unison(bright256 & 255);
			break;
		default:
			update_normal();
	}

	ledpos++;
	ledpos &= 15;
}

void LightDriver::force(int percentage) {
	if(!framebuffer) {
		return;
	}
	update_unison(((percentage * 255)/100) & 255);
}


void LightDriver::update_normal() {
	unsigned char *ptr=framebuffer;
	unsigned char offset;

	for(int ctr=0;ctr<leds;ctr++)	{
		offset = (ctr+ledpos)&15;
		*ptr++=colourscale[curpattern[offset]][r];
		*ptr++=colourscale[curpattern[offset]][g];
		*ptr++=colourscale[curpattern[offset]][b];
	}
}

void LightDriver::update_unison(unsigned char offset) {
	unsigned char *ptr=framebuffer;

	for(int ctr=0;ctr<leds;ctr++)	{
		*ptr++=colourscale[offset][r];
		*ptr++=colourscale[offset][g];
		*ptr++=colourscale[offset][b];
	}
}

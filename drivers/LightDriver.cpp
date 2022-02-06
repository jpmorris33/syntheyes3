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
	brightness = percentage;
	memcpy(curpattern,oldpattern,16);  

	// Rescale intensity table to currentl brightness
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

	unsigned char *ptr=framebuffer;
	unsigned char offset;

	for(int ctr=0;ctr<leds;ctr++)	{
		if(lightmode == LIGHTMODE_UNISON) {
			offset = (ledpos & 15);
		} else {
			offset = (ctr+ledpos)&15;
		}
		*ptr++=colourscale[curpattern[offset]][r];
		*ptr++=colourscale[curpattern[offset]][g];
		*ptr++=colourscale[curpattern[offset]][b];
	}

	ledpos++;
	ledpos &= 15;
}


#include "LightDriver.hpp"

#include <string.h>

extern uint32_t rainbow[16]; // Colour table for gradients


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
	// Rescale intensity table
}

void LightDriver::setMode(int mode) {
	lightmode = mode;
}

void LightDriver::setColour(uint32_t rgb) {
	colour = rgb;
}

void LightDriver::update() {

	// Update LED table
}


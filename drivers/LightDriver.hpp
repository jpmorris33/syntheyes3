#ifndef __LIGHTDRIVER_HPP__
#define __LIGHTDRIVER_HPP__

#include <stdint.h>
#include "../gpio.hpp"

#define LIGHTMODE_NORMAL 0
#define LIGHTMODE_UNISON 1

class LightDriver {
	public:
		virtual void init(int ledcount, const char *param);
		virtual void draw();
		virtual void update();
		void setPattern(unsigned char pattern[16]);
		void setColour(uint32_t rgb);
		void setBrightness(int percentage);
		void setMode(int mode);
	protected:
		int leds;
		int ledpos;
		unsigned char r,g,b;
		int brightness;
		int lightmode;
		unsigned char *framebuffer;
		unsigned char oldpattern[16];
		unsigned char curpattern[16];
};

#endif

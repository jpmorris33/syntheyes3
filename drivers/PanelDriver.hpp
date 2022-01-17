#ifndef __PANELDRIVER_HPP__
#define __PANELDRIVER_HPP__

#include <stdint.h>

class PanelDriver {
	public:
		virtual void init();
		virtual void draw();
		virtual void drawMirrored();
		virtual void updateRGB(unsigned char *img, int w, int h);
		virtual void updateRGB(unsigned char *img, int w, int h, uint32_t colour);
		virtual void updateRGBpattern(unsigned char *img, int w, int h, int offset);
		virtual void setPattern(unsigned char pattern[16][16]);
		virtual void clear(uint32_t colour);
};

#endif
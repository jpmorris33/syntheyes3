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
		virtual void clearH(int y, uint32_t colour);
		virtual void clearV(int x, uint32_t colour);
		virtual uint32_t getCaps();
		int getW();
		int getH();
	protected:
		int panelW;
		int panelH;
};

#define PANELCAPS_SPLIT 0x00000001	// Requires separate Transmitter/Reciver units
#define PANELCAPS_FIXED 0x00000002	// Display is fixed size

#endif
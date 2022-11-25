#ifndef __PANELDRIVER_HPP__
#define __PANELDRIVER_HPP__

#include <stdint.h>
#include "../gpio.hpp"

class PanelDriver {
	public:
		virtual void init(const char *param);
		virtual void draw();
		virtual void drawMirrored();
		virtual void updateRGB(unsigned char *img, int w, int h);
		virtual void updateRGB(unsigned char *img, int w, int h, uint32_t colour);
		virtual void updateRGBpattern(unsigned char *img, int w, int h, int offset);
		virtual void setPattern(unsigned char pattern[16][16]);
		virtual void clear(uint32_t colour);
		virtual void clearH(int y, uint32_t colour);
		virtual void clearV(int x, uint32_t colour);
		virtual void setBrightness(int percentage);
		virtual uint32_t getCaps();
		virtual void rotate180(unsigned char *buffer, int w, int h);
		int getW();
		int getH();
	protected:
		int panelW;
		int panelH;
		unsigned char *framebuffer;
		unsigned char rainbowpattern[16][16];
};

extern const char *getDriverParam(const char *string, const char *cmd);
extern int getDriverInt(const char *param);
extern const char *getDriverStr(const char *param);
extern bool rotated180;
extern class Timing *get_timer();

#define PANELCAPS_SPLIT 	0x00000001	// Requires separate Transmitter/Reciver units
#define PANELCAPS_FIXED 	0x00000002	// Display is fixed size
#define PANELCAPS_MONOCHROME	0x00000004	// Display is monochrome

#endif

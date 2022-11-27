#ifndef __FONT_HPP__
#define __FONT_HPP__

//
//	Display arbitrary text on the panel, usually as a scrolling message
//

#include "drivers/PanelDriver.hpp"
#include "drivers/Timing.hpp"

class Font {
	public:
		Font();
		void scroll(const char *msg, int yoffset, uint32_t col, bool interruptible, bool gradient, bool mirror);
		void errorMsg(const char *msg);
		void errorMsg(const char *msg, const char *param);
		void errorMsg(const char *msg, const char *param, const char *param2);
		void errorMsg(const char *msg, int param);
		void printVersion(const char *version, bool transmitter, int duration);
		void print(const char *msg);

	private:
		void printMsg(const char *msg, unsigned char *outbuf, int w, int h, int x, int y);
		void printVersion16(const char *version, bool transmitter, int duration);
		unsigned char fontchar[128];
};

#endif
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
		void scroll(const char *msg, int yoffset, uint32_t col);
		void scroll(const char *msg, int yoffset, uint32_t col, bool gradient);
		void errorMsg(const char *msg);
		void errorMsg(const char *msg, const char *param);
		void errorMsg(const char *msg, int param);
		void printVersion(const char *version, bool transmitter, int duration);

	private:
		void printMsg(const char *msg, unsigned char *outbuf, int w, int h, int x, int y);
		unsigned char fontchar[128];
};

#endif
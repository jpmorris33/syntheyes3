#ifndef __WS2811LIGHTDRIVER_HPP__
#define __WS2811LIGHTDRIVER_HPP__
#ifdef WS2811_SUPPORT

//
//	This driver requires the following BSD-2 library:  https://github.com/jgarff/rpi_ws281x
//

#include <stdint.h>
#include "../LightDriver.hpp"
#include <ws2811/ws2811.h>

class WS2811Lights : public LightDriver {
	public:
		virtual void init(int ledcount, const char *param);
		virtual void draw();
	private:
		ws2811_t ws2811;
};

#endif
#endif

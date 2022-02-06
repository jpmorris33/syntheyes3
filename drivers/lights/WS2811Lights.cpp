#ifdef PLATFORM_PI
#ifdef WS2811_SUPPORT

/**
 * WS2811 LED string using Jeremy Garff's wpi_ws281x library
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <ws2811.h>

#include "SDLLights.hpp"

#include <stdio.h>
#include <string.h>

//
//	Init the WS2811 string driver
//
void WS2811Lights::init(int ledcount, const char *param) {

	leds=ledcount;
	ledpos=0;
	r=g=b=0x80;
	brightness = 100;
	lightmode = LIGHTMODE_NORMAL;

	if(leds < 0) {
		framebuffer=NULL;
		leds=0;
		return;
	}

	framebuffer = (unsigned char *)calloc(1,leds*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	ws2811.freq = 800000;
	ws2811.dmanum = 10,	// Do not use 5
	ws2811.channel = {
		[0] =	{
			.gpionum = 21,	// 18 is pin 12, 21 (PCM) is pin 40
			.count = leds,
			.invert = 0,
			.brightness = 255,
			.strip_type = WS2811_STRIP_RGB,
		},
		[1] = {
			.gpionum = 0,
			.count = 0,
			.invert = 0,
			.brightness = 0,
		},
	};

	int ret;
	if ((ret = ws2811_init(&ws2811)) != WS2811_SUCCESS) {
		fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
		exit(1);
	}

	setBrightness(100);

}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void WS2811Lights::draw() {

	if(!framebuffer) {
		return;
	}
	unsigned char *ptr=framebuffer;
	uint32_t rgb=0;

	for(int ctr=0;ctr<leds;ctr++) {
		rgb = (*ptr++)<<16;
		rgb |= (*ptr++)<<8;
		rgb |= (*ptr++);
		ws2811.channel[0].leds[ctr] = rgb;
	}

	int ret;
        if ((ret = ws2811_render(&ws2811)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            return;
        }


}




#endif
#endif

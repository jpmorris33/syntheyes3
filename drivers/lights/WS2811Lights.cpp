#ifdef PLATFORM_PI
#ifdef WS2811_SUPPORT

/**
 * WS2811 LED string using Jeremy Garff's wpi_ws281x library
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <ws2811/ws2811.h>

#include "WS2811Lights.hpp"

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

	printf("*WS2811: Init %d lights\n", ledcount);

	reserveSpecialPin(40);

	framebuffer = (unsigned char *)calloc(1,leds*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	ws2811.freq = 800000;
	ws2811.dmanum = 10,	// Do not use 5
	ws2811.channel[0].gpionum = 21;	// 18 is pin 12, 21 (PCM) is pin 40
	ws2811.channel[0].count = leds;
	ws2811.channel[0].invert = 0;
	ws2811.channel[0].brightness = 255;
	ws2811.channel[0].strip_type = WS2811_STRIP_GRB;
	ws2811.channel[1].gpionum = 0;
	ws2811.channel[1].count = 0;
	ws2811.channel[1].invert = 0;
	ws2811.channel[1].brightness = 0;

	ws2811_return_t ret;
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
		rgb = ((*ptr++)<<16);
		rgb |= ((*ptr++)<<8);
		rgb |= (*ptr++);
		ws2811.channel[0].leds[ctr] = rgb;
	}

	ws2811_return_t ret;
        if ((ret = ws2811_render(&ws2811)) != WS2811_SUCCESS)
        {
            fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
            return;
        }


}




#endif
#endif

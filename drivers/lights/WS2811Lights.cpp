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

static int parseStripType(const char *type);
extern int mapPinToGPIO(int pin);
static int checkGPIO(int gpioPin);

//
//	Init the WS2811 string driver
//
void WS2811Lights::init(int ledcount, const char *param) {

	leds=ledcount;
	ledpos=0;
	r=g=b=0x80;
	brightness = 100;
	bright256 = 255;
	lightmode = LIGHTMODE_NORMAL;

	if(leds < 0) {
		framebuffer=NULL;
		leds=0;
		return;
	}

	printf("*WS2811: Init %d lights\n", ledcount);

	framebuffer = (unsigned char *)calloc(1,leds*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	int striptype = WS2811_STRIP_GRB;
	const char *p = getDriverParam(param, "type");
	if(p) {
		p=getDriverStr(p);
		striptype = parseStripType(p);
	}

	int gpioPin = 40;  // Default is PCM pin
	p=getDriverParam(param,"gpio");
	if(p) {
		gpioPin = getDriverInt(p);
	}

	int gpioNum = checkGPIO(gpioPin);
	if(gpioNum < 0) {
		printf("*WS2811: Invalid GPIO pin %d\n", gpioPin);
		free(framebuffer);
		framebuffer=NULL;
		leds=0;
		return;
	}
	reserveSpecialPin(gpioPin);

	ws2811.freq = 800000;
	ws2811.dmanum = 10,	// Do not use 5 as it can corrupt the disk
	ws2811.channel[0].gpionum = gpioNum;
	ws2811.channel[0].count = leds;
	ws2811.channel[0].invert = 0;
	ws2811.channel[0].brightness = 255;
	ws2811.channel[0].strip_type = striptype;
	ws2811.channel[1].gpionum = 0;
	ws2811.channel[1].count = 0;
	ws2811.channel[1].invert = 0;
	ws2811.channel[1].brightness = 0;

	ws2811_return_t ret;
	if ((ret = ws2811_init(&ws2811)) != WS2811_SUCCESS) {
		fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
		free(framebuffer);
		framebuffer=NULL;
		leds=0;
		return;
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


int parseStripType(const char *type) {
	if(!strcasecmp(type,"rgb")) {
		return WS2811_STRIP_RGB;
	}
	if(!strcasecmp(type,"rbg")) {
		return WS2811_STRIP_RBG;
	}
	if(!strcasecmp(type,"grb")) {
		return WS2811_STRIP_GRB;
	}
	if(!strcasecmp(type,"gbr")) {
		return WS2811_STRIP_GBR;
	}
	if(!strcasecmp(type,"brg")) {
		return WS2811_STRIP_BRG;
	}
	if(!strcasecmp(type,"bgr")) {
		return WS2811_STRIP_BGR;
	}

	if(!strcasecmp(type,"rgbw")) {
		return SK6812_STRIP_RGBW;
	}
	if(!strcasecmp(type,"rbgw")) {
		return SK6812_STRIP_RBGW;
	}
	if(!strcasecmp(type,"grbw")) {
		return SK6812_STRIP_GRBW;
	}
	if(!strcasecmp(type,"gbrw")) {
		return SK6812_STRIP_GBRW;
	}
	if(!strcasecmp(type,"brgw")) {
		return SK6812_STRIP_BRGW;
	}
	if(!strcasecmp(type,"bgrw")) {
		return SK6812_STRIP_BGRW;
	}

	return WS2811_STRIP_GRB;

}

//
//	Only some GPIO pins are valid
//

int checkGPIO(int gpioPin) {
	int gpio = mapPinToGPIO(gpioPin);
	switch(gpio) {
		case 10: 
		case 12: 
		case 18: 
		case 21: 
			return gpio;
		default:
			return -1;
	};
}


#endif
#endif

/**
 * Render to WS2811 matrix pairs
 */

#include "WS2811PicoPanel.hpp"
#include "../Timing.hpp"
#include "ws2812.pio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NEOPANEL_W 16
#define NEOPANEL_H 8

#define REFRESH_MS 16 // approximately 1/60 second

extern int mapPinToGPIO(int pin);
static Timing *refresh;

//
//	Init the Pico WS2811 driver
//
void WS2811PicoPanel::init(const char *param) {

	int pin = 11;

	panelW = NEOPANEL_W;
	panelH = NEOPANEL_H;

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	outbuffer = (uint32_t *)calloc(sizeof(uint32_t),panelW*panelH*2); // both eyes
	if(!outbuffer) {
		printf("Failed to allocate output buffer\n");
		exit(1);
	}

	int gpio = mapPinToGPIO(pin);

	if(gpio < 0) {
		printf("*WS2811: Invalid GPIO pin %d\n", gpio);
		free(framebuffer);
		framebuffer=NULL;
		free(outbuffer);
		outbuffer=NULL;
		return;
	}

	reserveSpecialPin(pin);

	// Init IO state machine with WS2811 driver
	PIO pio = pio0;
	int sm = 0;
	uint offset = pio_add_program(pio, &ws2812_program);
	ws2812_program_init(pio, sm, offset, gpio, 800000, false); // false is for WGRB, may need driver param for that
	
	refresh = get_timer();
	refresh->set(0);
}

//
//	Get driver capabilities
//

uint32_t WS2811PicoPanel::getCaps() {
	return PANELCAPS_FIXED;
}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void WS2811PicoPanel::draw() {
	unsigned char *inptr = &framebuffer[0];
	uint32_t *outptr = outbuffer;
	uint32_t grb=0;

	int skipoffset = (panelW/2) * 3;	// Skip the other 8 RGB triplets

	// Panel 1

	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 2

	inptr = &framebuffer[24];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 3

	inptr = &framebuffer[0];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 4

	inptr = &framebuffer[24];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	if(rotated180) {
		rotate180(outbuffer,panelW*2,panelH);
	}

//	memset(outbuffer,0x80,0x80);

	// Save bandwidth by not always updating
	if(refresh->elapsed()) {
		refresh->set(REFRESH_MS);

		int size = panelW*panelH*2;
		uint32_t *outptr = outbuffer;
		for(int ctr=0;ctr<size;ctr++) {
			pio_sm_put_blocking(pio0, 0, *outptr++);
		}
	}
}

//
//	Put the framebuffer onto the Unicorn HD panel backwards
//
void WS2811PicoPanel::drawMirrored() {
	unsigned char *inptr = &framebuffer[0];
	uint32_t *outptr = outbuffer;
	uint32_t grb=0;

	int skipoffset = (panelW/2) * 3;	// Skip the other 8 RGB triplets

	// Panel 1

	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 2

	inptr = &framebuffer[24];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 3

	// FIXME: NOT MIRRORED YET!

	inptr = &framebuffer[0];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	// Panel 4

	inptr = &framebuffer[24];
	for(int y=0;y<panelH;y++) {
		for(int x=0;x<8;x++) {
			grb = (*inptr++) << 16;
			grb |= (*inptr++) << 24;
			grb |= (*inptr++) << 8;
			*outptr++ = grb;
		}
		inptr += skipoffset;
	}

	if(rotated180) {
		rotate180(outbuffer,panelW*2,panelH);
	}

//	memset(outbuffer,0x80,0x80);

	// Save bandwidth by not always updating
	if(refresh->elapsed()) {
		refresh->set(REFRESH_MS);

		int size = panelW*panelH*2;
		uint32_t *outptr = outbuffer;
		for(int ctr=0;ctr<size;ctr++) {
			pio_sm_put_blocking(pio0, 0, *outptr++);
		}
	}
}


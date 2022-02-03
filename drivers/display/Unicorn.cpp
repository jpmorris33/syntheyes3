#ifdef PLATFORM_PI

/**
 * Render to a Unicorn HD panel via SPI
 */

#include "Unicorn.hpp"
#include "../PosixTiming.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define UNICORNPANEL_W 16
#define UNICORNPANEL_H 16

#define REFRESH_MS 16 // approximately 1/60 second

static unsigned char spioutputbuf[769];
extern bool transmitter;
static PosixTiming refresh;

//
//	Init the Unicorn HD driver
//
void Unicorn::init(const char *param) {

	panelW = UNICORNPANEL_W;
	panelH = UNICORNPANEL_H;

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	// Reserve the SPI0 pins
	reserveOutputPin(19);	// MOSI
	reserveInputPin(21);	// MISO
	reserveOutputPin(23);	// CLK
	reserveOutputPin(24);	// CS

	wiringPiSetup();
	wiringPiSPISetup(0,9000000);
	refresh.set(0);
}

//
//	Get driver capabilities
//

uint32_t Unicorn::getCaps() {
	return PANELCAPS_SPLIT|PANELCAPS_FIXED;
}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void Unicorn::draw() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &spioutputbuf[0];
	*outptr++=0x72;   // for addressing multiple panels, it's 0x73 + zero-based panel address

	// cheat and memcpy the whole lot
	memcpy(outptr,inptr,768);

	// The panel gets fussy if it's updated more than about 60Hz
	if(refresh.elapsed()) {
		refresh.set(REFRESH_MS);
		wiringPiSPIDataRW(0,spioutputbuf,769);
	}
}

//
//	Put the framebuffer onto the Unicorn HD panel backwards
//
void Unicorn::drawMirrored() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &spioutputbuf[0];
	*outptr++=0x72;   // for addressing multiple panels, it's 0x73 + zero-based panel address

	int windowwidth = panelW * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<panelH;ctr++)  {
		for(int xpos=windowwidth-3;xpos>=0;xpos-=3) {
			memcpy(outptr,&inptr[xpos],3);
			outptr+=3;
		}
		inptr += windowwidth;
	}

	// The panel gets fussy if it's updated more than about 60Hz
	if(refresh.elapsed()) {
		refresh.set(REFRESH_MS);
		wiringPiSPIDataRW(0,spioutputbuf,769);
	}
}


#endif

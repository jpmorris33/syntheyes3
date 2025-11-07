#include "Hub75Pi.hpp"
#include "../../platforms/Platform.hpp"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gpio.hpp"

#ifdef PLATFORM_PI
#include <wiringPi.h>
#endif

#define HUBPANEL_W 64
#define HUBPANEL_H 32

#define MAX_WIDTH 256

#define LOGIC(a) (!(a))	// Logic is inverted

void *background_blit(void *drv);
void convert(unsigned char *rgbrow, unsigned char *bitstream, int len, unsigned char masterClock);
extern Platform *sys;

static GPIOPin *cs_A,*cs_B,*cs_C,*cs_D,*cs_E;
static GPIOPin *pin_r1, *pin_r2, *pin_g1, *pin_g2, *pin_b1, *pin_b2;
static GPIOPin *OE, *clk, *latch;

static unsigned char r1buf[MAX_WIDTH];
static unsigned char r2buf[MAX_WIDTH];
static unsigned char g1buf[MAX_WIDTH];
static unsigned char g2buf[MAX_WIDTH];
static unsigned char b1buf[MAX_WIDTH];
static unsigned char b2buf[MAX_WIDTH];

//
//	Set up the driver for displaying on Hub75 panels
//

void Hub75Pi::init(const char *param) {

	panelW = HUBPANEL_W;
	panelH = HUBPANEL_H;

	printf("*Init Hub75 panel driver\n");

	const char *p = getDriverParam(param, "w");
	if(p) {
		int w = getDriverInt(p);
		if(w == 32 || w == 64) {
			panelW = w;
		}
		printf("*Hub75 display set width to %d\n",panelW);
	}
	p = getDriverParam(param, "h");
	if(p) {
		int h = getDriverInt(p);
		if(h == 16 || h == 32) {
			panelH = h;
		}
		printf("*Hub75 display set height to %d\n",panelH);
	}

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);	// Single eye RGB
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	outbuffer = (unsigned char *)calloc(1,panelW*panelH*6);	// Both eyes
	if(!outbuffer) {
		printf("Failed to allocate outbuffer\n");
		exit(1);
	}

	outline = (unsigned char *)calloc(1,(panelW*6)/8);	// Reduce to a monochrome bitmap, so panel width *3 (RGB) *2 (two eyes) / 8 (reduced to bits)
	if(!outline) {
		printf("Failed to allocate output line buffer\n");
		exit(1);
	}

#ifdef PLATFORM_PI
	wiringPiSetup();
#endif
	cs_A = reserveOutputPin(15); // GPIO 22
	cs_B = reserveOutputPin(37); // GPIO 26
	cs_C = reserveOutputPin(13); // GPIO 27

	if(panelH > 16) {
		cs_D = reserveOutputPin(38); // GPIO 20
	}
	if(panelH > 32) {
		cs_E = reserveOutputPin(18); // GPIO 24
	}

	OE = reserveOutputPin(7); // GPIO 4
	clk = reserveOutputPin(11); // GPIO 17
	latch = reserveOutputPin(40); // GPIO 21

	pin_r1 = reserveOutputPin(29); // GPIO 5
	pin_r2 = reserveOutputPin(32); // GPIO 12
	pin_g1 = reserveOutputPin(33); // GPIO 13
	pin_g2 = reserveOutputPin(36); // GPIO 16
	pin_b1 = reserveOutputPin(31); // GPIO 6
	pin_b2 = reserveOutputPin(16); // GPIO 23

	// Run the display update thread in the background
	sys->background(&background_blit, this);


}

uint32_t Hub75Pi::getCaps() {
	return 0; // Not fixed, not monochrome
}


//
//	Convert the framebuffer to the format the Hub75 display engine wants
//	The background renderer will use it as and when
//
void Hub75Pi::draw() {
	unsigned char *inptr = framebuffer;
	unsigned char *outptr = outbuffer;
	int WWW = panelW * 3;

	// TODO: May need some kind of "wait for retrace" thing if we get tearing

	for(int y=0;y<panelH;y++) {
		unsigned char *start = inptr;
		for(int x=0;x<WWW;x++) {
			*outptr++ = *inptr++;
		}
		// Now do it again for the second panel
		inptr = start;
		for(int x=0;x<WWW;x++) {
			*outptr++ = *inptr++;
		}
	}
}

//
//	And again, but with the second panel mirrored
//

void Hub75Pi::drawMirrored() {
	unsigned char *inptr = framebuffer;
	unsigned char *outptr = outbuffer;
	int WWW = panelW * 3;

	// TODO: May need some kind of "wait for retrace" thing if we get tearing

	for(int y=0;y<panelH;y++) {
		unsigned char *start = inptr;
		for(int x=0;x<WWW;x++) {
			*outptr++ = *inptr++;
		}
		// Now walk backwards to mirror the image on the second panel
		for(int x=0;x<panelW;x++) {
			inptr-=3;
			*outptr++ = inptr[0];
			*outptr++ = inptr[1];
			*outptr++ = inptr[2];
		}

		// now fast-forward to the next row
		inptr = start + WWW;
	}
}

//
//	Put the framebuffer onto the HUB75 panels with PWM.
//	Ideally pwmClock cycles from 0-255 continuously to simulate 8-bit R,G,B components, but may need larger increments if we can't get the frame rates we need
//

void Hub75Pi::blit(unsigned char pwmClock) {
	int h = panelH / 2;
	int secondHalf = panelW * 6 * h;	// Off
	int wbits = (panelW * 6) / 8;

	unsigned char *ptr = outbuffer;
	unsigned char *ptr2 = outbuffer + secondHalf;
	for(int row=0;row<h;row++) {

		// Set the address

		cs_A->write(LOGIC(row & 1));
		cs_B->write(LOGIC(row & 2));
		cs_C->write(LOGIC(row & 4));
		if(cs_D) {
			cs_D->write(LOGIC(row & 8));
		}
		if(cs_E) {
			cs_E->write(LOGIC(row & 16));
		}

		OE->write(GPIO_LOGIC_LOW); // Set output enable low

		// Split out the RGB values into modulated bitstreams for each row
		convert(ptr, r1buf, panelW << 1, pwmClock);
		convert(ptr+1, g1buf, panelW << 1, pwmClock);
		convert(ptr+2, b1buf, panelW << 1, pwmClock);
		ptr += (panelW * 6);

		// Bottom half
		convert(ptr2, r2buf, panelW << 1, pwmClock);
		convert(ptr2+1, g2buf, panelW << 1, pwmClock);
		convert(ptr2+2, b2buf, panelW << 1, pwmClock);
		ptr2 += (panelW * 6);

		// Now it is time to transmit the bitstreams to the panels

		latch->write(GPIO_LOGIC_LOW); // low
		for(int pos=0;pos < wbits; pos++) {
			pin_r1->writeByte(r1buf[pos], clk);
			pin_r2->writeByte(r2buf[pos], clk);
			pin_g1->writeByte(g1buf[pos], clk);
			pin_g2->writeByte(g2buf[pos], clk);
			pin_b1->writeByte(b1buf[pos], clk);
			pin_b2->writeByte(b2buf[pos], clk);
		}
		latch->write(GPIO_LOGIC_HIGH); // high
	}
	OE->write(GPIO_LOGIC_HIGH); // Set output enable high
}

void convert(unsigned char *rgbrow, unsigned char *bitstream, int len, unsigned char masterClock) {
	int l8 = len / 8;
	unsigned char *ptr = rgbrow;
	unsigned char *out = bitstream;
	unsigned char outbyte;

	for(int pos=0;pos<l8;pos++) {
		outbyte=0;
		if((*ptr++) > masterClock) {
			outbyte |= 128;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 64;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 32;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 16;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 8;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 4;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 2;
		}
		ptr +=2; // Skip G, B
		if((*ptr++) > masterClock) {
			outbyte |= 1;
		}
		*out++=outbyte;
	}

}

//
//	This runs on the second core to offload processing from the main thread
//

void *background_blit(void *driver) {

	Hub75Pi *hubDriver = (Hub75Pi *)driver;

	// Run the blit process in the background forever.
	// If it were to stop the display would instantly go blank so 
	// rather than poll it, we have a tight loop on a second thread.
	while(1) {
		for(int ctr=0;ctr<255;ctr++) {
			hubDriver->blit(ctr);
		}
	}

	return NULL;
}

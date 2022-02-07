/**
 *   MAX7219 driver
 */

#ifdef PLATFORM_PI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "../PosixTiming.hpp"
#include "MAX7219Panel.hpp"

#define MAXPANEL_W 16
#define MAXPANEL_H 16

#define BRIGHTNESS	2	// 2/15 - default, usually changed later

#define CMD_TEST	0x0f
#define CMD_INTENSITY	0x0a
#define CMD_SCANLIMIT	0x0b
#define CMD_DECODE	0x09
#define CMD_SHUTDOWN	0x0C

// Panel order
#define RPANEL_TL  3
#define RPANEL_TR  2
#define RPANEL_BL  1
#define RPANEL_BR  0
#define LPANEL_TL  7
#define LPANEL_TR  6
#define LPANEL_BL  5
#define LPANEL_BR  4

#define REFRESH_MS 16	// approximately 1/60 sec

static void sendData(int addr, unsigned char opcode, unsigned char data);
static unsigned char rgb2bits(unsigned char *rgb);
static unsigned char rgb2bits_mirror(unsigned char *rgb);
extern void init_pin_output(int pin);

static PosixTiming refresh;
static unsigned char spioutputbuf[16];

static GPIOPin *chipSelect;

//
//	Init the Virtual display driver
//
void MAX7219Panel::init(const char *param) {
	int csPin = 24;

	printf("*Init MAX7219 driver\n");
	const char *p = getDriverParam(param, "cs");
	if(p) {
		csPin = getDriverInt(p);
		printf("*MAX7219 using CS pin %d\n",csPin);
	}


	panelW = MAXPANEL_W;
	panelH = MAXPANEL_H;

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	wiringPiSetup();
	wiringPiSPISetup(0,16000000);

	// Reserve some of the SPI0 pins
	reserveSpecialPin(19);			// MOSI
	reserveSpecialPin(23);			// CLK
	chipSelect = reserveOutputPin(csPin);	// CS
	chipSelect->write(false); // Actually sets it high since we use inverse logic

	// Initialise the panels
	for(int panel=0;panel<8;panel++) {
		sendData(panel, CMD_TEST,0);
		sendData(panel, CMD_DECODE,0);			// Disable BCD decoder so we can sent sprite data instead
		sendData(panel, CMD_INTENSITY,BRIGHTNESS);	// Maximum brightness
		sendData(panel, CMD_SCANLIMIT,7);
		sendData(panel, CMD_SHUTDOWN,1);		// 0 turns it off, 1 turns it on
	}

	refresh.set(0);

}

//
//	Get driver capabilities
//

uint32_t MAX7219Panel::getCaps() {
	return PANELCAPS_FIXED|PANELCAPS_MONOCHROME;
}

//
//	Set brightness
//

void MAX7219Panel::setBrightness(int percentage) {
	int bright = percentage / 6;
	if(bright < 0) {
		bright=0;
	}
	if(bright > 15) {
		bright=15;
	}

	for(int panel=0;panel<8;panel++) {
		sendData(panel, CMD_INTENSITY,bright);
	}
}

//
//	Put the framebuffer onto the MAX7219
//

void MAX7219Panel::draw() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char outbyte;

	if(!refresh.elapsed()) {
		// Reduce SPI bus traffic (and thus CPU usage)
		return;
	}
	refresh.set(REFRESH_MS);

	// Currently 16x16 fixed, come up with a more universal solution later
	
	// Top row
	for(int row=0;row<8;row++)  {
		// Left
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_TL,row+1,outbyte);
		sendData(LPANEL_TL,row+1,outbyte);

		// Right
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_TR,row+1,outbyte);
		sendData(LPANEL_TR,row+1,outbyte);
	}

	// Bottom row
	for(int row=0;row<8;row++)  {
		// Left
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_BL,row+1,outbyte);
		sendData(LPANEL_BL,row+1,outbyte);

		// Right
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_BR,row+1,outbyte);
		sendData(LPANEL_BR,row+1,outbyte);
	}
}

void MAX7219Panel::drawMirrored() {
	unsigned char *inptr = &framebuffer[0];
	unsigned char outbyte;
	unsigned char outbyteM;

	if(!refresh.elapsed()) {
		// Reduce SPI bus traffic (and thus CPU usage)
		return;
	}
	refresh.set(REFRESH_MS);

	// Currently 16x16 fixed, come up with a more universal solution later
	
	// Top row
	for(int row=0;row<8;row++)  {
		// Left
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_TL,row+1,outbyte);
		sendData(LPANEL_TR,row+1,outbyteM);

		// Right
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_TR,row+1,outbyte);
		sendData(LPANEL_TL,row+1,outbyteM);
	}

	// Bottom row
	for(int row=0;row<8;row++)  {
		// Left
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_BL,row+1,outbyte);
		sendData(LPANEL_BR,row+1,outbyteM);

		// Right
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(RPANEL_BR,row+1,outbyte);
		sendData(LPANEL_BL,row+1,outbyteM);
	}
}




//
//  Send a command or data to the display chip
//

void sendData(int addr, unsigned char opcode, unsigned char data) {
	int offset=addr*2;
	memset(spioutputbuf,0,16);

	// Data is put into the array in reverse order
	// because the chip wants it sent that way
	spioutputbuf[15-(offset+1)]=opcode;
	spioutputbuf[15-offset]=data;

	// Blit it to the display
	chipSelect->write(true); // LOW
	wiringPiSPIDataRW(0,spioutputbuf,16);
	chipSelect->write(false); // HIGH
}

unsigned char rgb2bits(unsigned char *rgb) {
	unsigned char out=0;

	for(int ctr=0;ctr<8;ctr++) {
		out <<=1;
		if(rgb[0] || rgb[1] || rgb[2]) {
			out |= 1;
		}
		rgb+=3;
	}
	return out;
}

unsigned char rgb2bits_mirror(unsigned char *rgb) {
	unsigned char out=0;

	for(int ctr=0;ctr<8;ctr++) {
		out >>=1;
		if(rgb[0] || rgb[1] || rgb[2]) {
			out |= 128;
		}
		rgb+=3;
	}
	return out;
}

#endif

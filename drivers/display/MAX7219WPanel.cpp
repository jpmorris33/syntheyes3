/**
 *   MAX7219 driver (32x16)
 */

#ifdef PLATFORM_PI

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "../PosixTiming.hpp"
#include "MAX7219WPanel.hpp"

#define MAXPANEL_W 32
#define MAXPANEL_H 16

#define BRIGHTNESS	2	// 2/15 - default, usually changed later

#define CMD_TEST	0x0f
#define CMD_INTENSITY	0x0a
#define CMD_SCANLIMIT	0x0b
#define CMD_DECODE	0x09
#define CMD_SHUTDOWN	0x0C

// Panel order
#define PANEL_TA  3
#define PANEL_TB  2
#define PANEL_TC  1
#define PANEL_TD  0
#define PANEL_BA  7
#define PANEL_BB  6
#define PANEL_BC  5
#define PANEL_BD  4

#define REFRESH_MS 16	// approximately 1/60 sec

static void sendData(int addr, unsigned char opcode, unsigned char data);
static void sendDataL(int addr, unsigned char opcode, unsigned char data);
static void sendDataR(int addr, unsigned char opcode, unsigned char data);
static unsigned char rgb2bits(unsigned char *rgb);
static unsigned char rgb2bits_mirror(unsigned char *rgb);
extern void init_pin_output(int pin);

static PosixTiming refresh;
static unsigned char spioutputbuf[16];

static GPIOPin *chipSelectL;
static GPIOPin *chipSelectR;

//
//	Init the wide MAX7219 panel
//
void MAX7219WPanel::init(const char *param) {
	int csPinL = 18;
	int csPinR = 22;

	printf("*Init MAX7219W driver\n");

	const char *p = getDriverParam(param, "csl");
	if(p) {
		csPinL = getDriverInt(p);
		printf("*MAX7219W using left CS pin %d\n",csPinL);
	}
	p = getDriverParam(param, "csr");
	if(p) {
		csPinR = getDriverInt(p);
		printf("*MAX7219W using right CS pin %d\n",csPinR);
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
	chipSelectL = reserveOutputPin(csPinL);	// CS
	chipSelectL->write(false); // Actually sets it high since we use inverse logic
	chipSelectR = reserveOutputPin(csPinR);	// CS
	chipSelectR->write(false); // Actually sets it high since we use inverse logic

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

uint32_t MAX7219WPanel::getCaps() {
	return PANELCAPS_FIXED|PANELCAPS_MONOCHROME;
}

//
//	Set brightness
//

void MAX7219WPanel::setBrightness(int percentage) {
	int bright = percentage / 6;
	if(bright < 0) {
		bright=0;
	}
	if(bright > 15) {
		bright=15;
	}

	for(int panel=0;panel<8;panel++) {
		sendData(panel, CMD_INTENSITY,bright);
		sendData(panel, CMD_INTENSITY,bright);
	}
}

//
//	Put the framebuffer onto the MAX7219
//

void MAX7219WPanel::draw() {
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
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_TA,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_TB,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_TC,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_TD,row+1,outbyte);
	}

	// Bottom row
	for(int row=0;row<8;row++)  {
		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_BA,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_BB,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_BC,row+1,outbyte);

		outbyte = rgb2bits(inptr);
		inptr += 24; // 8 RGB triplets
		sendData(PANEL_BD,row+1,outbyte);
	}
}

void MAX7219WPanel::drawMirrored() {
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
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_TA,row+1,outbyte);
		sendDataL(PANEL_TD,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_TB,row+1,outbyte);
		sendDataL(PANEL_TC,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_TC,row+1,outbyte);
		sendDataL(PANEL_TB,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_TD,row+1,outbyte);
		sendDataL(PANEL_TA,row+1,outbyteM);
	}

	// Bottom row
	for(int row=0;row<8;row++)  {
		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_BA,row+1,outbyte);
		sendDataL(PANEL_BD,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_BB,row+1,outbyte);
		sendDataL(PANEL_BC,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_BC,row+1,outbyte);
		sendDataL(PANEL_BB,row+1,outbyteM);

		outbyte = rgb2bits(inptr);
		outbyteM = rgb2bits_mirror(inptr);
		inptr += 24; // 8 RGB triplets
		sendDataR(PANEL_BD,row+1,outbyte);
		sendDataL(PANEL_BA,row+1,outbyteM);
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

	// Blit it to both displays
	chipSelectL->write(true); // LOW
	chipSelectR->write(true); // LOW
	wiringPiSPIDataRW(0,spioutputbuf,16);
	chipSelectL->write(false); // HIGH
	chipSelectR->write(false); // HIGH
}

void sendDataL(int addr, unsigned char opcode, unsigned char data) {
	int offset=addr*2;
	memset(spioutputbuf,0,16);

	// Data is put into the array in reverse order
	// because the chip wants it sent that way
	spioutputbuf[15-(offset+1)]=opcode;
	spioutputbuf[15-offset]=data;

	// Blit it to the display
	chipSelectL->write(true); // LOW
	wiringPiSPIDataRW(0,spioutputbuf,16);
	chipSelectL->write(false); // HIGH
}

void sendDataR(int addr, unsigned char opcode, unsigned char data) {
	int offset=addr*2;
	memset(spioutputbuf,0,16);

	// Data is put into the array in reverse order
	// because the chip wants it sent that way
	spioutputbuf[15-(offset+1)]=opcode;
	spioutputbuf[15-offset]=data;

	// Blit it to the display
	chipSelectR->write(true); // LOW
	wiringPiSPIDataRW(0,spioutputbuf,16);
	chipSelectR->write(false); // HIGH
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

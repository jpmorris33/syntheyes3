/**
 *   MAX7219 driver
 */

#ifdef PLATFORM_PI

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "../PosixTiming.hpp"
#include "MAX7219Panel.hpp"

#define CS_PIN		10	// Pin 24 in HW
#define BRIGHTNESS	12	// 12/15

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

extern uint32_t rainbow[16]; // Colour table
static void sendData(int addr, unsigned char opcode, unsigned char data);
static unsigned char rgb2bits(unsigned char *rgb);
static unsigned char rgb2bits_mirror(unsigned char *rgb);
extern void init_pin_output(int pin);

static unsigned char rainbowpattern[16][16];
static PosixTiming refresh;
static unsigned char spioutputbuf[16];

//
//	Init the Virtual display driver
//
void MAX7219Panel::init() {

	printf("Init MAX7219 driver\n");

	panelW = MAXPANEL_W;
	panelH = MAXPANEL_H;

	// Init display

	// Set up data transfers

	wiringPiSetup();
	wiringPiSPISetup(0,16000000);
	init_pin_output(CS_PIN);
	digitalWrite(CS_PIN, HIGH);

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



void MAX7219Panel::updateRGB(unsigned char *img, int w, int h) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<panelW;x++) {
			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = in[0];
					out[1] = in[1];
					out[2] = in[2];
				}
				in +=3;
			}
			out+=3;
		}
	}
}


void MAX7219Panel::updateRGB(unsigned char *img, int w, int h, uint32_t colour) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<panelW;x++) {
			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = r;
					out[1] = g;
					out[2] = b;
				}
				in +=3;
			}
			out+=3;
		}
	}
}

void MAX7219Panel::updateRGBpattern(unsigned char *img, int w, int h, int offset) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;
	int index=0,xpos=0,ypos=0;
	unsigned char r,g,b;

	ypos=0;
	for(int y=0;y<panelH;y++) {
		in=&img[(w*3)*y];
		xpos=0;
		for(int x=0;x<panelW;x++) {
			index = (offset + (rainbowpattern[ypos][xpos]&0x0f))&0x0f;
			b=rainbow[index]&0xff;
			g=(rainbow[index]>>8)&0xff;
			r=(rainbow[index]>>16)&0xff;
			xpos++;
			xpos &= 0x0f; // Constrain to 16 pixels

			if(x<w && y<h) {
				if(in[0]|in[1]|in[2]) {
					out[0] = r;
					out[1] = g;
					out[2] = b;
				}
				in +=3;
			}
			out+=3;
		}

		ypos++;
		ypos &= 0x0f; // Constrain to 16 pixels

	}
}

//
//	Set the special effect pattern
//
void MAX7219Panel::setPattern(unsigned char pattern[16][16]) {
	memcpy(rainbowpattern, pattern, 16*16);
}

void MAX7219Panel::clear(uint32_t colour) {
	int len=panelW*panelH;
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	for(int ctr=0;ctr<len;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
	}
}

void MAX7219Panel::clearV(int x, uint32_t colour) {
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	int offset = (panelW-1)*3;

	if(x<0 || x >= panelW) {
		return;
	}

	ptr += (x*3);  // Find the column

	for(int ctr=0;ctr<panelH;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
		ptr+=offset;
	}
}

void MAX7219Panel::clearH(int y, uint32_t colour) {
	unsigned char *ptr=&framebuffer[0];

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	if(y<0 || y >= panelH) {
		return;
	}

	ptr += ((y*panelW)*3);  // Find the row

	for(int ctr=0;ctr<panelW;ctr++) {
		*ptr++=r;
		*ptr++=g;
		*ptr++=b;
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
	digitalWrite(CS_PIN,LOW);
	wiringPiSPIDataRW(0,spioutputbuf,16);
	digitalWrite(CS_PIN,HIGH);
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

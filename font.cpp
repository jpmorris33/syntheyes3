#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "font.hpp"

extern Timing *timing;
extern PanelDriver *panel;
extern class Expression *nextExpression;
extern bool check_pin(int pin);
extern void wait(int ms, bool interruptable);
extern unsigned char rainbowoffset;

static unsigned char fontimg[][8] = {
	// 0 - zero
	{
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000010,
	0b00000110,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000001,
	0b00000010,
	0b00000100,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000110,
	0b00000001,
	0b00000110,
	0b00000001,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000111,
	0b00000001,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000000,
	},
	{
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000010,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000011,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000000,
	},
	// 10 - space
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 11 - A
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000110,
	0b00000101,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000111,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000111,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000101,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000111,
	0b00000001,
	0b00000001,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000111,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000111,
	0b00000001,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000111,
	0b00000000,
	},
	// 37 - full stop
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000000,
	},
	// 38 - comma
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000100,
	},
	// 39 - dash
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000111,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 40 - slash
	{
	0b00000000,
	0b00000001,
	0b00000001,
	0b00000010,
	0b00000010,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	// 41 - colon
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000000,
	0b00000010,
	0b00000000,
	0b00000000,
	},
	// 42 - semicolon
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000000,
	0b00000010,
	0b00000100,
	0b00000000,
	},
	// 43 - dollar
	{
	0b00000000,
	0b00000010,
	0b00000011,
	0b00000110,
	0b00000011,
	0b00000110,
	0b00000010,
	0b00000000,
	},
	// 44 - (
	{
	0b00000000,
	0b00000010,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000010,
	0b00000000,
	},
	// 45 - )
	{
	0b00000000,
	0b00000010,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000010,
	0b00000000,
	},
	// 46 - [
	{
	0b00000000,
	0b00000110,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000110,
	0b00000000,
	},
	// 47 - ]
	{
	0b00000000,
	0b00000011,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000011,
	0b00000000,
	},
	// 48 - {
	{
	0b00000000,
	0b00000001,
	0b00000010,
	0b00000110,
	0b00000010,
	0b00000010,
	0b00000001,
	0b00000000,
	},
	// 49 - }
	{
	0b00000000,
	0b00000100,
	0b00000010,
	0b00000011,
	0b00000010,
	0b00000010,
	0b00000100,
	0b00000000,
	},
	// 50 - underscore
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000111,
	0b00000000,
	},
	// 51 - plus
	{
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000111,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 52 - hash
	{
	0b00000000,
	0b00000000,
	0b00000101,
	0b00001111,
	0b00000101,
	0b00001111,
	0b00000101,
	0b00000000,
	},
	// 53 - apostrophe
	{
	0b00000000,
	0b00000010,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 54 - quote
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 55 - exclamation
	{
	0b00000000,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000000,
	0b00000010,
	0b00000000,
	},
	// 56 - question
	{
	0b00000000,
	0b00000110,
	0b00000001,
	0b00000001,
	0b00000010,
	0b00000000,
	0b00000010,
	0b00000000,
	},
	// 57 - <
	{
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000100,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 58 - >
	{
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000001,
	0b00000010,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 59 - *
	{
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000010,
	0b00000101,
	0b00000000,
	0b00000000,
	0b00000000,
	},
	// 60 - %
	{
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000001,
	0b00000010,
	0b00000100,
	0b00000101,
	0b00000000,
	},
	// 61 - a
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000101,
	0b00000011,
	0b00000000,
	},
	{
	0b00000000,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000110,
	0b00000101,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000100,
	0b00000011,
	0b00000000,
	},
	{
	0b00000000,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000011,
	0b00000101,
	0b00000011,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000111,
	0b00000100,
	0b00000011,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000100,
	0b00000110,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000101,
	0b00000011,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000000,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000001,
	0b00000000,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000101,
	0b00000110,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000010,
	0b00000001,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000111,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000110,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000010,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000110,
	0b00000101,
	0b00000110,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	{
	0b00000000,
	0b00000011,
	0b00000101,
	0b00000011,
	0b00000001,
	0b00000001,
	0b00000001,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000111,
	0b00000100,
	0b00000100,
	0b00000100,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000011,
	0b00000100,
	0b00000010,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000100,
	0b00000110,
	0b00000100,
	0b00000100,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000011,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000111,
	0b00000111,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000010,
	0b00000101,
	0b00000000,
	},
	{
	0b00000000,
	0b00000101,
	0b00000101,
	0b00000011,
	0b00000001,
	0b00000001,
	0b00000110,
	0b00000000,
	},
	{
	0b00000000,
	0b00000000,
	0b00000111,
	0b00000001,
	0b00000010,
	0b00000100,
	0b00000111,
	0b00000000,
	},
};

Font::Font() {
	memset(fontchar,10,128); // Default to space
	// Patch in numbers
	for(int ctr=0;ctr<10;ctr++) {
		fontchar['0'+ctr]=ctr;
	}
	// Patch in uppercase
	for(int ctr=0;ctr<26;ctr++) {
		fontchar['A'+ctr]=11+ctr;
	}
	// Patch in lowercase
	for(int ctr=0;ctr<26;ctr++) {
		fontchar['a'+ctr]=61+ctr;
	}
	// Now the punctuation and other symbols
	fontchar['.']=37;
	fontchar[',']=38;
	fontchar['-']=39;
	fontchar['/']=40;
	fontchar[':']=41;
	fontchar[';']=42;
	fontchar['$']=43;
	fontchar['(']=44;
	fontchar[')']=45;
	fontchar['[']=46;
	fontchar[']']=47;
	fontchar['{']=48;
	fontchar['}']=49;
	fontchar['_']=50;
	fontchar['+']=51;
	fontchar['#']=52;
	fontchar['\'']=53;
	fontchar['\"']=54;
	fontchar['!']=55;
	fontchar['?']=56;
	fontchar['<']=57;
	fontchar['>']=58;
	fontchar['*']=59;
	fontchar['%']=60;
}

void Font::printMsg(const char *msg, unsigned char *outbuf, int w, int h, int x, int y) {
	unsigned char *outptr = &outbuf[0];
	unsigned char fontrow;

	int maxlen = (w/4) + 1; // Add an extra character for smooth scrolling
	int maxy=8+y;
	int len = strlen(msg);
	int index=0;
	int pos;

	// If it won't fit in the buffer, trim it down
	if(len > maxlen) {
		len=maxlen;
	}
	if(maxy>h) {
		maxy=h;
	}

	for(int ctr=0;ctr<len;ctr++) {
		for(int yctr=y;yctr<maxy;yctr++) {
			fontrow = fontimg[fontchar[(unsigned char)msg[ctr]&127]][yctr-y];
			for(int xctr=0;xctr<4;xctr++) {
				pos=x+xctr+(ctr<<2);
				if(pos>=0 && pos<w) {
					if(fontrow & 0x08) {
						index = (pos + (yctr*w))*3;
						outptr[index] = 0xff;
						outptr[index+1] = 0xff;
						outptr[index+2] = 0xff;
					}
				}
				fontrow<<=1; // Bitshift the character to get the next column
			}
		}
	}

}

void Font::scroll(const char *msg, int yoffset, uint32_t col, bool interruptible, bool gradient) {
	unsigned char bmp[16*16*3];

	char fullmsg[2048];
	memset(fullmsg,0,sizeof(fullmsg));
	strcpy(fullmsg,"    ");
	strncat(fullmsg,msg,2040);
	strcat(fullmsg,"    ");

	int len = strlen(fullmsg);

	for(int ctr=0;ctr<len;ctr++) {
		for(int delay=0;delay<4;delay++) {
			memset(bmp,0,sizeof(bmp));
			panel->clear(0);
			printMsg(&fullmsg[ctr],&bmp[0],16,16,-delay,yoffset);
			if(gradient) {
				panel->updateRGBpattern(bmp, 16, 16, rainbowoffset);
			} else {
				panel->updateRGB(bmp, 16, 16, col);
			}
			panel->draw();
			wait(20,false);

			if(interruptible && nextExpression) {
				return;
			}
		}
	}
}

void Font::errorMsg(const char *msg) {
	for(;;) {
		scroll(msg,4,0xff1010,false,false);
	}
}

void Font::errorMsg(const char *msg, const char *param) {
	char text[2048];
	snprintf(text,2047,msg,param);
	text[2047]=0;

	puts(text);  // To console


	for(;;) {
		scroll(text,4,0xff1010,false,false);
	}
}

void Font::errorMsg(const char *msg, const char *param1, const char *param2) {
	char text[2048];
	snprintf(text,2047,msg,param1,param2);
	text[2047]=0;

	puts(text);  // To console


	for(;;) {
		scroll(text,4,0xff1010,false,false);
	}
}

void Font::errorMsg(const char *msg, int param) {
	char text[2048];
	snprintf(text,2047,msg,param);
	text[2047]=0;

	puts(text);  // To console


	for(;;) {
		scroll(text,4,0xff1010,false,false);
	}
}

void Font::printVersion(const char *version, bool transmitter, int duration) {
	unsigned char bmp[16*16*3];
	unsigned short splash_transmit[16] = {
	    0b1101010110111101, 
	    0b1001010101010101, 
	    0b1101010101010111, 
	    0b0100100101010101, 
	    0b1100100101010101, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0000000000000000, 
	    0b0101011111010111, 
	    0b0010010101010010, 
	    0b0101010001010010, 
	};
	unsigned short splash_receive[16] = {
	    0b1101010110111101,
	    0b1001010101010101,
	    0b1101010101010111,
	    0b0100100101010101,
	    0b1100100101010101,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000000000000000,
	    0b0000110110101000,
	    0b0000100100101000,
	    0b0000100110010000,
	};
	unsigned short *imgsrc = transmitter ? &splash_transmit[0] : &splash_receive[0];
	unsigned char *ptr = &bmp[0];
	memset(ptr,0,sizeof(bmp));

	for(int y=0;y<16;y++) {
		unsigned short imgbit = imgsrc[y];
		for(int x=0;x<16;x++) {
			if(imgbit & 0x8000) {
				ptr[0]=0xff;
				ptr[1]=0xff;
				ptr[2]=0xff;
			}
			ptr += 3;
			imgbit <<=1;
		}
	}

	printMsg(version,&bmp[0],16,16,0,5);
	panel->updateRGB(bmp, 16, 16, 0x808080);

	for(int ctr=0;ctr<duration;ctr++) {
		panel->draw();
		timing->wait_microseconds(1000);
	}
}

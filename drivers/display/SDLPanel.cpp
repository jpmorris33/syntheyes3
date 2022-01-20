#ifdef PLATFORM_LINUX

/**
 * Dummy unicorn panel driver with no SPI transfer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
unsigned char spioutputbuf[769];

//
//	Init the SDL driver
//
#include <SDL2/SDL.h>

#include "SDLPanel.hpp"

#include <stdio.h>
#include <string.h>

static SDL_Window *win;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static unsigned char outbuf[768];

extern uint32_t rainbow[16]; // Colour table
static unsigned char rainbowpattern[16][16];

//
//	Init the Unicorn HD driver
//
void SDLPanel::init() {

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

	win = SDL_CreateWindow("",0,0,160,160,SDL_WINDOW_RESIZABLE);

	if (!win){
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(win, -1, 0);
	if (!renderer){
		fprintf(stderr, "Count not open renderer, aborting\n");
		exit(1);
	}

	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,16,16);
	if (!texture){
		fprintf(stderr, "Failed to create texture\n");
		exit(1);
	}

	SDL_SetRenderDrawColor( renderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
	SDL_RenderClear( renderer );
}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void SDLPanel::draw() {

	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &outbuf[0];

	int windowwidth = SDLPANEL_W * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<SDLPANEL_H;ctr++)  {
		memcpy(outptr,inptr,windowwidth);
		inptr += windowwidth;
		outptr += windowwidth;
	}

	SDL_UpdateTexture(texture, NULL, &outbuf[0], windowwidth);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

}

void SDLPanel::drawMirrored() {

	int w, h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	unsigned char *inptr = &framebuffer[0];
	unsigned char *outptr = &outbuf[0];

	int windowwidth = SDLPANEL_W * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<SDLPANEL_H;ctr++)  {
		for(int xpos=windowwidth-3;xpos>=0;xpos-=3) {
			memcpy(outptr,&inptr[xpos],3);
			outptr+=3;
		}
		inptr += windowwidth;
	}

	SDL_UpdateTexture(texture, NULL, &outbuf[0], windowwidth);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

}



void SDLPanel::updateRGB(unsigned char *img, int w, int h) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	for(int y=0;y<SDLPANEL_H;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<SDLPANEL_W;x++) {
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


void SDLPanel::updateRGB(unsigned char *img, int w, int h, uint32_t colour) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;

	unsigned char b=colour&0xff;
	unsigned char g=(colour>>8)&0xff;
	unsigned char r=(colour>>16)&0xff;

	for(int y=0;y<SDLPANEL_H;y++) {
		in=&img[(w*3)*y];
		for(int x=0;x<SDLPANEL_W;x++) {
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

void SDLPanel::updateRGBpattern(unsigned char *img, int w, int h, int offset) {
	unsigned char *out = &framebuffer[0];
	unsigned char *in = img;
	int index=0,xpos=0,ypos=0;
	unsigned char r,g,b;

	ypos=0;
	for(int y=0;y<SDLPANEL_H;y++) {
		in=&img[(w*3)*y];
		xpos=0;
		for(int x=0;x<SDLPANEL_W;x++) {
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
void SDLPanel::setPattern(unsigned char pattern[16][16]) {
	memcpy(rainbowpattern, pattern, 16*16);
}

void SDLPanel::clear(uint32_t colour) {
	int len=SDLPANEL_W*SDLPANEL_H;
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
#endif
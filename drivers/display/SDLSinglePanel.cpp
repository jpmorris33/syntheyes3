#ifdef PLATFORM_LINUX

/**
 * Virtual two-head LED display panel using SDL
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

//
//	Init the SDL driver
//
#include <SDL2/SDL.h>

#include "SDLSinglePanel.hpp"

#include <stdio.h>
#include <string.h>

#define SDLPANEL_W 16
#define SDLPANEL_H 16

//
//	Init the Virtual display driver
//
void SDLSinglePanel::init(const char *param) {

	panelW = SDLPANEL_W;
	panelH = SDLPANEL_H;

	printf("*Init SDL combined panel driver\n");

	const char *p = getDriverParam(param, "w");
	if(p) {
		int w = getDriverInt(p);
		if(w == 16 || w == 32 || w == 64) {
			panelW = w;
		}
		printf("*SDL virtual display set width to %d\n",panelW);
	}
	p = getDriverParam(param, "h");
	if(p) {
		int h = getDriverInt(p);
		if(h == 16 || h == 32 || h == 64) {
			panelH = h;
		}
		printf("*SDL virtual display set height to %d\n",panelH);
	}

	framebuffer = (unsigned char *)calloc(1,panelW*panelH*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	outbuffer = (unsigned char *)calloc(1,panelW*2*panelH*3);
	if(!outbuffer) {
		printf("Failed to allocate outbuffer\n");
		exit(1);
	}

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);

	win = SDL_CreateWindow("",0,0,panelW*20,panelH*10,SDL_WINDOW_RESIZABLE);

	if (!win){
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(win, -1, 0);
	if (!renderer){
		fprintf(stderr, "Count not open renderer, aborting\n");
		exit(1);
	}

	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,panelW*2,panelH);
	if (!texture){
		fprintf(stderr, "Failed to create texture\n");
		exit(1);
	}

	SDL_SetRenderDrawColor( renderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
	SDL_RenderClear( renderer );
}

//
//	Get driver capabilities
//

uint32_t SDLSinglePanel::getCaps() {
	return PANELCAPS_FIXED;
}


//
//	Put the framebuffer onto the Unicorn HD panel
//
void SDLSinglePanel::draw() {

	unsigned char *inptr = framebuffer;
	unsigned char *outptr = outbuffer;

	int windowwidth = panelW * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<panelH;ctr++)  {
		// Left
		memcpy(outptr,inptr,windowwidth);
		outptr += windowwidth;
		// Right
		memcpy(outptr,inptr,windowwidth);
		outptr += windowwidth;
		inptr += windowwidth;
	}

	if(rotated180) {
		rotate180(outbuffer,panelW*2,panelH);
	}

	SDL_UpdateTexture(texture, NULL, outbuffer, windowwidth*2);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

}

void SDLSinglePanel::drawMirrored() {

	unsigned char *inptr = framebuffer;
	unsigned char *outptr = outbuffer;

	int windowwidth = panelW * 3;	// 16 RGB triplets

	for(int ctr=0;ctr<panelH;ctr++)  {
		// Left
		for(int xpos=windowwidth-3;xpos>=0;xpos-=3) {
			memcpy(outptr,&inptr[xpos],3);
			outptr+=3;
		}
		// Right
		memcpy(outptr,inptr,windowwidth);
		outptr += windowwidth;
		inptr += windowwidth;
	}

	if(rotated180) {
		rotate180(outbuffer,panelW*2,panelH);
	}

	SDL_UpdateTexture(texture, NULL, outbuffer, windowwidth*2);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

}




#endif

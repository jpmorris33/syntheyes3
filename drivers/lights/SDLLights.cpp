#ifdef PLATFORM_LINUX

/**
 * Virtual LED string using SDL
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "SDLLights.hpp"

#include <stdio.h>
#include <string.h>

//
//	Init the Virtual display driver
//
void SDLLights::init(int ledcount, const char *param) {

	leds=ledcount;
	ledpos=0;
	r=g=b=0x80;
	brightness = 100;
	lightmode = LIGHTMODE_NORMAL;

	if(leds < 1) {
		framebuffer=NULL;
		leds=0;
		return;
	}

	printf("*Init SDL lights driver with %d LEDs\n",leds);

	framebuffer = (unsigned char *)calloc(1,leds*3);
	if(!framebuffer) {
		printf("Failed to allocate framebuffer\n");
		exit(1);
	}

	win = SDL_CreateWindow("x",0,0,leds*10,10,SDL_WINDOW_RESIZABLE);

	if (!win){
		printf("Failed to open window: %s\n", SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(win, -1, 0);
	if (!renderer){
		fprintf(stderr, "Count not open renderer, aborting\n");
		exit(1);
	}

	texture = SDL_CreateTexture(renderer,SDL_PIXELFORMAT_RGB24,SDL_TEXTUREACCESS_STREAMING,leds,1);
	if (!texture){
		fprintf(stderr, "Failed to create texture\n");
		exit(1);
	}

	SDL_SetRenderDrawColor( renderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
	SDL_RenderClear( renderer );

	int w,h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);

	setBrightness(100);

}

//
//	Put the framebuffer onto the Unicorn HD panel
//
void SDLLights::draw() {

	if(!framebuffer) {
		return;
	}

	int w,h;
	SDL_QueryTexture(texture, NULL, NULL, &w, &h);
	SDL_UpdateTexture(texture, NULL, framebuffer, leds*3);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

}





#endif

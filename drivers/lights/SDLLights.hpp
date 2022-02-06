#ifndef __SDLLIGHTDRIVER_HPP__
#define __SDLLIGHTDRIVER_HPP__

#include <stdint.h>
#include <SDL2/SDL.h>
#include "../LightDriver.hpp"

class SDLLights : public LightDriver {
	public:
		virtual void init(int ledcount, const char *param);
		virtual void draw();
	private:
		SDL_Window *win;
		SDL_Renderer *renderer;
		SDL_Texture *texture;
};

#endif

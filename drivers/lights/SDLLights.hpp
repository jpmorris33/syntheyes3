#ifndef __SDLLIGHTDRIVER_HPP__
#define __SDLLIGHTDRIVER_HPP__

#include <stdint.h>
#include "../LightDriver.hpp"

class SDLLights : public LightDriver {
	public:
		virtual void init(int ledcount, const char *param);
		virtual void draw();
	private:
		struct SDL_Window *win;
		struct SDL_Renderer *renderer;
		struct SDL_Texture *texture;
};

#endif

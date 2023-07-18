#ifdef PLATFORM_LINUX
#include <SDL2/SDL.h>

#include "SDLSensor.hpp"

void SDLSensor::init(const char *param) {
	printf("*Init SDL keyboard sensor\n");
}

bool SDLSensor::check() {
	int got=0;
	SDL_PumpEvents();
	const unsigned char *keys = SDL_GetKeyboardState(NULL);

	for(int ctr=0;ctr<8;ctr++) {
		channelmap[ctr]=keys[SDL_SCANCODE_A + ctr];
		got |= channelmap[ctr];
	}
	return got;
}


bool SDLSensor::isChannel(int channel) {

	if (channel < 0 || channel > 7) {
		return false;
	}

	return channelmap[channel];
}

#endif
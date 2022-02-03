#ifndef PLATFORM_PI

//
//  Stubs for pi emulation
//

#include "syntheyes.hpp"
#include "drivers/display/SDLPanel.hpp"
#include "drivers/display/SDLSinglePanel.hpp"
#include "drivers/serial/VirtualSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"
#include <SDL2/SDL.h>

extern PanelDriver *panel;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern SerialDriver *serial;

void initPanel(const char *driver, const char *params) {
	if(panel) {
		return;
	}

	// Initialise any other drivers here

	if(!strcasecmp(driver, "SDLSingle")) {
		panel = new SDLSinglePanel();
		panel->init(params);
	}

}

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	ack = new PosixTiming();
	gradient = new PosixTiming();
	serial = new VirtualSerialDriver();

	if(!panel) {
		panel = new SDLPanel();
		panel->init("");
	}

	// Fake serial port file
	strcpy(serialPort,"/tmp/_eyetmp_");

}


void pi_init() {
}

int mapPin(int pin) {
	switch(pin)	{
		case 40:
			return 29;
		//   39 is ground
		case 38:
			return 28;
		case 37:
			return 25;
		case 36:
			return 27;
		case 35:
			return 24;
		//   34 is ground
		case 33:
			return 23;
		case 32:
			return 26;
		case 31:
			return 22;
		//   30 is ground
		case 29:
			return 21;
		default:
			return -1;
	};
}

void init_pin_input(int pin) {
}

void init_pin_output(int pin) {
}

bool check_pin(int pin) {
	SDL_PumpEvents();
	const unsigned char *keys = SDL_GetKeyboardState(NULL);
	if(keys[SDL_SCANCODE_ESCAPE]) {
		exit(1);
	}

	// Negative pins are for polling ESC
	if(pin < 0) {
		return false;
	}

	// Otherwise you can fake GPIO with keys 1-9
	if (keys[SDL_SCANCODE_1 + (pin-21)]) {
		return true;
	}

	return false;
}

void set_pin(int pin, bool state) {
	if(pin != 29) { // No ACK pin spam
		printf("Set GPIO pin %d to %d\n",pin,state);
	}
}

void poll_keyboard() {
	check_pin(-666);  // Check fake GPIO
}

#endif

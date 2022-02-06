#ifndef PLATFORM_PI

//
//  Stubs for pi emulation
//

#include "syntheyes.hpp"
#include "colourutils.hpp"
#include "drivers/display/SDLPanel.hpp"
#include "drivers/display/SDLSinglePanel.hpp"
#include "drivers/lights/SDLLights.hpp"
#include "drivers/serial/VirtualSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"
#include <SDL2/SDL.h>

extern PanelDriver *panel;
extern LightDriver *lights;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern Timing *lighttimer;
extern SerialDriver *serial;
extern bool forcetransmitter;

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
	lighttimer = new PosixTiming();
	serial = new VirtualSerialDriver();

	if(!panel) {
		panel = new SDLPanel();
		panel->init("");
	}

	lights = new SDLLights();
	lights->init(8,"");
	lights->setColour(lightcolour);
	lights->setPattern(lightpattern_triangle);

	// Fake serial port file
	strcpy(serialPort,"/tmp/_eyetmp_");

	if(!(panel->getCaps() & PANELCAPS_SPLIT)) {
		forcetransmitter=true; // Single systems are always the transmitter
	}

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
		case 28:
			return 31;
		case 27:
			return 30;
		case 26:
			return 11;
		//   25 is ground
		case 24:
			return 10;
		case 23:
			return 14;
		case 22:
			return 6;
		case 21:
			return 13;
		//   20 is ground
		case 19:
			return 12;
		case 18:
			return 5;
		//   17 is +3v
		case 16:
			return 4;
		case 15:
			return 3;
		//   14 is ground
		case 13:
			return 2;
		case 12:
			return 1;
		case 11:
			return 0;
		case 10:
			return 16;
		//   9 is ground
		case 8:
			return 15;
		case 7:
			return 7;
		//   6 is ground
		case 5:
			return 9;
		//   4 is +5v
		case 3:
			return 8;
		//   2 is +5v
		//   1 is +3v
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

#ifndef PLATFORM_PI

//
//  Stubs for pi emulation
//

#include "syntheyes.hpp"
#include "colourutils.hpp"
#include "drivers/display/SDLPanel.hpp"
#include "drivers/display/SDLSinglePanel.hpp"
#include "drivers/display/SDLScreen.hpp"
#include "drivers/lights/SDLLights.hpp"
#include "drivers/servo/TestServo.hpp"
#include "drivers/serial/VirtualSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"
#include <SDL2/SDL.h>

#include "PosixPlatform.hpp"
#include "EmbeddedPosixPlatform.hpp"

extern Platform *sys;
extern PanelDriver *panel;
extern LightDriver *lights;
extern ServoDriver *servo;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern Timing *lighttimer;
extern Timing *micwindow;
extern SerialDriver *serial;
extern bool forcetransmitter;


void init_platform() {
#ifdef EMBEDDED_ENABLED
	sys = new EmbeddedPosixPlatform();	// Experimental
#else
	sys = new PosixPlatform();
#endif
}

void initPanel(const char *driver, const char *params) {
	if(panel) {
		return;
	}

	// Initialise any other drivers here

	if(!strcasecmp(driver, "SDL")) {
		panel = new SDLPanel();
		panel->init(params);	// To support driver options
	}

	if(!strcasecmp(driver, "SDLSingle")) {
		panel = new SDLSinglePanel();
		panel->init(params);
	}

	if(!strcasecmp(driver, "SDLScreen")) {
		panel = new SDLScreen();
		panel->init(params);
	}

}

void initLights(const char *driver, int numlights, const char *params) {
	if(lights) {
		return;
	}

	// Initialise any other drivers here

	if(!strcasecmp(driver, "SDLLights")) {
		lights = new SDLLights();
		lights->init(numlights,params);
		lights->setColour(lightcolour);
		lights->setPattern(lightpattern_triangle);
	}

}

void initServo(const char *driver, int angle, const char *params) {
	if(servo) {
		return;
	}

	// Initialise any other drivers here
	if(!strcasecmp(driver, "TESTSERVO")) {
		servo = new TestServo();
		servo->init(angle,params);
	}
}

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	ack = new PosixTiming();
	gradient = new PosixTiming();
	lighttimer = new PosixTiming();
	micwindow = new PosixTiming();
	serial = new VirtualSerialDriver();

	if(!panel) {
		panel = new SDLPanel();
		panel->init("");
	}

	// Fake serial port file
	strcpy(serialPort,"/tmp/_eyetmp_");
#ifdef EMBEDDED_ENABLED
	// Provide a fake serial port
	strcpy(serialPort,"./");
#endif

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
		// 27 and 28 are reserved for EEPROM
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

int mapPinToGPIO(int pin) {

	switch(pin)	{
		case 40:
			return 21;
		//   39 is ground
		case 38:
			return 20;
		case 37:
			return 26;
		case 36:
			return 16;
		case 35:
			return 19;
		//   34 is ground
		case 33:
			return 13;
		case 32:
			return 12;
		case 31:
			return 6;
		//   30 is ground
		case 29:
			return 5;
		// 27 and 28 are reserved for EEPROM
		case 26:
			return 7;
		//   25 is ground
		case 24:
			return 8;
		case 23:
			return 11;
		case 22:
			return 25;
		case 21:
			return 9;
		//   20 is ground
		case 19:
			return 10;
		case 18:
			return 24;
		//   17 is +3v
		case 16:
			return 23;
		case 15:
			return 22;
		//   14 is ground
		case 13:
			return 27;
		case 12:
			return 18;
		case 11:
			return 17;
		//   10 is UART
		//   9 is ground
		//   8 is UART
		case 7:
			return 4;
		//   6 is ground
		case 5:
			return 3;
		//   4 is +5v
		case 3:
			return 2;
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
		sys->exit(1);
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

Timing *get_timer() {
	return new PosixTiming();
}


GPIOPin *init_spi(int csPin, long speed, int mode, int bus) {
	return reserveOutputPin(csPin);
}

void blit_spi(int bus, unsigned char *data, int len) {
	// Not supported
}

#endif

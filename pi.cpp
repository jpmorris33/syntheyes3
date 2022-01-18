#ifdef PLATFORM_PI

#include "syntheyes.hpp"
#include <wiringPi.h>
#include "drivers/display/Unicorn.hpp"
#include "drivers/PosixTiming.hpp"

extern PanelDriver *panel;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *gradient;

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	gradient = new PosixTiming();

	panel = new Unicorn();
	panel->init();
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

void init_pin(int pin) {
	pinMode(pin,INPUT);
	pullUpDnControl(pin,PUD_UP);
}

// Return true if triggered
bool check_pin(int pin) {
	return !digitalRead(pin);
}

void set_pin(int pin, bool state) {
	digitalWrite(pin, state);
}

#endif

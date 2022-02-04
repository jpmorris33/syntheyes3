#ifdef PLATFORM_PI

#include "syntheyes.hpp"
#include <string.h>
#include <wiringPi.h>
#include "drivers/display/Unicorn.hpp"
#include "drivers/display/MAX7219Panel.hpp"
#include "drivers/serial/PiSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"
#include "gpio.hpp"

extern PanelDriver *panel;
extern SerialDriver *serial;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern bool transmitter;
extern bool forcetransmitter;

static GPIOPin *deviceId=NULL;

void init_pin_input(int pin);

void initPanel(const char *driver, const char *params) {
	if(panel) {
		return;
	}

	if(!strcasecmp(driver, "MAX7219")) {
		panel = new MAX7219Panel();
		panel->init(params);
	}
}

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	ack = new PosixTiming();
	gradient = new PosixTiming();
	serial = new PiSerialDriver();

	if(!panel) {
		panel = new Unicorn();
		panel->init("");
	}

	// If pin 29 (21 in WiringPi) is grounded, we're the transmitter
	deviceId = reserveInputPin(29);
	timing->wait_microseconds(250);
	if(!deviceId->check()) {
		transmitter=false;
	}

	// Default serial port for Pi Zero with bluetooth disabled
	strcpy(serialPort,"/dev/ttyAMA0");

	if(panel->getCaps() & PANELCAPS_SPLIT) {
		// Reserve the serial pins  (Note, we don't know for sure if we're the transmitter or receiver yet so grab both for now)
		reserveOutputPin(8);
		reserveInputPin(10);
	} else {
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
	pinMode(pin,INPUT);
	pullUpDnControl(pin,PUD_UP);
}

void init_pin_output(int pin) {
	pinMode(pin, OUTPUT);
}

// Return true if triggered
bool check_pin(int pin) {
	return !digitalRead(pin);
}

void set_pin(int pin, bool state) {
	digitalWrite(pin, !state);
}

void poll_keyboard() {
}

#endif

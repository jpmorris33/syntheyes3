#ifdef PLATFORM_PI

#include "syntheyes.hpp"
#include <string.h>
#include <wiringPi.h>
#include "drivers/display/Unicorn.hpp"
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

static GPIOPin *deviceId=NULL;

void init_pin_input(int pin);

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	ack = new PosixTiming();
	gradient = new PosixTiming();
	serial = new PiSerialDriver();

	panel = new Unicorn();
	panel->init();

	// If pin 29 (21 in WiringPi) is grounded, we're the transmitter
	deviceId = new GPIOPin(29, DEVICE_BOTH, false);
	timing->wait_microseconds(250);
	if(!deviceId->check()) {
		transmitter=false;
	}

	// Default serial port for Pi Zero with bluetooth disabled
	strcpy(serialPort,"/dev/ttyAMA0");

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

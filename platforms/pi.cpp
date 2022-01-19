#ifdef PLATFORM_PI

#include "syntheyes.hpp"
#include <string.h>
#include <wiringPi.h>
#include "drivers/display/Unicorn.hpp"
#include "drivers/serial/PiSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"

extern PanelDriver *panel;
extern SerialDriver *serial;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern bool transmitter;

void init_pin(int pin);

void initPanel() {
	timing = new PosixTiming();
	cooldown = new PosixTiming();
	ack = new PosixTiming();
	gradient = new PosixTiming();
	serial = new PiSerialDriver();

	panel = new Unicorn();
	panel->init();

	// if pin 29 (BCM5 - 21 in WiringPi) is grounded, we're the transmitter
	init_pin(21);
	timing->wait_microseconds(250);
	if(digitalRead(21)) {
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

void init_pin(int pin) {
	pinMode(pin,INPUT);
	pullUpDnControl(pin,PUD_UP);
}

// Return true if triggered
bool check_pin(int pin) {
	return !digitalRead(pin);
}

void set_pin(int pin, bool state) {
	// We're already using the GPIO pins on the transmitter for input,
	// so disable this for safety reasons unless we're the receiver
	if(!transmitter) {
		pinMode(pin, OUTPUT);
		digitalWrite(pin, state);
	}
}

#endif

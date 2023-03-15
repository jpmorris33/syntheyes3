#ifdef PLATFORM_PI

#include "syntheyes.hpp"
#include "colourutils.hpp"
#include <string.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "drivers/display/Unicorn.hpp"
#include "drivers/display/MAX7219Panel.hpp"
#include "drivers/display/MAX7219WPanel.hpp"
#include "drivers/display/SDLScreen.hpp"
#include "drivers/lights/WS2811Lights.hpp"
#include "drivers/servo/PiServo.hpp"
#include "drivers/serial/PiSerialDriver.hpp"
#include "drivers/PosixTiming.hpp"
#include "gpio.hpp"

#include "PosixPlatform.hpp"

extern Platform *sys;
extern PanelDriver *panel;
extern SerialDriver *serial;
extern ServoDriver *servo;
extern Timing *timing;
extern Timing *cooldown;
extern Timing *ack;
extern Timing *gradient;
extern Timing *lighttimer;
extern Timing *micwindow;
extern bool transmitter;
extern bool forcetransmitter;

static GPIOPin *deviceId=NULL;

void init_pin_input(int pin);

void init_platform() {
	sys = new PosixPlatform();
}


void initPanel(const char *driver, const char *params) {
	if(panel) {
		return;
	}

	if(!strcasecmp(driver, "MAX7219")) {
		panel = new MAX7219Panel();
		panel->init(params);
	}
	if(!strcasecmp(driver, "MAX7219W")) {
		panel = new MAX7219WPanel();
		panel->init(params);
	}
#ifdef SDL_SUPPORT
	if(!strcasecmp(driver, "SDLScreen")) {
		panel = new SDLScreen();
		panel->init(params);
	}
#endif
}

void initLights(const char *driver, int numlights, const char *params) {
	if(lights) {
		return;
	}

	// Initialise any other drivers here
#ifdef WS2811_SUPPORT
	if(!strcasecmp(driver, "WS2811")) {
		lights = new WS2811Lights();
		lights->init(numlights,params);
		lights->setColour(lightcolour);
		lights->setPattern(lightpattern_triangle);
	}
#endif

}

void initServo(const char *driver, int angle, const char *params) {
	if(servo) {
		return;
	}

	// Initialise any other drivers here
	if(!strcasecmp(driver, "PISERVO")) {
		servo = new PiServo();
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
		reserveSpecialPin(8);		// Must be Special - otherwise we'll break the serial comms as they run in ALT0 mode
		reserveSpecialPin(10);
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

Timing *get_timer() {
	return new PosixTiming();
}

GPIOPin *init_spi(int cspin, long speed, int mode, int bus) {
	wiringPiSetup();
	wiringPiSPISetup(bus,speed);

	// Reserve the SPI0 pins
	reserveSpecialPin(19);	// MOSI
	reserveSpecialPin(21);	// MISO
	reserveSpecialPin(23);	// CLK
	return reserveOutputPin(cspin);
}

void blit_spi(int bus, unsigned char *data, int len) {
	wiringPiSPIDataRW(bus,data,len);
}

#endif

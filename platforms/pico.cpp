#ifdef PLATFORM_PICO

//
//  Raspberry Pico
//

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <cstring>

#include "../syntheyes.hpp"
#include "../colourutils.hpp"
#include "../drivers/servo/TestServo.hpp"
#include "../drivers/serial/VirtualSerialDriver.hpp"
#include "../drivers/display/WS2811PicoPanel.hpp"
#include "../drivers/display/Hub75Pico.hpp"
#include "../drivers/PicoTiming.hpp"

#include "PicoPlatform.hpp"
#include "../gpio.hpp"

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

extern GPIOPin *ackPin;



void init_platform() {

	stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	sys = new PicoPlatform();
	ackPin = reserveOutputPin(PICO_DEFAULT_LED_PIN);
}

void initPanel(const char *driver, const char *params) {
	// Initialise any other drivers here

	if(!panel) {
//		panel = new WS2811PicoPanel();
		panel = new Hub75Pico();
		panel->init("");
	}
}

void initLights(const char *driver, int numlights, const char *params) {
	if(lights) {
		return;
	}

	// Initialise any other drivers here
}

void initServo(const char *driver, int angle, const char *params) {
	if(servo) {
		return;
	}
}

void initSensor(const char *driver, const char *params) {
	if(sensor) {
		return;
	}
}


void initPanel() {
	timing = new PicoTiming();
	cooldown = new PicoTiming();
	ack = new PicoTiming();
	gradient = new PicoTiming();
	lighttimer = new PicoTiming();
	micwindow = new PicoTiming();
	serial = new VirtualSerialDriver();

	if(!panel) {
		panel = new Hub75Pico();
//		panel = new WS2811PicoPanel();
		panel->init("");
	}

	// Fake serial port file
	// Provide a fake serial port
	strcpy(serialPort,"./");

	if(!(panel->getCaps() & PANELCAPS_SPLIT)) {
		forcetransmitter=true; // Single systems are always the transmitter
	}

}


void pi_init() {
}

int mapPin(int pin) {
	return pin; // No lookup table needed like on Arduino
}

// However, mapping physical pin to GPIO number is needed

int mapPinToGPIO(int pin) {

	switch(pin)	{
		// 40 is +5v
		// 39 is +5v
		// 38 is GND
		// 37 is 3.3V
		// 36 is 3.3V
		// 35 is ADC VREF
		case 34:
			return 28;
		// 33 is GND
		case 32:
			return 27;
		case 31:
			return 26;
		//   30 is RUN
		case 29:
			return 22;
		// 28 is GND
		case 27:
			return 21;
		case 26:
			return 20;
		case 25:
			return 19;
		case 24:
			return 18;
		// 23 is GND
		case 22:
			return 17;
		case 21:
			return 16;
		case 20:
			return 15;
		case 19:
			return 14;
		//   18 is GND
		case 17:
			return 13;
		case 16:
			return 12;
		case 15:
			return 11;
		case 14:
			return 10;
		//   13 is GND
		case 12:
			return 9;
		case 11:
			return 8;
		case 10:
			return 7;
		case 9:
			return 6;
		//   8 is GND
		case 7:
			return 5;
		case 6:
			return 4;
		case 5:
			return 3;
		case 4:
			return 2;
		//   3 is GND
		case 2:
			return 1;
		case 1:
			return 0;
		default:
			return -1;
	};
}

void init_pin_input(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_IN);
	gpio_pull_up(pin);
}

void init_pin_output(int pin) {
	gpio_init(pin);
	gpio_set_dir(pin, GPIO_OUT);
}

bool check_pin(int pin) {
	// Negative pins are for polling ESC
	if(pin < 0) {
		return false;
	}

	return !gpio_get(pin);
}

void set_pin(int pin, bool state) {
        gpio_put(pin, state);
}

void shift_out_msb(int pin, int clockpin, unsigned char value) {
	// NOT IMPLEMENTED YET!
}


void poll_keyboard() {
}

Timing *get_timer() {
	return new PicoTiming();
}

GPIOPin *init_spi(int csPin, long speed, int mode, int bus) {

	spi_init(bus?spi1:spi0,speed);	// Currently just 0 or 1

	// If we ever need to set the mode it can be done here

	// Need to actually make it work!
	return reserveOutputPin(csPin);
}

void blit_spi(int bus, unsigned char *data, int len) {
	spi_write_blocking(bus?spi1:spi0,data,len);	// Currently just 0 or 1
}

int init_i2c(int bus, int addr) {
	return -1; // Not yet supported (placeholder)
}

unsigned char i2c_readReg(int handle, int reg) {
	return 0;  // Not yet supported
}

void i2c_writeReg(int handle, int reg, unsigned char val) {
	// Not yet supported
}


void debugLight() {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

#endif

//
//  GPIO wrapper
//
//  This calls down to the Platform layer to perform the actual I/O.
//  However, since we usually have two devices with their own GPIO banks,
//  it's important to steer the signals to the right destination.
//
//  We also need to register the GPIO pins and make sure we're not trying
//  to use one pin for both input and output as this may damage the hardware
//

#include <stdio.h>
#include <stdlib.h>
#include "syntheyes.hpp"
#include "gpio.hpp"

extern void init_pin_input(int pin);
extern void init_pin_output(int pin);
extern void set_pin(int pin, bool state);
extern void shift_out_msb(int pin, int clock, unsigned char val);
extern bool check_pin(int pin);
extern int mapPin(int pin);

extern bool transmitter;

static bool same_device(char dev1, char dev2);
static char verify_mode(char mode);
static void log_gpio(GPIOPin *ptr);

static GPIOPin *anchor = NULL;

//
//  Set up the GPIO pin
//

GPIOPin::GPIOPin(int inpin, char indevice, char gpiomode) {
	next = NULL;

	realpin = inpin;
	pin = mapPin(inpin);
	device = indevice;
	mode = verify_mode(gpiomode);
	reserved = false;
	invert = false;

	// Set up the GPIO pin
	if(rightDevice()) {
		if(mode == GPIOMODE_OUTPUT) {
			init_pin_output(pin);
		}
		if(mode == GPIOMODE_INPUT) {
			init_pin_input(pin);
		}
		// If it's SPECIAL, don't touch it
	}

	log_gpio(this);
}

//
//  Is this the right machine for the GPIO operation?
//

bool GPIOPin::rightDevice() {
	if(device == DEVICE_TRANSMITTER && !transmitter) {
		return false;
	}
	if(device == DEVICE_RECEIVER && transmitter) {
		return false;
	}

	// Either it matches, or it's set to BOTH
	return true;
}

//
//	Set or clear a GPIO pin after first making sure we're allowed to do that, and we're on the correct device
//	Note that GPIO pins are wired to return logic HIGH (i.e +3V) when nothing is attached...
//	So calling with TRUE here will actually set the pin LOW (0v)
//

void GPIOPin::write(bool state) {
	if((!isOutput()) || (!rightDevice())) {
		return;
	}

	set_pin(pin,state);
}

//
//	Shift out 8 bits, first making sure we're allowed to do that, and we're on the correct device
//

void GPIOPin::writeByte(unsigned char value, GPIOPin *clock) {
	if((!isOutput()) || (!rightDevice())) {
		return;
	}

	// This, with clock->getPin() displays red on the Hub75 panel irrespective of the dpin - clock is realpin 11, gpio 17, wpi 0
	// 'pin' is pre-mapped to wiringPi pins, getPin returns the realpin and shift_out_msb will map that itself
	shift_out_msb(pin, clock->getPin(), value);
}

//
//	Read a GPIO pin after first making sure we're allowed to do that, and we're on the correct device.
//	Note that GPIO pins are wired to return logic HIGH (i.e +3V) when nothing is attached...
//	So returning TRUE here actually means the pin is set LOW (0V).
//

bool GPIOPin::check() {
	if((!isInput()) || (!rightDevice())) {
		return false;
	}

	if(invert) {
		// May want to invert logic on specific pins
		return !check_pin(pin);
	}
	return check_pin(pin);
}

//
//  Returns the original pin, without any translation
//

int GPIOPin::getPin() {
	return realpin;
}

char GPIOPin::getDevice() {
	return device;
}

bool GPIOPin::isInput() {
	return (mode == GPIOMODE_INPUT);
}

bool GPIOPin::isOutput() {
	return (mode == GPIOMODE_OUTPUT);
}

bool GPIOPin::isReserved() {
	if(mode == GPIOMODE_SPECIAL) {
		return true;
	}
	return reserved;
}

void GPIOPin::reserve() {
	reserved=true;
}


GPIOPin *GPIOPin::findConflict() {
	if(!anchor) {
		return NULL;
	}

	for(GPIOPin *ptr=anchor;ptr->next;ptr=ptr->next) {
		if(conflicting(ptr)) {
			// Oh dear
			printf("GPIO pin %p conflicting with %p\n",this,ptr);
			return ptr;
		}
	}

	return NULL;		
}

//
//  See if we have a conflict over a pin (on the same device)
//


bool GPIOPin::conflicting(GPIOPin *ptr) {
	if(!ptr || ptr == this) {
		// Can't conflict with yourself (or NULL)
		return false;
	}
	if(realpin != ptr->getPin()) {
		return false;
	}
	if(!same_device(device, ptr->getDevice())) {
		return false;
	}
	if(ptr->isReserved()) {
		return true;	// If this pin is taken by the hardware, you're not having it
	}
	if(isOutput() == ptr->isOutput()) {
		return false;
	}
	return true; // Oh dear
}

void GPIOPin::setInverted() {
	invert = true;
}


bool same_device(char dev1, char dev2) {
	if(dev1 == DEVICE_BOTH || dev2 == DEVICE_BOTH) {
		return true;
	}
	if(dev1 == dev2) {
		return true;
	}
	return false;
}

char verify_mode(char mode) {
	switch(mode) {
		case GPIOMODE_INPUT:
		case GPIOMODE_OUTPUT:
			return mode;
		default:
			return GPIOMODE_SPECIAL;
	}
}

//
//  GPIO pin registration
//

void log_gpio(GPIOPin *gpio) {

	if(!anchor) {
		anchor = gpio;
		return;
	}

	if(anchor && !anchor->next) {
		anchor->next = gpio;
		return;
	}

	// Scoot to the end and add it
	GPIOPin *ptr;
	for(ptr=anchor;ptr->next;ptr=ptr->next);
	ptr->next = gpio;
}

//
//  Reserve pins for the display drivers
//

GPIOPin *reserveOutputPin(int pin) {
	GPIOPin *gpio = new GPIOPin(pin,transmitter?DEVICE_TRANSMITTER:DEVICE_RECEIVER,GPIOMODE_OUTPUT);
	gpio->reserve();
	return gpio;
}

GPIOPin *reserveInputPin(int pin) {
	GPIOPin *gpio = new GPIOPin(pin,transmitter?DEVICE_TRANSMITTER:DEVICE_RECEIVER,GPIOMODE_INPUT);
	gpio->reserve();
	return gpio;
}

//
//	SPI pins on the Pi must NOT be reprogrammed as Output or it'll jam the SPI engine up for good
//

GPIOPin *reserveSpecialPin(int pin) {
	GPIOPin *gpio = new GPIOPin(pin,transmitter?DEVICE_TRANSMITTER:DEVICE_RECEIVER,GPIOMODE_SPECIAL);
	gpio->reserve();
	return gpio;
}

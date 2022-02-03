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
extern bool check_pin(int pin);
extern int mapPin(int pin);

extern bool transmitter;

static bool same_device(char dev1, char dev2);
static void log_gpio(GPIOPin *ptr);

static GPIOPin *anchor = NULL;

//
//  Set up the GPIO pin
//

GPIOPin::GPIOPin(int inpin, char indevice, bool inoutput) {
	next = NULL;

	realpin = inpin;
	pin=mapPin(inpin);
	device=indevice;
	output=inoutput;
	reserved=false;

	// Set up the GPIO pin
	if(rightDevice()) {
		if(output) {
			init_pin_output(pin);
		} else {
			init_pin_input(pin);
		}
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
	if((!output) || (!rightDevice())) {
		return;
	}

	set_pin(pin,state);
}

//
//	Read a GPIO pin after first making sure we're allowed to do that, and we're on the correct device.
//	Note that GPIO pins are wired to return logic HIGH (i.e +3V) when nothing is attached...
//	So returning TRUE here actually means the pin is set LOW (0V).
//

bool GPIOPin::check() {
	if(output || (!rightDevice())) {
		return false;
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

bool GPIOPin::isOutput() {
	return output;
}

bool GPIOPin::isReserved() {
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
	if(output == ptr->isOutput()) {
		return false;
	}
	return true; // Oh dear
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
	GPIOPin *gpio = new GPIOPin(pin,transmitter?DEVICE_TRANSMITTER:DEVICE_RECEIVER,true);
	gpio->reserve();
	return gpio;
}

GPIOPin *reserveInputPin(int pin) {
	GPIOPin *gpio = new GPIOPin(pin,transmitter?DEVICE_TRANSMITTER:DEVICE_RECEIVER,false);
	gpio->reserve();
	return gpio;
}
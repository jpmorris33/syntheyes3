#include "ServoDriver.hpp"

#include <stdio.h>
#include <string.h>


//
//  Default stubs for the concrete implementations to override
//

void ServoDriver::init(int defaultAngle, const char *param) { curAngle = targetAngle = defaultAngle; timer = NULL; }
void ServoDriver::write(int newAngle) { curAngle = newAngle; }


//
//  Common code
//

void ServoDriver::setAngle(int newAngle) {
	if(newAngle < 0) {
		newAngle=0;
	}
	if(newAngle > 360) {
		newAngle=360;
	}
	write(newAngle);
	targetAngle=newAngle;
	seekDelay=4;
}

void ServoDriver::seekAngle(int newAngle) {
	if(newAngle < 0) {
		newAngle=0;
	}
	if(newAngle > 360) {
		newAngle=360;
	}
	targetAngle=newAngle;
	if(timer) {
		timer->set(seekDelay);
	}
}

int ServoDriver::getAngle() {
	return curAngle;
}

void ServoDriver::setDelay(int delay) {
	if(delay >= 0) {
		seekDelay=delay;
	} else {
		seekDelay=0;
	}
}

//
//	Handle gradual position changes
//

void ServoDriver::update() {
	if(timer) {
		if(!timer->elapsed())
			return;
		timer->set(seekDelay);
	}

	if(curAngle == targetAngle) {
		return;
	}

	if(curAngle < targetAngle) {
		curAngle++;
		write(curAngle);
		return;
	}

	if(curAngle > targetAngle) {
		curAngle--;
		write(curAngle);
		return;
	}
}


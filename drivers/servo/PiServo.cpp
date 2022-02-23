#ifdef PLATFORM_PI

/**
 * WiringPi servo code
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <wiringPi.h>

#include "PiServo.hpp"
#include "../PosixTiming.hpp"

extern int mapPin(int pin);

//
//	Init the WiringPi servo driver
//
void PiServo::init(int defaultAngle, const char *param) {

	curAngle=targetAngle=defaultAngle;
	seekDelay = 0;
	timer = new PosixTiming();


	int pwmPin = 12;
	const char *p = getDriverParam(param, "pwm");
	if(p) {
		pwmPin = getDriverInt(p);
	}

	hwPin = mapPin(pwmPin);
	if(hwPin < 0) {
		printf("*PiServo: Could not use PWM pin %d\n", pwmPin);
		return;
	}

	printf("*PiServo: Set to initial angle %d on pin %d\n", defaultAngle,pwmPin);

	reserveSpecialPin(pwmPin);
	pwmSetMode(PWM_MODE_MS);
	pinMode(hwPin,PWM_OUTPUT);
	pwmSetClock(4000);
	pwmSetRange(1024);	

	write(curAngle);
}

//
//  Write to the servo
//

void PiServo::write(int angle) {
	pwmWrite(hwPin, (angle * 1023)/360);
	curAngle=angle;
}


#endif

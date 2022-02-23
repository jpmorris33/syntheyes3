/**
 * Dummy servo code
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "../PosixTiming.hpp"
#include "TestServo.hpp"

extern int mapPin(int pin);

//
//	Init the WiringPi servo driver
//
void TestServo::init(int defaultAngle, const char *param) {

	curAngle=targetAngle=defaultAngle;
	seekDelay = 0;
	timer = new PosixTiming();

	printf("*TestServo: Set to initial angle %d\n", defaultAngle);

	write(curAngle);
}

//
//  Write to the servo
//

void TestServo::write(int angle) {
	printf("current servo angle now %d\n",angle);
	curAngle=angle;
}


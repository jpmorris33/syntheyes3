/*
SynthEyes V3 - a programmable eye display for raspberry Pi

BSD 3-Clause License

Copyright (c) 2022, Joseph P Morris
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "syntheyes.hpp"
#include "patterns.h"

extern void pi_init();
extern void initPanel();
extern bool check_pin(int pin);
extern void readConfig(FILE *fp);

static void runEyes();
static void getNextState();
static void setNextState(Expression *newState);
static bool check_comms();
static bool check_gpio();
static bool check_serial();
static bool check_network();


PanelDriver *panel = NULL;
Timing *timing = NULL;
Timing *cooldown = NULL;
Timing *gradient = NULL;

Font font;
ExpressionList expressions;
uint32_t eyecolour = 0xff8700;
int cooldown_time = 5;

// Rainbow effect
uint32_t rainbow[16] = {0xff1700,0xff7200,0xffce00,0xe8ff00,0x79ff00,0x1fff00,0x00ff3d,0x00ff98,0x00fff4,0x00afff,0x0054ff,0x0800ff,0x6300ff,0xbe00ff,0xff00e4,0xff0089};
int rainbowspeed = 10;
unsigned char rainbowoffset=0;

char gifDir[512];
Expression *idle=NULL;
Expression *nextState=NULL;
Expression *lastState=NULL;
ExpressionSet *gpioList = NULL;



int main(int argc, char *argv[]){
FILE *fp=NULL;

initPanel();
set_pattern(PATTERN_V);


if(argc > 1) {
	// Switch off the display (for testing on the Pi)
	if(!strcmp(argv[1],"off")) {
		panel->clear(0);
		timing->wait_microseconds(100000);
		panel->draw();
		timing->wait_microseconds(100000);
		exit(0);
	}
}



font.printVersion(VERSION, true, 3000);	// Wait 3 sec

srandom(time(NULL));

strcpy(gifDir,"/boot/");

if(argc > 3) {
	if(!strcasecmp(argv[2], "-config")) {
		fp=fopen(argv[3],"r");
		if(!fp) {
			font.errorMsg("Error: failed to open config file '%s'\n",argv[3]);
		}
	}
}

// For development
if(!fp) {
	fp=fopen("./eyeconfig3.txt","r");
}

if(!fp) {
	fp=fopen("/boot/eyeconfig3.txt","r");
	if(!fp) {
		font.errorMsg("Error: failed to open config file '/boot/eyeconfig3.txt'\n");
	}
}
readConfig(fp);
fclose(fp);

// Now do some final checks
idle=expressions.findFirstByTrigger(TRIGGER_IDLE);
if(!idle) {
	font.errorMsg("Error: No IDLE animation declared in config file.  Make sure you have something like...  idle: idle.gif");
}

gpioList = expressions.findAllByTrigger(TRIGGER_GPIO);


pi_init();
expressions.initGPIO();

cooldown->set(1); // It'll be elapsed by the time we check
gradient->set(1); // It'll be elapsed by the time we check

runEyes();

}

//
//  Probably a better design would be to have fixed types, e.g.
//  IDLE, BLINK, RANDOM, PIN
//  Maybe even replace video:  with idle: blink: wink etc to make
//  the syntax clearer.  The number could be pin or random chance
//  depending on the command used.
//
//  Then add a findVideos() function which returns a list of videos
//  matching that type, which we can then pick from at random.
//  This will provide for multiple random videos for the same task,
//  e.g. different idle videos, blinking and winking via BLINK.
//  We could in fact dispense with BLINK entirely and make that
//  part of RANDOM...
//

void runEyes() {
for(;;)  {
	if(nextState) {
		printf("Roll animation '%s'\n",nextState->name);
		lastState=nextState;
		nextState=NULL;
		cooldown->set(cooldown_time*1000);	// Prevent immediate retriggering
		lastState->play();
	} else {
		idle->play();
	}
	getNextState();
}
}

void getNextState() {
	// If we've been given something by the GPIO inputs
	if(nextState) {
		return;
	}

	setNextState(expressions.pick(TRIGGER_RANDOM));
	if(nextState) {
		return;
	}
	nextState=NULL;
}


void setNextState(Expression *newState) {
	// Prevent retriggering
	if(lastState && newState == lastState) {
		if(!cooldown->elapsed()) {
			nextState=NULL;
			return;
		}
		lastState = NULL; // disable cooldown check
	}

	// Otherwise, do it
	nextState = newState;
}

//
//	Check for external input from GPIO
//

bool check_comms() {
	// TODO: Handle ACK light here as well

	if(check_serial()) {
		return true;
	}

	if(check_network()) {
		return true;
	}

	return check_gpio();
}

bool check_serial() {
	return false;
}

bool check_network() {
	return false;
}


bool check_gpio() {
	int ctr,len=0;
	Expression *exp;

	if(nextState) {
		return false;
	}

	// Check the GPIO pins, if any are defined
	if(gpioList) {	
		len = gpioList->count();
	}

	for(ctr=0;ctr<len;ctr++) {
		exp = gpioList->get(ctr);
		if(check_pin(exp->parameter)) {
			ExpressionSet *videos = expressions.findByGPIO(exp->parameter);
			if(videos) {
				setNextState(videos->getRandom());
				delete videos;
				return true;
			}
		}
	}
	return false;
}


void wait(int ms, bool interruptable) {
	timing->set(ms);
	while(!timing->elapsed()) {
		panel->draw();
		timing->wait_microseconds(50);

		update_rainbow();

		if(check_comms() && interruptable) {
			break;
		}
		check_pin(-666);  // Check for ESC on desktop test version
	}
}

void update_rainbow() {
	// Update the rainbow tick
	if(gradient->elapsed()) {
		gradient->set(rainbowspeed);
		rainbowoffset++;
		rainbowoffset &= 0x0f;
	}
}

void set_pattern(int pattern) {
	switch(pattern) {
		case PATTERN_H:
			panel->setPattern(pattern_h);
			break;
		case PATTERN_O:
			panel->setPattern(pattern_o);
			break;
		default:
			panel->setPattern(pattern_v);
			break;
	}
}

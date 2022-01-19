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

static void serial_init();
static void serial_transmit(Expression *exp);
static Expression *serial_receive();

PanelDriver *panel = NULL;
SerialDriver *serial = NULL;
Timing *timing = NULL;
Timing *cooldown = NULL;
Timing *gradient = NULL;

Font font;
ExpressionList expressions;
uint32_t eyecolour = 0xff8700;
int cooldown_time = 5;
bool transmitter = true;
bool forcetransmitter = false;
char serialPort[256];
int serialRate=19200;

// Rainbow effect
uint32_t rainbow[16] = {0xff1700,0xff7200,0xffce00,0xe8ff00,0x79ff00,0x1fff00,0x00ff3d,0x00ff98,0x00fff4,0x00afff,0x0054ff,0x0800ff,0x6300ff,0xbe00ff,0xff00e4,0xff0089};
int rainbowspeed = 10;
unsigned char rainbowoffset=0;
bool flash_state=true;

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
		if(!strcasecmp(argv[1],"receiver")) {
			transmitter=false;
		}
		if(!strcasecmp(argv[1],"transmitter")) {
			transmitter=true;
		}
		// Switch off the display (for testing on the Pi)
		if(!strcasecmp(argv[1],"off")) {
			panel->clear(0);
			timing->wait_microseconds(100000);
			panel->draw();
			timing->wait_microseconds(100000);
			exit(0);
		}
		if((!strcasecmp(argv[1],"version")) || (!strcasecmp(argv[1],"-version"))) {
			font.printVersion(VERSION, transmitter, 3000);	// Wait 3 sec
			exit(0);
		}
	}

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

	if(forcetransmitter) {
		printf("Forcing into transmitter mode by config file\n");
		transmitter=true;
	}

	font.printVersion(VERSION, transmitter, 3000);	// Wait 3 sec

	srandom(time(NULL));

	// Now do some final checks
	idle=expressions.findFirstByTrigger(TRIGGER_IDLE);
	if(!idle) {
		font.errorMsg("Error: No IDLE animation declared in config file.  Make sure you have something like...  idle: idle.gif");
	}

	gpioList = expressions.findAllByTrigger(TRIGGER_GPIO);


	pi_init();
	if(transmitter) {
		// We don't want to do this for the receiver as it will probably mess up the ACK light
		expressions.initGPIO();
	}

	cooldown->set(1); // It'll be elapsed by the time we check
	gradient->set(1); // It'll be elapsed by the time we check

	serial_init();

	runEyes();

	// Shouldn't ever get here
	return 0;
}

//
//	This is the main loop - it picks the appropriate video, and runs it while waiting for something else to be cued up.
//	New videos will be polled for during the wait() function, via GPIO or possibly network comms in future
//

void runEyes() {
	for(;;)  {
		if(nextState) {
			serial_transmit(nextState);
			printf("Roll animation '%s'\n",nextState->name);
			lastState=nextState;
			nextState=NULL;
			cooldown->set(cooldown_time*1000);	// Prevent immediate retriggering
			lastState->play();
		} else {
			// Nothing cued up for playback, play the default
			idle->play();
		}
		// Pick a random video if we don't already have one ready to roll
		getNextState();
	}
}

//
//	Pick a random video, or not as the case may be
//

void getNextState() {
	// If we've already been given something by the GPIO or comms
	if(nextState) {
		return;
	}

	// If we're acting as a receiver only, don't pick any expressions automatically
	if(!transmitter) {
		return;
	}

	setNextState(expressions.pick(TRIGGER_RANDOM));
	if(nextState) {
		return;
	}
	nextState=NULL;
}

//
//	Cue up the next video.  This also does a check to prevent the same video running twice
//	in too short a period for debouncing the GPIO and preventing double-blinks
//

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
//	Check for external input from GPIO or network comms
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

//
//	Get serial command
//

bool check_serial() {
	if(transmitter) {
		// You're supposed to send, not listen
		return false;
	}

	Expression *exp = serial_receive();
	if(exp) {
		// Set this directly so it plays immediately to avoid sync problems
		nextState = exp;
		return true;
	}

	return false;
}

//
//	Hook for getting commands from an app in future
//

bool check_network() {
	if(transmitter) {
		// You're supposed to send, not listen
		return false;
	}

	// If we get a name, use findByName() to try and find it, then cue it immediately, no ifs or buts
	// And also transmit the name to the receiver

	return false;
}

//
//	Check for commands from the GPIO pins
//

bool check_gpio() {
	int ctr,len=0;
	Expression *exp;

	// If we're acting as a receiver only, don't check the GPIO
	if(!transmitter) {
		return false;
	}

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

//
//	Wait for a few milliseconds and perform various administrative tasks
//	This is polled during the animation engine
//

void wait(int ms, bool interruptable) {
	timing->set(ms);
	while(!timing->elapsed()) {

		// Drawing is no longer done here since we need to know whether to call draw or drawMirrored()
		timing->wait_microseconds(50);

		update_rainbow();

		if(check_comms() && interruptable) {
			break;
		}
		check_pin(-666);  // Check for ESC on desktop test version
	}
}

//
//	Update the gradient colour cycling for that draw mode
//

void update_rainbow() {
	// Update the rainbow tick
	if(gradient->elapsed()) {
		gradient->set(rainbowspeed);
		rainbowoffset++;
		rainbowoffset &= 0x0f;

		// Tie to the gradient system
		if(!rainbowoffset) {
			flash_state = !flash_state;
		}
	}
}

//
//	Set the gradient pattern, these are currently hardcoded
//

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


void serial_init() {
	if(transmitter) {
		if(!serial->open_write(serialPort,19200)) {
			font.errorMsg("Transmitter: Error opening serial port '%s'\n",serialPort);
		}
	} else {
		if(!serial->open_read(serialPort,19200)) {
			font.errorMsg("Receiver: Error opening serial port '%s'\n",serialPort);
		}
	}
}

void serial_transmit(Expression *exp) {
	char msg[1024];
	int ret,len;

	if(!transmitter) {
		return;
	}

	memset(msg,0,sizeof(msg));
	snprintf(msg,sizeof(msg)-1,"PLAY: (%s)",exp->name);
	len=strlen(msg)+1;
	
	ret=serial->write(msg);
	if(ret != len) {
		printf("Warning: wrote %d bytes to serial instead of %d\n",ret,len);
	}
}

Expression *serial_receive() {
	char buffer[1024];
	int ret;
	char *paramstart,*paramend;
	memset(buffer,0,sizeof(buffer));

	if(transmitter) {
		return NULL;
	}

	ret=serial->read(buffer,sizeof(buffer));
	if(ret<1) {
		return NULL;
	}

	// Got something
	paramstart = strchr(buffer,'(');
	if(!paramstart) {
		printf("Warning: Comms '%s' didn't have opening brace - aborting\n", buffer);
		return NULL;
	}
	*paramstart++=0;
	if(strcmp(buffer,"PLAY: ")) {
		printf("Warning: Comms '%s' didn't start with PLAY: \n", buffer);
	}
	paramend = strchr(paramstart,')');
	if(!paramend) {
		printf("Warning: Comms '%s' didn't have closing brace - aborting\n", paramstart);
		return NULL;
	}
	*paramend=0;

	Expression *exp = expressions.findByName(paramstart);
	if(!exp) {
		printf("Warning: did not find video '%s' from Comms command\n", paramstart);
		return NULL;
	}

	return exp;
}

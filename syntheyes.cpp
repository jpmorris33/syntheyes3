/*
SynthEyes V3 - a programmable eye display for raspberry Pi

BSD 3-Clause License

Copyright (c) 2022, Joseph P Morris, IT-HE SOFTWARE
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
#include "colourutils.hpp"
#include "patterns.h"

extern void pi_init();
extern void initPanel();
extern void init_colourutils();
extern void init_platform();
extern void scanConfig(FileIO *fp);
extern void readConfig(FileIO *fp);
extern void poll_keyboard(); // For ESC to quit on desktop

static void runEyes();
static void getnextExpression();
static void setnextExpression(Expression *newExpression);
static bool check_comms();
static bool check_gpio();
static bool check_serial();
static bool check_network();
static void update_ack();
static void update_rainbow();
static void update_lights();
static void update_servo();

static void serial_init();
static void serial_transmit(Expression *exp);
static Expression *serial_receive();

PanelDriver *panel = NULL;
LightDriver *lights = NULL;
SerialDriver *serial = NULL;
ServoDriver *servo = NULL;
Timing *timing = NULL;
Timing *cooldown = NULL;
Timing *gradient = NULL;
Timing *lighttimer = NULL;
Timing *ack = NULL;
Timing *micwindow = NULL;
Platform *sys = NULL;

Font font;
ExpressionList expressions;
uint32_t eyecolour = 0xff8700;
uint32_t lightcolour = 0xff8700;
uint32_t miccolour = 0;
int cooldown_time = 5;
bool seamless=false;
bool transmitter = true;
bool forcetransmitter = false;
char serialPort[256];
int serialRate=19200;
GPIOPin *mic = NULL;
bool micInvert=false;
int micBright=100;
int micDim=10;
int micDelay = 500;
GPIOPin *ackPin = NULL;
int ackTime = 750;
int randomChance = 75; // 75 percent chance of picking a random event to space things out

// Rainbow effect
uint32_t rainbow[16] = {0xff1700,0xff7200,0xffce00,0xe8ff00,0x79ff00,0x1fff00,0x00ff3d,0x00ff98,0x00fff4,0x00afff,0x0054ff,0x0800ff,0x6300ff,0xbe00ff,0xff00e4,0xff0089};
int rainbowspeed = 10;
int lightspeed = 25;
unsigned char rainbowoffset=0;
bool flash_state=true;
int scrollspeed = 40;

char gifDir[512];
Expression *idle=NULL;
Expression *nextExpression=NULL;
Expression *lastExpression=NULL;
ExpressionSet *gpioList = NULL;

static bool noMirror=false;


int main(int argc, char *argv[]){
	FileIO *fp=NULL;

	init_platform();
	init_colourutils();

	printf("SynthEyes v%s - (C)2022 IT-HE Software\n",VERSION);
	puts(  "========================================\n");
	puts("Software provided under the 3-clause BSD license\n");

	puts("*Open config...");

	strcpy(gifDir,"/boot/");

	if(argc > 3) {
		if(!strcasecmp(argv[2], "-config")) {
			fp=sys->openFile(argv[3],"r");
			if(!fp) {
				font.errorMsg("Error: failed to open config file '%s'\n",argv[3]);
			}
		}
	}

	// For development
	if(!fp) {
		fp=sys->openFile("./eyeconfig3.txt","r");
	}

	if(!fp) {
		fp=sys->openFile("/boot/eyeconfig3.txt","r");
		if(!fp) {
			font.errorMsg("Error: failed to open config file '/boot/eyeconfig3.txt'\n");
		}
	}
	scanConfig(fp);

	puts("*Panel init...");

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
			if(lights) {
				lights->force(0);
				lights->draw();
			}

			panel->clear(0);
			timing->wait_microseconds(100000);
			panel->draw();
			timing->wait_microseconds(100000);
			sys->exit(0);
		}
		if((!strcasecmp(argv[1],"version")) || (!strcasecmp(argv[1],"-version"))) {
			font.printVersion(VERSION, transmitter, 3000);	// Wait 3 sec
			sys->exit(0);
		}
	}

	if(forcetransmitter) {
		printf("Forcing into transmitter mode by config file\n");
		transmitter=true;
	}

	puts("*Read config...");

	readConfig(fp);
	sys->closeFile(fp);

	font.printVersion(VERSION, transmitter, 3000);	// Wait 3 sec

	srandom(time(NULL));

	puts("*System setup...");

	// Now do some final checks
	idle=expressions.findFirstByTrigger(TRIGGER_IDLE);
	if(!idle) {
		font.errorMsg("Error: No IDLE animation declared in config file.  Make sure you have something like...  idle: idle     gif: idle.gif");
	}

	gpioList = expressions.findAllByTrigger(TRIGGER_GPIO);


	pi_init();
	expressions.initBackgrounds();

	// They'll be elapsed by the time we check
	cooldown->set(1);
	gradient->set(1);
	lighttimer->set(1);
	micwindow->set(1);

	if(lights) {
		lights->setColour(lightcolour);
	}

	puts("*Serial comms init...");

	serial_init();

	if(transmitter) {
		puts("*All OK!  Running as transmitter");
	} else {
		puts("*All OK!  Running as receiver");
	}

	// Decide if we need to display the mirrored image
	// If we're on a single system rather than a split (dual Pi) setup, we'll want to call drawMirrored() most of the time
	// If we're on a split system, we only want to call drawMirrored() for the Receiver unit
	noMirror=false;
	if((panel->getCaps() & PANELCAPS_SPLIT) && transmitter) {
		noMirror=true;
	}

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
		if(nextExpression) {
			serial_transmit(nextExpression);
			dbprintf("Roll animation '%s'\n",nextExpression->name);
			lastExpression=nextExpression;
			nextExpression=NULL;
			cooldown->set(cooldown_time*1000);	// Prevent immediate retriggering
			lastExpression->play();
			// Now run the idle animation for a bit unless in protogen mode
			if(!seamless) {
				idle->play();
			}
		} else {
			// Nothing cued up for playback, play the default
			idle->play();
		}
		// Pick a random video if we don't already have one ready to roll
		getnextExpression();
	}
}

//
//	Pick a random video, or not as the case may be
//

void getnextExpression() {
	// If we've already been given something by the GPIO or comms
	if(nextExpression) {
		return;
	}

	// If we're acting as a receiver only, don't pick any expressions automatically
	if(!transmitter) {
		return;
	}

	// Random chance of picking a random expression.  This is to help prevent it running several animations in succession
	int chance = random()%100;
	if(randomChance <= chance) {
		// You lose, try again
		return;
	}
	

	setnextExpression(expressions.pick(TRIGGER_RANDOM));
	if(nextExpression) {
		return;
	}
	nextExpression=NULL;
}

//
//	Cue up the next video.  This also does a check to prevent the same video running twice
//	in too short a period for debouncing the GPIO and preventing double-blinks
//

void setnextExpression(Expression *newExpression) {
	// Prevent retriggering
	if(lastExpression && newExpression == lastExpression) {
		if(!cooldown->elapsed()) {
			nextExpression=NULL;
			return;
		}
		lastExpression = NULL; // disable cooldown check
	}

	// Otherwise, do it
	nextExpression = newExpression;
}

//
//	Check for external input from GPIO or network comms
//

bool check_comms() {
	update_ack();

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
		nextExpression = exp;
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

	if(nextExpression) {
		return false;
	}

	// Check the GPIO pins, if any are defined
	if(gpioList) {	
		len = gpioList->count();
	}

	for(ctr=0;ctr<len;ctr++) {
		exp = gpioList->get(ctr);
		if(exp->pin->check()) {
			ExpressionSet *videos = expressions.findByGPIO(exp->parameter);
			if(videos) {
				setnextExpression(videos->getRandom());
				delete videos;
				return true;
			}
		}
	}
	return false;
}

void update_ack() {
	if(ackPin) {
		if(ack->elapsed()) {
			ackPin->write(true);
		} else {
			ackPin->write(false);
		}
	}
}

void update_servo() {
	if(servo) {
		servo->update();
	}
}


//
//	Wait for a few milliseconds and perform various administrative tasks
//	This is polled during the animation engine
//

void wait(int ms, bool interruptable) {
	timing->set(ms);
	while(!timing->elapsed()) {

		// Drawing is no longer done here since we need to know whether to call draw or drawMirrored()
		timing->wait_microseconds(500);

		update_rainbow();

		update_lights();

		update_servo();

		if(check_comms()) {
			// Flash the ACK light, if enabled
			if(nextExpression && nextExpression->ack) {
				ack->set(ackTime);
			}
			if(interruptable) {
				break;
			}
		}
		poll_keyboard();  // Check for ESC on desktop test version
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
//	Update the status lights (if present)
//

void update_lights() {
	if(!lights) {
		return;
	}

	// If we have a microphone input over GPIO, flash the lights
	if(mic) {
		if(!micwindow->elapsed()) {
			lights->setColour(miccolour ? miccolour : lightcolour);
			lights->force(mic->check() != micInvert ? micBright : micDim);
			lights->draw();
			lighttimer->set(lightspeed); // Reset the timer to stop it immediately being overridden
			return;
		} else {
			if(mic->check() != micInvert) {
				lights->setColour(miccolour ? miccolour : lightcolour);
				lights->force(micBright);
				lights->draw();
				lighttimer->set(lightspeed);	// Reset the timer to stop it immediately being overridden
				micwindow->set(micDelay);	// Open a window during which we output the mic to the lights
				return;
			}
		}
	}

	// Update the status lights
	if(lighttimer->elapsed()) {
		lights->setColour(lightcolour);
		lights->update();
		lighttimer->set(lightspeed);
		lights->draw();
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

//
//	Set the lighting pattern, these are currently hardcoded
//

void set_lightpattern(int pattern) {
	if(!lights) {
		return;
	}

	switch(pattern) {
		case LIGHTPATTERN_SAW:
			lights->setPattern(lightpattern_saw);
			break;
		case LIGHTPATTERN_TRIANGLE:
			lights->setPattern(lightpattern_triangle);
			break;
		default:
			lights->setPattern(lightpattern_saw);
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
		if(buffer[0]) {
			printf("Warning: Comms '%s' didn't have opening brace - aborting\n", buffer);
		}
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


void drawEyes(bool mirror) {
	// If we're on a split system, but not on the mirrored panel,
	// Disable the mirror effect
	if(noMirror) {
		mirror=false;
	}

	if(mirror) {
		panel->drawMirrored();
	} else {
		panel->draw();
	}
}

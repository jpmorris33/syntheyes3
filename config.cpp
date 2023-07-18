//
//  Configuration reader
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "syntheyes.hpp"

void readConfig(FileIO *fp);
static void preParse(const char *line);
static void parse(const char *line);
void makepath(char path[1024], const char *filename);

static char *nextWord(char *input);
static const char *nextWordStart(const char *input);
static const char *nextWordEnd(const char *input);
static const char *findWord(const char *input, int pos);
static uint32_t parseColour(const char *hex);
static int parseDrawmode(const char *mode);
static int parseLightmode(const char *mode);
static int parseSensorChannel(const char *channel);
static void add_action(ExpressionAction *slot, const char *input);
static GPIOPin *parseGPIO(const char *cmd, const char *param, bool output);
static GPIOPin *parseGPIO(const char *cmd, const char *param, bool output, char forceDevice);
static bool parseTrue(const char *param);
static bool parseFalse(const char *param);
static int parseBlinkmode(const char *mode);

#ifdef DEBUGGING
static const char *videotype(int type);
#endif

extern int mapPin(int pin);
extern char gifDir[512];
extern int cooldown_time;
extern int rainbowspeed;
extern int scrollspeed;
extern int lightspeed;
extern bool forcetransmitter;
extern bool rotated180;
extern bool seamless;
extern GPIOPin *ackPin;
extern int ackTime;
extern GPIOPin *mic;
extern bool micInvert;
extern int micBright;
extern int micDim;
extern int micDelay;

extern int randomChance;
extern void initPanel(const char *driver, const char *params);
extern void initLights(const char *driver, int numlights, const char *params);
extern void initServo(const char *driver, int angle, const char *params);
extern void initSensor(const char *driver, const char *params);

//
//  Config reader
//

void scanConfig(FileIO *fp) {
	char buf[1024];

	fp->seek(0L);

	for(;;) {
		if(fp->eof()) {
			return;
		}
		buf[0]=0;
		if(fp->readLine(buf,1024)) {
			preParse(buf);
		}
	}
}

void readConfig(FileIO *fp) {
	char buf[1024];

	fp->seek(0L);

	for(;;) {
		if(fp->eof()) {
			return;
		}
		buf[0]=0;
		if(fp->readLine(buf,1024)) {
			parse(buf);
		}
	}
}

//
//  Pre-parser for display driver etc
//


void preParse(const char *line) {
	char buf[1024];
	char cmd[1024];
	char param[1024];
	char param2[1024];
	const char *word;

	if(line[0] == '#') {
		return;
	}

	SAFE_STRCPY(buf,line);

	word=findWord(buf,1);
	SAFE_STRCPY(cmd,word);
	nextWord(cmd);

	word=findWord(buf,2);
	SAFE_STRCPY(param,word);

	word=findWord(buf,3);
	SAFE_STRCPY(param2,word);

	word=findWord(buf,4);

	// Anything there?
	if(!cmd[0]) {
		return;
	}
	if(!param[0]) {
		return;
	}

	if(!strcasecmp(cmd,"display:")) {
		nextWord(param);
		initPanel(param, param2);
	}

	if(!strcasecmp(cmd,"rotate180:")) {
		nextWord(param);
		if(parseTrue(param)) {
			rotated180=true;
		}
	}

	if(!strcasecmp(cmd,"lights:")) {
		nextWord(param);
		int numlights=8;
		if(param2[0]) {
			nextWord(param2);
			numlights=atoi(param2);
		}
		initLights(param, numlights > 0 ? numlights : 8, word);
	}

	if(!strcasecmp(cmd,"servo:")) {
		nextWord(param);
		int angle=0;
		if(param2[0]) {
			nextWord(param2);
			angle=atoi(param2);
		}
		initServo(param, angle, word);
	}

	if(!strcasecmp(cmd,"sensordrv:")) {
		nextWord(param);
		nextWord(param2);
		initSensor(param, param2);
	}

	if(!strcasecmp(cmd,"include:")) {
		nextWord(param);
		char path[2048];
		strcpy(path,"/boot/");
		strcat(path,param);

		printf("*Including config file '%s'\n",param);
		FileIO *fp = sys->openFile(path,"r");
		if(!fp) {
			fp = sys->openFile(param,"r");
		}
		if(fp) {
			scanConfig(fp);
			sys->closeFile(fp);
		}
	}

}


//
//  Full-detail parses
//

void parse(const char *line) {
	char buf[1024];
	char cmd[1024];
	char param[1024];
	const char *word;
	GPIOPin *ground;

	static char curname[256];
	static int curtype;
	static Expression *curexp;


	if(line[0] == '#') {
		return;
	}

	SAFE_STRCPY(buf,line);

	word=findWord(buf,1);
	SAFE_STRCPY(cmd,word);
	nextWord(cmd);

	word=findWord(buf,2);
	SAFE_STRCPY(param,word);

	// Anything there?
	if(!cmd[0]) {
		return;
	}
	if(!param[0]) {
		return;
	}

	// Chain to another config file

	if(!strcasecmp(cmd,"include:")) {
		nextWord(param);
		char path[2048];
		strcpy(path,"/boot/");
		strcat(path,param);
		FileIO *fp = sys->openFile(path,"r");
		if(!fp) {
			fp = sys->openFile(param,"r");
		}
		if(fp) {
			readConfig(fp);
			sys->closeFile(fp);
		}
	}

	// Basic setup, these are single-line commands

	if(!strcasecmp(cmd,"gifdir:")) {
		nextWord(param);
		// Only use it if it exists
		if(sys->access(param,ACCESS_R_OK|ACCESS_X_OK)) {
			SAFE_STRCPY(gifDir,param);
		} else {
			dbprintf("Warning: gifdir '%s' not present\n",param);
		}
	}

	if(!strcasecmp(cmd,"eyecolour:")) {
		nextWord(param);
		uint32_t col  = parseColour(param);
		// Don't let them set it to black
		if(col) {
			eyecolour = col;
		}
	}
	if(!strcasecmp(cmd,"eyecolor:")) {
		nextWord(param);
		uint32_t col  = parseColour(param);
		// Don't let them set it to black
		if(col) {
			eyecolour = col;
		}
	}
	if(!strcasecmp(cmd,"lightcolour:")) {
		nextWord(param);
		lightcolour = parseColour(param);
	}
	if(!strcasecmp(cmd,"lightcolor:")) {
		nextWord(param);
		lightcolour = parseColour(param);
	}
	if(!strcasecmp(cmd,"cooldown:")) {
		nextWord(param);
		cooldown_time  = atoi(param);
		dbprintf("Set cooldown timer to %d sec\n", cooldown_time);
	}
	if(!strcasecmp(cmd,"setrainbow:")) {
		nextWord(param);
		int offset  = atoi(param);
		if(offset < 1 || offset > 16) {
			// "Zippy, what the hell are you talking about?"  "The ****ing Garden of Eden!"
			font.errorMsg("Error: 'setrainbow:' index %d must be 1-16", offset);
		}
		word = findWord(buf,3);
		SAFE_STRCPY(param,word);
		nextWord(param);

		rainbow[offset-1] = parseColour(param);		
		dbprintf("Set rainbow entry %d/16 to #%06x\n", offset, rainbow[offset-1]);
	}
	if(!strcasecmp(cmd,"effect:")) {
		nextWord(param);
		if(!strcasecmp(param,"rainbow_v")) {
			dbprintf("Eye will have a vertical rainbow effect\n");
			set_pattern(PATTERN_V);
		}

		if(!strcasecmp(param,"rainbow_h")) {
			dbprintf("Eye will have a horizontal rainbow effect\n");
			set_pattern(PATTERN_H);
		}

		if(!strcasecmp(param,"rainbow_o")) {
			dbprintf("Eye will have a square rainbow effect\n");
			set_pattern(PATTERN_O);
		}
	}
	if(!strcasecmp(cmd,"rainbowspeed:")) {
		nextWord(param);
		rainbowspeed = atoi(param);
		dbprintf("Set rainbow delay to %d ms\n",rainbowspeed);
	}
	if(!strcasecmp(cmd,"transmitter:")) {
		nextWord(param);
		if(parseTrue(param)) {
			forcetransmitter=true;
		}
	}
	if(!strcasecmp(cmd,"serial:")) {
		nextWord(param);
		SAFE_STRCPY(serialPort,param);
	}
	if(!strcasecmp(cmd,"baud:")) {
		nextWord(param);
		serialRate = atoi(param);
		dbprintf("Set baud rate to %d bits/sec\n",serialRate);
	}
	if((!strcasecmp(cmd,"ackpin:")) || (!strcasecmp(cmd,"ack_pin:"))) {
		nextWord(param);
		ackPin = parseGPIO("AckPin:",param, true, DEVICE_RECEIVER);
		dbprintf("Set ACK pin to %d\n",ackPin->getPin());
	}
	if((!strcasecmp(cmd,"acktime:")) || (!strcasecmp(cmd,"ack_time:"))) {
		nextWord(param);
		ackTime = atoi(param);
		dbprintf("Set ACK light duration to %d ms\n",ackTime);
	}
	if((!strcasecmp(cmd,"micpin:")) || (!strcasecmp(cmd,"mic_pin:"))) {
		nextWord(param);
		mic = parseGPIO("MicPin:",param, false);
		dbprintf("Set microphone pin to %d\n",mic->getPin());
	}
	if((!strcasecmp(cmd,"micinvert:")) || (!strcasecmp(cmd,"mic_invert:"))) {
		nextWord(param);
		if(parseTrue(param)) {
			micInvert=true;
		}
		dbprintf("Microphone inverted = %d\n",micInvert);
	}
	if((!strcasecmp(cmd,"micbright:")) || (!strcasecmp(cmd,"mic_bright:"))) {
		nextWord(param);
		int brightness = atoi(param);
		if(brightness >= 0 && brightness <= 100) {
			micBright=brightness;
			dbprintf("Set microphone bright mode to %d percent\n",brightness);
		}
	}
	if((!strcasecmp(cmd,"micdim:")) || (!strcasecmp(cmd,"mic_dim:"))) {
		nextWord(param);
		int brightness = atoi(param);
		if(brightness >= 0 && brightness <= 100) {
			micDim=brightness;
			dbprintf("Set microphone dim mode to %d percent\n",brightness);
		}
	}
	if((!strcasecmp(cmd,"micdelay:")) || (!strcasecmp(cmd,"mic_delay:"))) {
		nextWord(param);
		int delay = atoi(param);
		if(delay >= 0) {
			micDelay=delay;
			dbprintf("Set detection window to %d ms\n",delay);
		}
	}
	if(!strcasecmp(cmd,"miccolour:")) {
		nextWord(param);
		miccolour = parseColour(param);
	}
	if(!strcasecmp(cmd,"miccolor:")) {
		nextWord(param);
		miccolour = parseColour(param);
	}

	if((!strcasecmp(cmd,"randomchance:")) || (!strcasecmp(cmd,"random_chance:"))) {
		nextWord(param);
		randomChance = atoi(param);
		dbprintf("Set random chance to %d percent\n",randomChance);
	}
	if(!strcasecmp(cmd,"scrollspeed:")) {
		nextWord(param);
		scrollspeed = atoi(param);
		dbprintf("Set scrolling delay to %d ms\n",scrollspeed);
	}
	if(!strcasecmp(cmd,"seamless:")) {
		nextWord(param);
		if(parseTrue(param)) {
			seamless=true;
		}
		if(parseFalse(param)) {
			seamless=false;
		}
	}
	if(!strcasecmp(cmd,"brightness:")) {
		nextWord(param);
		int brightness = atoi(param);
		if(panel) {
			panel->setBrightness(brightness);
		}
		dbprintf("Set panel brightness to %d percent\n",brightness);
	}

	if(!strcasecmp(cmd,"lightpattern:")) {
		nextWord(param);
		if(!strcasecmp(param,"triangle")) {
			dbprintf("Status lights will use a triangle ramp\n");
			set_lightpattern(LIGHTPATTERN_TRIANGLE);
		}

		if(!strcasecmp(param,"saw")) {
			dbprintf("Status lights will use a sawtooth ramp\n");
			set_lightpattern(LIGHTPATTERN_SAW);
		}
	}

	if(!strcasecmp(cmd,"lightspeed:")) {
		nextWord(param);
		lightspeed = atoi(param);
		dbprintf("Set status lights delay to %d ms\n",lightspeed);
	}

	if(!strcasecmp(cmd,"lightbrightness:")) {
		nextWord(param);
		int brightness = atoi(param);
		if(lights) {
			lights->setBrightness(brightness);
			dbprintf("Set lights brightness to %d percent\n",brightness);
		}
	}

	// Force a GPIO pin to act as signal ground
	if((!strcasecmp(cmd,"ground:"))) {
		nextWord(param);
		ground = parseGPIO("ground:",param, true);
		ground->write(true); // 0v owing to inverted logic
		dbprintf("Grounding GPIO pin %d\n",ground->getPin());
	}

	//
	//	Expression setup, this is stateful as it spans multiple lines
	//

	// First define the event trigger and name

	if(!strcasecmp(cmd,"idle:")) {
		SAFE_STRCPY(curname,param);
		nextWord(curname);
		curtype=TRIGGER_IDLE;
		curexp=NULL;
	}
	if(!strcasecmp(cmd,"random:")) {
		SAFE_STRCPY(curname,param);
		nextWord(curname);
		curtype=TRIGGER_RANDOM;
		curexp=NULL;
	}
	if(!strcasecmp(cmd,"gpio:")) {
		SAFE_STRCPY(curname,param);
		nextWord(curname);
		curtype=TRIGGER_GPIO;
		curexp=NULL;
	}
	if(!strcasecmp(cmd,"scripted:")) {
		SAFE_STRCPY(curname,param);
		nextWord(curname);
		curtype=TRIGGER_SCRIPT;
		curexp=NULL;
	}
	if(!strcasecmp(cmd,"sensor:")) {
		SAFE_STRCPY(curname,param);
		nextWord(curname);
		curtype=TRIGGER_SENSOR;
		curexp=NULL;
	}

	// Now say if it's an animation or a scrolly

	if(!strcasecmp(cmd,"gif:")) {
		// Check for silliness
		if(!curname[0]) {
			font.errorMsg("Error: 'gif:' command must have a type and name first, e.g. idle: my_idle_animation");
		}
		if(curexp) {
			font.errorMsg("Error: 'gif:' command used twice for expression '%s'", curname);
		}
		if(expressions.findByName(curname)) {
			font.errorMsg("Expression name '%s' already used", curname);
		}

		// Okay, let's go
		nextWord(param);
		char path[1024];
		makepath(path,param);
		curexp = new GifExpression(path);
		if(!curexp) {
			font.errorMsg("Error creating GIF expression '%s'", curname);
		}
		expressions.add(curexp);

		// Set up the basic stuff, we can do the rest later
		SAFE_STRCPY(curexp->name, curname);
		curexp->trigger = curtype;
		if(curtype != TRIGGER_IDLE) {
			curexp->interruptible=false;
		}
		dbprintf("Added type %s gif expression '%s'\n",videotype(curtype),curname);

		// And reset things just to be safe
		curname[0]=0;
		curtype=TRIGGER_NEVER;
	}

	if(!strcasecmp(cmd,"scroll:")) {
		// Check for silliness
		if(!curname[0]) {
			font.errorMsg("Error: 'scroll:' command must have a type and name first, e.g. idle: my_idle_animation");
		}
		if(curexp) {
			font.errorMsg("Error: 'scroll:' command used twice for expression '%s'", curname);
		}
		if(expressions.findByName(curname)) {
			font.errorMsg("Expression name '%s' already used", curname);
		}

		// Okay, let's go
		curexp = new ScrollExpression(param);
		if(!curexp) {
			font.errorMsg("Error creating Scroll expression '%s'", curname);
		}
		expressions.add(curexp);

		// Set up the basic stuff, we can do the rest later
		SAFE_STRCPY(curexp->name, curname);
		curexp->trigger = curtype;
		if(curtype == TRIGGER_IDLE) {
			curexp->interruptible=false;
		}
		dbprintf("Added type %s scroll expression '%s'\n",videotype(curtype), curname);

		// And reset things just to be safe
		curname[0]=0;
		curtype=TRIGGER_NEVER;
	}

	if(!strcasecmp(cmd,"blink:")) {
		// Check for silliness
		if(!curname[0]) {
			font.errorMsg("Error: 'blink:' command must have a type and name first, e.g. idle: my_idle_animation");
		}
		if(curexp) {
			font.errorMsg("Error: 'blink:' command used twice for expression '%s'", curname);
		}
		if(expressions.findByName(curname)) {
			font.errorMsg("Expression name '%s' already used", curname);
		}

		nextWord(param);
		int blinkmode = parseBlinkmode(param);

		// Okay, let's go
		curexp = new BlinkExpression(blinkmode);
		if(!curexp) {
			font.errorMsg("Error creating Blink expression '%s'", curname);
		}
		expressions.add(curexp);

		// Set up the basic stuff, we can do the rest later
		SAFE_STRCPY(curexp->name, curname);
		curexp->trigger = curtype;
		if(curtype == TRIGGER_IDLE) {
			curexp->interruptible=false;
		}
		dbprintf("Added type %s scroll expression '%s'\n",videotype(curtype), curname);

		// And reset things just to be safe
		curname[0]=0;
		curtype=TRIGGER_NEVER;
	}

	// Now let's deal with the optional(ish) stuff

	if(!strcasecmp(cmd,"chance:")) {
		if(!curexp) {
			font.errorMsg("Error: 'chance:' no expression defined");
		}
		if(curexp->trigger != TRIGGER_RANDOM) {
			font.errorMsg("Error: 'chance:' is only for random expressions");
		}
		nextWord(param);
		curexp->parameter = atoi(param);
		dbprintf("Set chance to %d percent for expression '%s'\n",curexp->parameter,curexp->name);
	}
	
	if(!strcasecmp(cmd,"pin:")) {
		if(!curexp) {
			font.errorMsg("Error: 'pin:' no expression defined");
		}
		if(curexp->trigger != TRIGGER_GPIO) {
			font.errorMsg("Error: 'pin:' is only for GPIO expressions");
		}
		nextWord(param);

		// Force the device to be the transmitter for GPIO-triggered expressions
		curexp->pin = parseGPIO("Pin:", param, false, DEVICE_TRANSMITTER);
		curexp->parameter = curexp->pin->getPin();
		dbprintf("Set gpio pin to %d for expression '%s'\n",curexp->parameter,curexp->name);
	}

	if(!strcasecmp(cmd,"invertpin:")) {
		if(!curexp) {
			font.errorMsg("Error: 'invertpin:' no expression defined");
		}
		if(curexp->trigger != TRIGGER_GPIO) {
			font.errorMsg("Error: 'invertpin:' is only for GPIO expressions");
		}
		if(!curexp->pin) {
			font.errorMsg("Error: 'invertpin:' pin hasn't been declared yet");
		}
		nextWord(param);
		if(parseTrue(param)) {
			curexp->pin->setInverted();
			dbprintf("Set gpio pin %d to use inverted logic for expression '%s'\n",curexp->parameter,curexp->name);
		}

	}

	if(!strcasecmp(cmd,"sensorchannel:")) {
		if(!curexp) {
			font.errorMsg("Error: 'sensorchannel:' no expression defined");
		}
		if(curexp->trigger != TRIGGER_SENSOR) {
			font.errorMsg("Error: 'sensorchannel:' is only for sensor-triggered expressions");
		}
		nextWord(param);

		// Set the channel to use (default is -1 or 'any')
		curexp->sensorchannel = parseSensorChannel(param);
		registerSensorChannel(curexp->sensorchannel);
		dbprintf("Set sensor channel to %d for expression '%s'\n",curexp->parameter,curexp->name);
	}

	if(!strcasecmp(cmd,"drawmode:")) {
		if(!curexp) {
			font.errorMsg("Error: 'drawmode:' no expression defined");
		}
		nextWord(param);
		curexp->drawmode = parseDrawmode(param);
		if(curexp->drawmode < 0) {
			font.errorMsg("Unknown drawmode '%s'", param);
		}
		dbprintf("Set drawmode to %s for expression '%s'\n",param,curexp->name);
	}

	if((!strcasecmp(cmd,"colour:")) || (!strcasecmp(cmd,"color:"))) {
		if(!curexp) {
			font.errorMsg("Error: 'colour:' no expression defined");
		}
		nextWord(param);
		if(parseColour(param)) {
			curexp->colour = parseColour(param);
			if(curexp->drawmode == DRAWMODE_COLOUR) {
				curexp->drawmode = DRAWMODE_MONOCHROME; // Assume they want to see the colour
			}
			dbprintf("Set colour to 0x%x for expression '%s'\n",curexp->colour,curexp->name);
		} else {
			dbprintf("Not setting the eye colour to black\n");
		}
	}

	if(!strcasecmp(cmd,"before:")) {
		if(!curexp) {
			font.errorMsg("Error: 'before:' no expression defined");
		}
		if(curexp->beforeactions >= MAX_ACTIONS) {
			font.errorMsg("Error: expression '%s' has too many before actions", curexp->name);
		}

		add_action(&curexp->before[curexp->beforeactions], param);
		curexp->beforeactions++;
	}

	if(!strcasecmp(cmd,"after:")) {
		if(!curexp) {
			font.errorMsg("Error: 'after:' no expression defined");
		}
		if(curexp->afteractions >= MAX_ACTIONS) {
			font.errorMsg("Error: expression '%s' has too many before actions", curexp->name);
		}

		add_action(&curexp->after[curexp->afteractions], param);
		curexp->afteractions++;
	}

	// By default GIF animations are mirrored for the other eye.  This can disable it for the given animation
	if(!strcasecmp(cmd,"mirror:")) {
		if(!curexp) {
			font.errorMsg("Error: 'mirror:' no expression defined");
		}
		nextWord(param);
		if(parseFalse(param)) {
			curexp->mirror = false;
		}
		if(parseTrue(param)) {
			curexp->mirror = true;
		}
	}

	// By default GIF animations show an ACK light if triggered via GPIO.  This can disable it for the given animation
	if(!strcasecmp(cmd,"ack:")) {
		if(!curexp) {
			font.errorMsg("Error: 'ack:' no expression defined");
		}
		nextWord(param);
		if(parseFalse(param)) {
			curexp->ack = false;
		}
	}

	if(!strcasecmp(cmd,"background:")) {
		if(!curexp) {
			font.errorMsg("Error: 'background:' no expression defined");
		}
		nextWord(param);
		curexp->backgroundname = strdup(param);
	}

	// By default GIF animations show an ACK light if triggered via GPIO.  This can disable it for the given animation
	if(!strcasecmp(cmd,"interruptible:")) {
		if(!curexp) {
			font.errorMsg("Error: 'interruptible:' no expression defined");
		}
		nextWord(param);
		if(parseFalse(param)) {
			curexp->interruptible = false;
		}
		if(parseTrue(param)) {
			curexp->interruptible = true;
		}
	}

	// Enable looping the GIF as a special case since we're deliberately ignoring the NETSCAPE block
	if(!strcasecmp(cmd,"loop:")) {
		if(!curexp) {
			font.errorMsg("Error: 'loop:' no expression defined");
		}
		nextWord(param);
		if(parseFalse(param)) {
			curexp->loop = false;
		}
		if(parseTrue(param)) {
			curexp->loop = true;
			curexp->interruptible = true; // For safety reasons
		}
	}

	if((!strcasecmp(cmd,"scrolltop:")) || (!strcasecmp(cmd,"scroll_top:"))) {
		if(!curexp) {
			font.errorMsg("Error: 'scroll_top:' no expression defined");
		}
		nextWord(param);
		curexp->scrolltop = atoi(param);
		if(curexp->scrolltop < 0 || curexp->scrolltop > 15) {
			curexp->scrolltop = 4;
		}
	}

	if((!strcasecmp(cmd,"blinkspeed:")) || (!strcasecmp(cmd,"blink_speed:"))) {
		if(!curexp) {
			font.errorMsg("Error: 'blink_speed:' no expression defined");
		}
		nextWord(param);
		curexp->blinkspeed = atoi(param);
	}

}

//
//	Add a hook to do something before or after the animation has played
//

void add_action(ExpressionAction *slot, const char *input) {
	char cmd[1024];
	const char *word;
	char param[1024];

	slot->parameter=0;
	slot->strparameter="";

	word = findWord(input, 1);
	SAFE_STRCPY(cmd, word);
	nextWord(cmd);

	word = findWord(input, 2);
	SAFE_STRCPY(param, word);
	nextWord(param);

	if((!strcasecmp(cmd, "setgpio")) || (!strcasecmp(cmd, "set_gpio"))) {
		slot->type = ACTION_SETGPIO;
		slot->pin = parseGPIO(cmd,param,true);
	}

	if((!strcasecmp(cmd, "cleargpio")) || (!strcasecmp(cmd, "clear_gpio"))) {
		slot->type = ACTION_CLEARGPIO;
		slot->pin = parseGPIO(cmd,param,true);
	}

	if((!strcasecmp(cmd, "chain")) || (!strcasecmp(cmd, "play"))) {
		slot->type = ACTION_CHAIN;
		slot->strparameter = strdup(param);
		// Don't check for correctness as the expression might not exist yet
	}

	if((!strcasecmp(cmd, "wait")) || (!strcasecmp(cmd, "delay"))) {
		slot->type = ACTION_WAIT;
		slot->parameter = atoi(param);
	}

	if((!strcasecmp(cmd, "lightcolour")) || (!strcasecmp(cmd, "light_colour"))) {
		slot->type = ACTION_LIGHTCOLOUR;
		slot->parameter = parseColour(param);
	}
	if((!strcasecmp(cmd, "lightcolor")) || (!strcasecmp(cmd, "light_color"))) {
		slot->type = ACTION_LIGHTCOLOUR;
		slot->parameter = parseColour(param);
	}

	if((!strcasecmp(cmd, "lightmode")) || (!strcasecmp(cmd, "light_mode"))) {
		slot->type = ACTION_LIGHTMODE;
		slot->parameter = parseLightmode(param);
	}

	if((!strcasecmp(cmd, "setservo")) || (!strcasecmp(cmd, "set_servo"))) {
		slot->type = ACTION_SETSERVO;
		slot->parameter = atoi(param);
	}

	if((!strcasecmp(cmd, "servospeed")) || (!strcasecmp(cmd, "servo_speed"))) {
		slot->type = ACTION_SERVOSPEED;
		slot->parameter = atoi(param);
	}

	if((!strcasecmp(cmd, "servodelay")) || (!strcasecmp(cmd, "servo_delay"))) {
		slot->type = ACTION_SERVOSPEED;
		slot->parameter = atoi(param);
	}

	if((!strcasecmp(cmd, "seekservo")) || (!strcasecmp(cmd, "seek_servo"))) {
		slot->type = ACTION_SEEKSERVO;
		slot->parameter = atoi(param);
	}
}

//
//  Mostly for debugging, decode the trigger type
//

#ifdef DEBUGGING
const char *videotype(int type) {
	switch(type) {
		case TRIGGER_IDLE:
			return "IDLE";
		case TRIGGER_RANDOM:
			return "RANDOM";
		case TRIGGER_GPIO:
			return "GPIO";
		case TRIGGER_SCRIPT:
			return "SCRIPTED";
		default:
			return "UNSUPPORTED!";
	};
}
#endif

//
// String utils
//


char *nextWord(char *input) {
	char *start=(char *)nextWordStart(input);
	char *end=(char *)nextWordEnd(start);
	*end=0;

	return start;
}

const char *nextWordStart(const char *input) {
	const char *start=input;
	while(isspace(*start)) start++;
	return start;
}

const char *nextWordEnd(const char *input) {
	const char *start=input,*end=NULL;
	while(isspace(*start)) start++;
	end=start;
	while(*end && !isspace(*end)) end++;
	return end;
}

const char *findWord(const char *input, int pos) {
	if(pos < 1) {
		return "";
	}

	const char *ptr=input;
	ptr=nextWordStart(ptr);

	for(int ctr=1;ctr<pos;ctr++) {
		ptr=nextWordEnd(ptr);
		ptr=nextWordStart(ptr);
	}
		
	return ptr;
}

void makepath(char path[1024], const char *filename) {
	int len;
	strcpy(path,gifDir);

	len=strlen(path);

	if(len > 0 && path[strlen(path)-1] != '/') {
		strcat(path,"/");
	}
	strcat(path,filename);
}


uint32_t parseColour(const char *hex) {
	uint32_t col = 0;

	// Look for variables first

	if((!strcasecmp(hex, "eyecolour")) || (!strcasecmp(hex, "eyecolor"))) {
		return eyecolour;
	}
	if((!strcasecmp(hex, "lightcolour")) || (!strcasecmp(hex, "lightcolor"))) {
		return lightcolour;
	}

	// Then try parsing it as an RGB triplet

	if(hex[0] == '#') hex++;
	if(hex[0] == '0' && hex[1] == 'x') hex+=2;

	sscanf(hex,"%x",&col);
	return col;
}

int parseDrawmode(const char *mode) {
	if(!strcasecmp(mode, "colour")) {
		return DRAWMODE_COLOUR;
	}
	if(!strcasecmp(mode, "color")) {
		return DRAWMODE_COLOUR;
	}
	if(!strcasecmp(mode, "monochrome")) {
		return DRAWMODE_MONOCHROME;
	}
	if(!strcasecmp(mode, "mono")) {
		return DRAWMODE_MONOCHROME;
	}
	if(!strcasecmp(mode, "gradient")) {
		return DRAWMODE_GRADIENT;
	}
	if(!strcasecmp(mode, "flash")) {
		return DRAWMODE_FLASH;
	}
	if(!strcasecmp(mode, "flashing")) {
		return DRAWMODE_FLASH;
	}
	if(!strcasecmp(mode, "cycle")) {
		return DRAWMODE_CYCLE;
	}
	return -1;
}

int parseLightmode(const char *mode) {
	if(!strcasecmp(mode, "normal")) {
		return LIGHTMODE_NORMAL;
	}
	if(!strcasecmp(mode, "unison")) {
		return LIGHTMODE_UNISON;
	}
	if(!strcasecmp(mode, "stop")) {
		return LIGHTMODE_STOP;
	}
	return LIGHTMODE_NORMAL;
}

GPIOPin *parseGPIO(const char *cmd, const char *param, bool output, char forceDevice) {

	char device = forceDevice ? forceDevice : DEVICE_BOTH;
	int pin=0;

	if(param[0] == 'T' || param[0] == 't') {
		device = DEVICE_TRANSMITTER;
		param++;
	} else {
		if(param[0] == 'R' || param[0] == 'r') {
			device = DEVICE_RECEIVER;
			param++;
		}
	}

	int len=strlen(param);
	for(int ctr=0;ctr<len;ctr++) {
		if(!isdigit(param[ctr])) {
			font.errorMsg("Error in %s - GPIO pin '%s' is not a number\n", cmd, param);
		}
	}	
	
	pin = atoi(param);
	if(mapPin(pin) < 0) {
		font.errorMsg("Error in %s - Unsupported GPIO pin %s\n", cmd, param);
	}

	GPIOPin *ptr = new GPIOPin(pin,device,output?GPIOMODE_OUTPUT:GPIOMODE_INPUT);
	if(!ptr) {
		font.errorMsg("Error in %s - Error registering GPIO pin %s\n", cmd, param);
	}

	GPIOPin *conflict = ptr->findConflict();
	if(conflict) {
		if(conflict->isReserved()) {
			font.errorMsg("Error in %s - GPIO pin %s reserved by hardware\n", cmd, param);
		}
		font.errorMsg("Error in %s - GPIO pin %s cannot be both INPUT and OUTPUT on the same device\n", cmd, param);
	}

	return ptr;
}

GPIOPin *parseGPIO(const char *cmd, const char *param, bool output) {
	return parseGPIO(cmd, param, output, 0);
}


// Return true if the parameter corresponds to 'true', 'on' or 'yes' etc.  Returns false if not recognised.
bool parseTrue(const char *param) {
	if((!strcasecmp(param, "on")) || (!strcasecmp(param, "true")) || (!strcasecmp(param, "yes"))) {
		return true;
	}
	return false;
}

// Return true if the parameter corresponds to 'false', 'off' or 'no' etc.  Returns false if not recognised.
bool parseFalse(const char *param) {
	if((!strcasecmp(param, "off")) || (!strcasecmp(param, "false")) || (!strcasecmp(param, "no"))) {
		return true;
	}
	return false;
}

int parseBlinkmode(const char *mode) {
	if(!strcasecmp(mode, "default")) {
		return BLINK_TOP;
	}
	if(!strcasecmp(mode, "left")) {
		return BLINK_LEFT;
	}
	if(!strcasecmp(mode, "right")) {
		return BLINK_RIGHT;
	}
	if(!strcasecmp(mode, "top")) {
		return BLINK_TOP;
	}
	if(!strcasecmp(mode, "bottom")) {
		return BLINK_BOTTOM;
	}
	if((!strcasecmp(mode, "vertical")) || (!strcasecmp(mode, "vert"))) {
		return BLINK_VERT;
	}
	if((!strcasecmp(mode, "horizontal")) || (!strcasecmp(mode, "horiz"))) {
		return BLINK_HORIZ;
	}
	if((!strcasecmp(mode, "square")) || (!strcasecmp(mode, "all"))) {
		return BLINK_ALL;
	}

	font.errorMsg("Unsupported blink mode '%s'",mode);
	return 0;
}

int parseSensorChannel(const char *channel) {
	if(!strcasecmp(channel, "any")) {
		return -1;
	}

	return atoi(channel);
}

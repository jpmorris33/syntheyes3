//
//  Configuration reader
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "syntheyes.hpp"

void readConfig(FILE *fp);
void parse(const char *line);
void makepath(char path[1024], const char *filename);

static const char *videotype(int type);
static char *nextWord(char *input);
static const char *nextWordStart(const char *input);
static const char *nextWordEnd(const char *input);
static const char *findWord(const char *input, int pos);
static uint32_t parseColour(const char *hex);
static int parseDrawmode(const char *mode);
static void add_event(ExpressionEvent *slot, const char *input);

extern int mapPin(int pin);
extern char gifDir[512];
extern int cooldown_time;
extern int rainbowspeed;
extern bool forcetransmitter;

//
//  Config reader
//

void readConfig(FILE *fp) {
	char buf[1024];

	for(;;) {
		if(feof(fp)) {
			return;
		}
		buf[0]=0;
		if(fgets(buf,1024,fp)) {
			parse(buf);
		}
	}
}


void parse(const char *line) {
	char buf[1024];
	char cmd[1024];
	char param[1024];
	const char *word;

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


	// Basic setup, these are single-line commands

	if(!strcasecmp(cmd,"gifdir:")) {
		nextWord(param);
		SAFE_STRCPY(gifDir,param);
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
	if(!strcasecmp(cmd,"cooldown:")) {
		nextWord(param);
		cooldown_time  = atoi(param);
		printf("Set cooldown timer to %d sec\n", cooldown_time);
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
		printf("Set rainbow entry %d/16 to #%06x\n", offset, rainbow[offset-1]);
	}
	if(!strcasecmp(cmd,"effect:")) {
		nextWord(param);
		if(!strcasecmp(param,"rainbow_v")) {
			printf("Eye will have a vertical rainbow effect\n");
			set_pattern(PATTERN_V);
		}

		if(!strcasecmp(param,"rainbow_h")) {
			printf("Eye will have a horizontal rainbow effect\n");
			set_pattern(PATTERN_H);
		}

		if(!strcasecmp(param,"rainbow_o")) {
			printf("Eye will have a square rainbow effect\n");
			set_pattern(PATTERN_O);
		}
	}
	if(!strcasecmp(cmd,"rainbowspeed:")) {
		nextWord(param);
		rainbowspeed = atoi(param);
		printf("Set rainbow delay to %d ticks\n",rainbowspeed);
	}
	if(!strcasecmp(cmd,"transmitter:")) {
		nextWord(param);
		if(!strcasecmp(param,"true")) {
			forcetransmitter=true;
		}
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

	// Now say if it's an animation or a scrolly

	if(!strcasecmp(cmd,"gif:")) {
		// Check for silliness
		if(!curname) {
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
			curexp->interruptable=false;
		}
		printf("Added type %s gif expression '%s'\n",videotype(curtype),curname);

		// And reset things just to be safe
		curname[0]=0;
		curtype=TRIGGER_NEVER;
	}

	if(!strcasecmp(cmd,"scroll:")) {
		// Check for silliness
		if(!curname) {
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
			curexp->interruptable=false;
		}
		printf("Added type %s scroll expression '%s'\n",videotype(curtype), curname);

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
		printf("Set chance to %d percent for expression '%s'\n",curexp->parameter,curexp->name);
	}
	
	if(!strcasecmp(cmd,"pin:")) {
		if(!curexp) {
			font.errorMsg("Error: 'pin:' no expression defined");
		}
		if(curexp->trigger != TRIGGER_GPIO) {
			font.errorMsg("Error: 'pin:' is only for GPIO expressions");
		}
		nextWord(param);
		curexp->parameter = atoi(param);

		curexp->parameter=mapPin(curexp->parameter);
		if(curexp->parameter < 0) {
			font.errorMsg("Error in Pin: Unsupported GPIO pin %s\n", param);
		}
		printf("Set gpio pin to %d for expression '%s'\n",curexp->parameter,curexp->name);
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
		printf("Set drawmode to %s for expression '%s'\n",param,curexp->name);
	}

	if((!strcasecmp(cmd,"colour:")) || (!strcasecmp(cmd,"color:"))) {
		if(!curexp) {
			font.errorMsg("Error: 'colour:' no expression defined");
		}
		nextWord(param);
		if(parseColour(param)) {
			curexp->colour = parseColour(param);
			curexp->drawmode = DRAWMODE_MONOCHROME; // Assume they want to see the colour
			printf("Set colour to 0x%x for expression '%s'\n",curexp->colour,curexp->name);
		} else {
			printf("Not setting the eye colour to black\n");
		}
	}

	if(!strcasecmp(cmd,"before:")) {
		if(!curexp) {
			font.errorMsg("Error: 'before:' no expression defined");
		}
		if(curexp->beforeevents >= MAX_EVENTS) {
			font.errorMsg("Error: expression '%s' has too many before events", curexp->name);
		}

		add_event(&curexp->before[curexp->beforeevents], param);
		curexp->beforeevents++;
	}

	if(!strcasecmp(cmd,"after:")) {
		if(!curexp) {
			font.errorMsg("Error: 'after:' no expression defined");
		}
		if(curexp->afterevents >= MAX_EVENTS) {
			font.errorMsg("Error: expression '%s' has too many before events", curexp->name);
		}

		add_event(&curexp->after[curexp->afterevents], param);
		curexp->afterevents++;
	}

	if(!strcasecmp(cmd,"mirror:")) {
		if(!curexp) {
			font.errorMsg("Error: 'mirror:' no expression defined");
		}
		nextWord(param);
		if((!strcasecmp(param, "off")) || (!strcasecmp(param, "false"))) {
			curexp->mirror = false;
		}
	}

}

//
//	Add a hook to do something before or after the animation has played
//

void add_event(ExpressionEvent *slot, const char *input) {
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
		slot->type = EVENT_SETGPIO;
		slot->parameter = atoi(param);
		slot->parameter=mapPin(slot->parameter);
		if(slot->parameter < 0) {
			font.errorMsg("Error in SetGPIO: Unsupported GPIO pin %s\n", param);
		}
	}

	if((!strcasecmp(cmd, "cleargpio")) || (!strcasecmp(cmd, "clear_gpio"))) {
		slot->type = EVENT_CLEARGPIO;
		slot->parameter = atoi(param);
		slot->parameter=mapPin(slot->parameter);
		if(slot->parameter < 0) {
			font.errorMsg("Error in ClearGPIO: Unsupported GPIO pin %s\n", param);
		}
	}

	if((!strcasecmp(cmd, "chain")) || (!strcasecmp(cmd, "play"))) {
		slot->type = EVENT_CHAIN;
		slot->strparameter = strdup(param);
		// Don't check for correctness as the expression might not exist yet
	}
}

//
//  Mostly for debugging, decode the trigger type
//

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
	return -1;
}

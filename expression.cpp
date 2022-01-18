#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "syntheyes.hpp"


extern Expression *nextState;
extern bool transmitter;
extern bool flash_state;

extern void wait(int ms, bool interruptable);
static int compareProb(const void *a, const void *b);
extern void init_pin(int pin);


void Expression::play() {};

void Expression::event(ExpressionEvent *ev) {
	Expression *exp=NULL;

	switch(ev->type) {
		case EVENT_SETGPIO:
			set_pin(ev->parameter, true);
			break;
		case EVENT_CLEARGPIO:
			set_pin(ev->parameter, false);
			break;
		case EVENT_CHAIN:
			exp = expressions.findByName(ev->strparameter);
			if(exp) {
				// If you want to set up an infinite loop, that's not my problem
				exp->play();
			} else {
				printf("Did not find animation '%s'\n", ev->strparameter);
			}
			break;
		default:
			break;
	}
}


GifExpression::GifExpression(const char *path) {

	interruptable=true;
	mirror=true;
	trigger=TRIGGER_NEVER;
	parameter=0;
	drawmode = DRAWMODE_COLOUR;
	colour = eyecolour;
	beforeevents=0;
	afterevents=0;
	next = NULL;

	gif = loadgif(path);
	if(!gif) {
		font.errorMsg("Error: could not find video file '%s'\n",path);
	}
}

void GifExpression::play() {
	int delay;

	for(int ctr=0;ctr<beforeevents;ctr++) {
		event(&before[ctr]);
	}

	for(int ctr=0;ctr<gif->frames;ctr++) {
		delay = drawFrame(ctr)/10;
		for(int ctr2=0;ctr2<delay;ctr2++) {
			wait(10,interruptable);
			drawFrame(ctr);
			if(interruptable && nextState) {
				break;
			}
		}

		if(interruptable && nextState) {
			break;
		}
	}

	for(int ctr=0;ctr<afterevents;ctr++) {
		event(&after[ctr]);
	}

}


int GifExpression::drawFrame(int frame) {

	if(frame > gif->frames) {
		frame = gif->frames-1;
	}
	if(frame<0) {
		frame=0;
	}

	panel->clear(0);
	switch(drawmode) {
		case DRAWMODE_COLOUR:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h);
			break;
		case DRAWMODE_MONOCHROME:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h, colour);
			break;
		case DRAWMODE_GRADIENT:
			panel->updateRGBpattern(gif->frame[frame].imgdata,gif->w,gif->h, rainbowoffset);
			break;
		case DRAWMODE_FLASH:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h, flash_state ? colour : 0);
			break;
		default:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h);
		break;
	};

	if(transmitter || (!mirror)) {
		panel->draw();
	} else {
		panel->drawMirrored();
	}

	return gif->frame[frame].delay;
}




ScrollExpression::ScrollExpression(const char *message) {
	text = strdup(message);

	interruptable=false;
	mirror=false; // Mirroring the scrolly doesn't make much sense, but I suppose it could be implemented in future
	trigger=TRIGGER_NEVER;
	parameter=0;
	drawmode = DRAWMODE_COLOUR;
	colour = eyecolour;
	beforeevents=0;
	afterevents=0;
	next = NULL;

}

void ScrollExpression::play() {

	for(int ctr=0;ctr<beforeevents;ctr++) {
		event(&before[ctr]);
	}

	switch(drawmode) {
		case DRAWMODE_COLOUR:
		case DRAWMODE_MONOCHROME:
			font.scroll(text,4,colour);
			break;
		case DRAWMODE_GRADIENT:
			font.scroll(text,4,colour,rainbowoffset);
			break;
		default:
			font.scroll(text,4,colour);
		break;
	};


	for(int ctr=0;ctr<afterevents;ctr++) {
		event(&after[ctr]);
	}

}





ExpressionSet::ExpressionSet(int size) {

	if(size < 1) {
		set=NULL;
		len=0;
		return;
	}

	set=(Expression **)calloc(sizeof(Expression *), size);
	len=size;
}

ExpressionSet::~ExpressionSet() {

	if(set) {
		free(set);
		set=NULL;
	}
}

int ExpressionSet::count() {
	return len;
}

bool ExpressionSet::put(int pos, Expression *ptr) {
	if(!set) {
		return false;
	}
	if(pos<0 || pos >= len) {
		return false;
	}

	set[pos] = ptr;
	return true;
}

Expression *ExpressionSet::get(int pos) {
	if(!set || !len || pos < 0 || pos >= len) {
		return NULL;
	}
	return set[pos];
}

void ExpressionSet::sortByProbability() {
	if(set && len) {
		qsort(set,len,sizeof(Expression *),compareProb);
	}
}

Expression *ExpressionSet::getRandom() {
	if(!set || !len) {
		return NULL;
	}
	return set[random()%len];
}

Expression *ExpressionSet::getRandomWeighted() {
	int chance,ctr;
	if(!set || !len) {
		return NULL;
	}

	sortByProbability();

	for(ctr=0;ctr<len;ctr++) {
		chance = random()%100;
		if(set[ctr]->parameter >= chance) {
			return set[ctr];
		}
	}
	return NULL;
}


ExpressionList::ExpressionList() {
	anchor=NULL;
}

void ExpressionList::add(Expression *e) {

	e->next = NULL;
	if(!anchor) {
		anchor = e;
		return;
	}

	if(anchor && !anchor->next) {
		anchor->next = e;
		return;
	}

	// Scoot to the end and add it
	Expression *ptr;
	for(ptr=anchor;ptr->next;ptr=ptr->next);
	ptr->next = e;
}

//
//	High level expression picker
//

Expression *ExpressionList::pick(int type) {
	ExpressionSet *list = findAllByTrigger(type);
	Expression *result=NULL;

	if(!list) {
		return result; // Null
	}

	if(type == TRIGGER_RANDOM) {
		result = list->getRandomWeighted();
	} else {
		result = list->getRandom();
	}

	delete list;
	return result;
}

Expression *ExpressionList::findByName(const char *name) {
	if(!anchor || !name) {
		return NULL;
	}

	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(!strcasecmp(name, ptr->name)) {
			return ptr;
		}
	}

	return NULL;

}



Expression *ExpressionList::findFirstByTrigger(int type) {
	if(!anchor) {
		return NULL;
	}

	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == type) {
			return ptr;
		}
	}

	return NULL;

}

ExpressionSet *ExpressionList::findAllByTrigger(int type) {

	if(!anchor) {
		return NULL;
	}

	int total=0;
	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == type) {
			total++;
		}
	}

	if(!total) {
		return NULL;
	}

	ExpressionSet *list = new ExpressionSet(total);
	if(!list) {
		return list;
	}

	int ctr=0;
	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == type) {
			list->put(ctr++, ptr);
		}
	}

	return list;

}


ExpressionSet *ExpressionList::findByGPIO(int pin) {

	if(!anchor) {
		return NULL;
	}

	int total=0;
	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == TRIGGER_GPIO && ptr->parameter == pin) {
			total++;
		}
	}

	if(!total) {
		return NULL;
	}

	ExpressionSet *list = new ExpressionSet(total);
	if(!list) {
		return list;
	}

	int ctr=0;
	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == TRIGGER_GPIO && ptr->parameter == pin) {
			list->put(ctr++, ptr);
		}
	}

	return list;
}

void ExpressionList::initGPIO() {
	if(!anchor) {
		return;
	}

	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == TRIGGER_GPIO) {
			init_pin(ptr->parameter);
		}
	}
}



static int compareProb(const void *a, const void *b) {
	Expression *va=*(Expression **)a;
	Expression *vb=*(Expression **)b;

	// We only want to do this for random videos
	if(va->trigger != TRIGGER_RANDOM && vb->trigger != TRIGGER_RANDOM) {
		return 0;
	}
	if(va->trigger != TRIGGER_RANDOM) {
		return -1;
	}
	if(vb->trigger != TRIGGER_RANDOM) {
		return +1;
	}
	return vb->parameter-va->parameter;
}


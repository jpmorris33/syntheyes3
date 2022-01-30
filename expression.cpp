//
//  Expression rendering classes (GifExpression, ScrollExpression, BlinkExpression)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "syntheyes.hpp"


extern Expression *nextExpression;
extern Expression *idle;
extern bool transmitter;
extern bool flash_state;

extern void wait(int ms, bool interruptible);

static void drawEyelid(int mode, int pos, int width, int height);


//
//  Base class
//

void Expression::play() {};

void Expression::drawFirstFrame() {};

void Expression::initDefaults() {
	interruptible=true;
	mirror=true;
	ack=true;
	loop=false;
	scrolltop=0;
	blinkspeed=6;
	background=NULL;
	backgroundname=NULL;
	trigger=TRIGGER_NEVER;
	parameter=0;
	drawmode = DRAWMODE_COLOUR;
	colour = eyecolour;
	beforeevents=0;
	afterevents=0;
	next = NULL;
}

void Expression::event(ExpressionEvent *ev) {
	Expression *exp=NULL;

	switch(ev->type) {
		case EVENT_SETGPIO:
			if(ev->pin) {
				ev->pin->write(true);
			}
			break;
		case EVENT_CLEARGPIO:
			if(ev->pin) {
				ev->pin->write(false);
			}
			break;
		case EVENT_CHAIN:
			exp = expressions.findByName(ev->strparameter);
			if(exp) {
				// If you want to set up an infinite loop, that's not my problem
				exp->play();
			} else {
				printf("Warning: CHAIN did not find animation '%s'\n", ev->strparameter);
			}
			break;
		default:
			break;
	}
}

//
//  GIF animations
//

GifExpression::GifExpression(const char *path) {

	initDefaults();

	gif = loadgif(path);
	if(!gif) {
		font.errorMsg("Error: could not find video file '%s'\n",path);
	}
}

void GifExpression::play() {
	int delay;
	bool stop=false;

	for(int ctr=0;ctr<beforeevents;ctr++) {
		event(&before[ctr]);
	}

	do {
		for(int ctr=0;ctr<gif->frames;ctr++) {
			delay = drawFrame(ctr)/10;
			for(int ctr2=0;ctr2<delay;ctr2++) {
				wait(10,interruptible);
				drawFrame(ctr);
				// If something has come up and we're interruptible, stop
				if(interruptible && nextExpression) {
					stop=true;
					break;
				}
			}

			if(stop) {
				// Bounce out of the outer loop as well
				// We still want to run the event hooks in case GPIOs need resetting
				// TODO: May need to prevent chain() events happening in this case
				break;
			}
		}
	} while(loop && (!stop));

	for(int ctr=0;ctr<afterevents;ctr++) {
		event(&after[ctr]);
	}

}

void GifExpression::drawFirstFrame() {
	drawFrameOnly(0);
}


int GifExpression::drawFrame(int frame) {

	if(frame > gif->frames) {
		frame = gif->frames-1;
	}
	if(frame<0) {
		frame=0;
	}

	panel->clear(0);
	if(background) {
		background->drawFirstFrame();
	}

	drawFrameOnly(frame);

	if(transmitter || (!mirror)) {
		panel->draw();
	} else {
		panel->drawMirrored();
	}

	return gif->frame[frame].delay;
}

void GifExpression::drawFrameOnly(int frame) {
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
		case DRAWMODE_CYCLE:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h, rainbow[rainbowoffset&0x0f]);
			break;
		default:
			panel->updateRGB(gif->frame[frame].imgdata,gif->w,gif->h);
		break;
	};
}

//
//  Scrollers
//


ScrollExpression::ScrollExpression(const char *message) {

	initDefaults();

	interruptible=false;
	mirror=false; // Mirroring the scrolly doesn't make much sense, but I suppose it could be implemented in future
	scrolltop=SCROLL_TOP_DEFAULT;

	text = strdup(message);
}

void ScrollExpression::play() {

	for(int ctr=0;ctr<beforeevents;ctr++) {
		event(&before[ctr]);
	}

	do {
		switch(drawmode) {
			case DRAWMODE_COLOUR:
			case DRAWMODE_MONOCHROME:
				font.scroll(text, scrolltop, colour, interruptible, false);
				break;
			case DRAWMODE_GRADIENT:
				font.scroll(text, scrolltop, colour, interruptible, true);
				break;
			case DRAWMODE_FLASH:
				font.scroll(text, scrolltop, flash_state ? colour : 0, interruptible, false);
				break;
			case DRAWMODE_CYCLE:
				font.scroll(text, scrolltop, rainbow[rainbowoffset&0x0f], interruptible, false);
				break;
			default:
				font.scroll(text, scrolltop, colour, interruptible, false);
			break;
		};
		if(interruptible && nextExpression) {
			break;
		}

	} while(loop);


	for(int ctr=0;ctr<afterevents;ctr++) {
		event(&after[ctr]);
	}

}

void ScrollExpression::drawFirstFrame() {};


//
//  Procedural blinking
//

BlinkExpression::BlinkExpression(int mode) {

	initDefaults();

	interruptible=false;
	scrolltop=0;

	blinkmode=mode;
}

void BlinkExpression::play() {
	bool stop=false;
	int height = panel->getH();
	int width = panel->getW();
	int pos=0;
	int frames=height;
	int maxpos,halfway;

	if(blinkmode & BLINK_HORIZ) {
		if(width > height) {
			frames=width;
		}
	}

	maxpos = frames * 2;	// Both directions
	maxpos += frames/4;	// Add some extra for a delay when shut
	halfway = maxpos/2;

	for(int ctr=0;ctr<beforeevents;ctr++) {
		event(&before[ctr]);
	}

	do {
		for(int ctr=0;ctr<maxpos;ctr++) {

			for(int ctr2=0;ctr2<blinkspeed;ctr2++) {
				wait(1,interruptible);

				if(interruptible && nextExpression) {
					stop=true;
					break;
				}

				panel->clear(0);
				if(background) {
					background->drawFirstFrame();
				}

				drawEyelid(blinkmode,pos,width,height);

				if(transmitter || (!mirror)) {
					panel->draw();
				} else {
					panel->drawMirrored();
				}
			}

			if(stop) {
				// Bounce out of the outer loop as well
				// We still want to run the event hooks in case GPIOs need resetting
				break;
			}

			// Pick the direction and update the blink position
			if(ctr < halfway) {
				pos++;
			} else {
				pos--;
			}

		}
	} while(loop);

	for(int ctr=0;ctr<afterevents;ctr++) {
		event(&after[ctr]);
	}
}

void BlinkExpression::drawFirstFrame() {};


void drawEyelid(int blinkmode, int pos, int width, int height) {
	int ctr;
	width--;
	height--;

	if(blinkmode & BLINK_TOP) {
		for(ctr=0;ctr<=pos;ctr++) {
			panel->clearH(ctr,0);
		}
	}
	if(blinkmode & BLINK_BOTTOM) {
		for(ctr=0;ctr<=pos;ctr++) {
			panel->clearH(height-ctr,0);
		}
	}
	if(blinkmode & BLINK_LEFT) {
		for(ctr=0;ctr<=pos;ctr++) {
			panel->clearV(ctr,0);
		}
	}
	if(blinkmode & BLINK_RIGHT) {
		for(ctr=0;ctr<=pos;ctr++) {
			panel->clearV(width-ctr,0);
		}
	}
}

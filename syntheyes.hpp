#ifndef __SYNTHEYES_HPP__
#define __SYNTHEYES_HPP__

#define VERSION "3.00"

#include "expression.hpp"
#include "font.hpp"
#include "drivers/PanelDriver.hpp"
#include "drivers/SerialDriver.hpp"
#include "drivers/Timing.hpp"

extern PanelDriver *panel;
extern Timing *timing;
extern Font font;
extern ExpressionList expressions;
extern uint32_t eyecolour;
extern uint32_t rainbow[16]; // Colour table
extern unsigned char rainbowoffset;
extern char serialPort[256];
extern int serialRate;

extern void set_pin(int pin, bool state);
extern void set_pattern(int pattern);
extern void update_rainbow();

#define SAFE_STRCPY(a,b) {strncpy(a,b,sizeof(a));a[sizeof(a)-1]=0;}

#ifdef DEBUGGING
	#define dbprintf printf
#else
	#define dbprintf(...)  ;
#endif


// Rainbow patterns

#define PATTERN_V 0
#define PATTERN_H 1
#define PATTERN_O 2

#endif

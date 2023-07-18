#ifndef __EXPRESSION_HPP__
#define __EXPRESSION_HPP__

#include <stdbool.h>
#include <stdint.h>

#define TRIGGER_NEVER 0
#define TRIGGER_IDLE 1
#define TRIGGER_RANDOM 2
#define TRIGGER_GPIO 3
#define TRIGGER_SCRIPT 4
#define TRIGGER_SENSOR 5

#define DRAWMODE_COLOUR 0
#define DRAWMODE_MONOCHROME 1
#define DRAWMODE_GRADIENT 2
#define DRAWMODE_FLASH 3
#define DRAWMODE_CYCLE 4

#define ACTION_NONE		0
#define ACTION_SETGPIO		1
#define ACTION_CLEARGPIO	2
#define ACTION_CHAIN		3
#define ACTION_WAIT		4
#define ACTION_LIGHTCOLOUR	5
#define ACTION_LIGHTMODE	6
#define ACTION_SETSERVO		7
#define ACTION_SERVOSPEED	8
#define ACTION_SEEKSERVO	9

#define BLINK_TOP 	1
#define BLINK_LEFT	2
#define BLINK_RIGHT	4
#define BLINK_BOTTOM	8
#define BLINK_VERT (BLINK_TOP|BLINK_BOTTOM)
#define BLINK_HORIZ (BLINK_LEFT|BLINK_RIGHT)
#define BLINK_ALL (BLINK_TOP|BLINK_BOTTOM|BLINK_LEFT|BLINK_RIGHT)

#define SCROLL_TOP_DEFAULT 4

#include "gpio.hpp"
#include "gifload.hpp"

//
//	Supported expression types
//

#define MAX_ACTIONS 8

struct ExpressionAction {
	int type;
	int parameter;
	GPIOPin *pin;
	const char *strparameter;
};

class Expression {
	public:
		virtual void play();
		virtual void drawFirstFrame();
		void action(ExpressionAction *ev);

		char name[256];
		int trigger;	// Idle, Random, GPIO etc
		bool interruptible;
		bool mirror;
		bool ack;
		bool loop;
		int blinkspeed;
		int scrolltop;
		Expression *background;
		const char *backgroundname;
		int parameter;
		int sensorchannel;
		GPIOPin *pin;
		int drawmode;
		uint32_t colour;
		ExpressionAction before[MAX_ACTIONS];
		ExpressionAction after[MAX_ACTIONS];
		unsigned char beforeactions;
		unsigned char afteractions;

		Expression *next;
	protected:
		void initDefaults();
};

class GifExpression : public Expression {
	public:
		GifExpression(const char *path);
		void play();
		void drawFirstFrame();
	private:
		int drawFrame(int frame);
		void drawFrameOnly(int frame);
		GIFANIM *gif;
};

class ScrollExpression : public Expression {
	public:
		ScrollExpression(const char *message);
		void play();
		void drawFirstFrame();

		char *text;
};

class BlinkExpression : public Expression {
	public:
		BlinkExpression(int mode);
		void play();
		void drawFirstFrame();
	private:
		int blinkmode;
};

class ExpressionSet {
	public:
		ExpressionSet(int size);
		~ExpressionSet();
		bool put(int pos, Expression *ptr);
		Expression *get(int pos);
		Expression *getRandom();
		Expression *getRandomWeighted();
		int count();

	private:
		void sortByProbability();

		Expression **set;
		int len;
};



class ExpressionList {

	public:
		ExpressionList();
		void add(Expression *e);
		Expression *pick(int triggertype);
		Expression *findByName(const char *name);
		Expression *findFirstByTrigger(int triggertype);
		ExpressionSet *findAllByTrigger(int triggertype);
		ExpressionSet *findByGPIO(int pin);
		ExpressionSet *findBySensor(int channel); // Can be -1 for 'any'
		void initBackgrounds();
	private:
		Expression *anchor;


};


#endif
//
//  Expression management classes (ExpressionList, ExpressionSet)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "syntheyes.hpp"

static int compareProb(const void *a, const void *b);

ExpressionSet::ExpressionSet(int size) {

	if(size < 1) {
		set=NULL;
		len=0;
		return;
	}

	set=(Expression **)sys->alloc(sizeof(Expression *) * size);
	len=size;
}

ExpressionSet::~ExpressionSet() {

	if(set) {
		sys->free(set);
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
		if(ptr->trigger == TRIGGER_GPIO && ptr->pin && ptr->parameter == pin) {
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
		if(ptr->trigger == TRIGGER_GPIO && ptr->pin && ptr->parameter == pin) {
			list->put(ctr++, ptr);
		}
	}

	return list;
}

ExpressionSet *ExpressionList::findBySensor(int channel) {

	if(!anchor) {
		return NULL;
	}

	int total=0;
	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->trigger == TRIGGER_SENSOR && (ptr->sensorchannel == -1 || ptr->sensorchannel == channel || channel == -1)) {
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
		if(ptr->trigger == TRIGGER_SENSOR && (ptr->sensorchannel == -1 || ptr->sensorchannel == channel || channel == -1)) {
			list->put(ctr++, ptr);
		}
	}

	return list;
}

void ExpressionList::initBackgrounds() {
	if(!anchor) {
		return;
	}

	for(Expression *ptr=anchor;ptr;ptr=ptr->next) {
		if(ptr->backgroundname) {
			ptr->background = findByName(ptr->backgroundname);
			if(!ptr->background) {
				font.errorMsg("Error: background expression '%s' not found", ptr->backgroundname);
			}
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


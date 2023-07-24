//
//	Parser
//

#include <string.h>
#include <stdlib.h>
#include "../syntheyes.hpp"

static void clean(char *input);

const char *getDriverParam(const char *string, const char *cmd) {
	if((!string) || (!cmd)) {
		return NULL;
	}

	int cmdlen = strlen(cmd);
	const char *ptr = strstr(string,cmd);
	if(!ptr) {
		return NULL;
	}
	ptr += cmdlen;
	if(*ptr != '=')	{
		return NULL;
	}
	return ++ptr;
}

int getDriverInt(const char *param) {
	char word[256];
	SAFE_STRCPY(word, param);
	clean(word);

	char *ptr = strchr(word,',');
	if(ptr) {
		*ptr=0;
	}

	return atoi(word);
}

int getDriverHex(const char *param) {
	char word[256];
	SAFE_STRCPY(word, param);
	clean(word);

	char *ptr = strchr(word,',');
	if(ptr) {
		*ptr=0;
	}

	return strtol(word,NULL,16);
}

const char *getDriverStr(const char *param) {
	static char word[256];
	SAFE_STRCPY(word, param);
	clean(word);

	char *ptr = strchr(word,',');
	if(ptr) {
		*ptr=0;
	}
	return word;
}

void clean(char *input) {
	char *ptr=strchr(input,0x0a);
	if(ptr) {
		*ptr=0;
	}
	ptr=strchr(input,0x0d);
	if(ptr) {
		*ptr=0;
	}
}

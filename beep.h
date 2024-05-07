#ifndef BEEP_C
#define BEEP_C

#include "common.h"
char *getBeep(void);
int getbeepLength(void);

char *getBeep(){
	char beep[512] = {
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11,
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11, 
	0x11, 0x11, 0x11, 0x11
	};
	return beep;	
}
int getbeepLength(){
	int beepLength = 512;
	return beepLength;
}
#endif
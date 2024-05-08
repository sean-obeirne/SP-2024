#include "users.h"
#include "ulib.h"
#include "audio.h"
#include "beep.h"


USERMAIN( sound ) {
	
	int n;
	int count =30;
	char c = 's';
	char buf[128];

	playSound((uint8_t *)getBeep(), getbeepLength());

	

	return (42);

}

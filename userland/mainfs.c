#ifndef MAIN_FS_H_
#define MAIN_FS_H_

#include "users.h"
#include "ulib.h"
#include "filesystem.h"
#include "fshelper.h"
#include "ramdisk.h"

/**
** User function main #1:  exit, write
**
** Prints its ID, then loops N times delaying and printing, then exits.
** Verifies the return byte count from each call to write().
**
** Invoked as:  mainfs  x  n
**	 where x is the ID character
**		   n is the iteration count
*/

USERMAIN( mainfs ) {
	int count = 30; // default iteration count
	char ch = '1';	// default character to print
	char buf[128];	// local char buffer

	// process the command-line arguments
	switch( argc ) {
	case 3:	count = str2int( argv[2], 10 );
			// FALL THROUGH
	case 2:	ch = argv[1][0];
			break;
	default:
			sprint( buf, "mainfs: argc %d, args: ", argc );
			cwrites( buf );
			for( int i = 0; i <= argc; ++i ) {
				sprint( buf, " %s", argv[argc] ? argv[argc] : "(null)" );
				cwrites( buf );
			}
			cwrites( "\n" );
	}

	// announce our presence
	int n = swritech( ch );
	if( n != 1 ) {
		sprint( buf, "== %c, write #1 returned %d\n", ch, n );
		cwrites( buf );
	}

	// iterate and print the required number of other characters
	for( int i = 0; i < count; ++i ) {
		DELAY(STD);
		n = swritech( ch );
		if( n != 1 ) {
			sprint( buf, "== %c, write #2 returned %d\n", ch, n );
			cwrites( buf );
		}
	}

	// all done!
	exit( 0 );

	// should never reach this code; if we do, something is
	// wrong with exit(), so we'll report it

	char msg[] = "*1*";
	msg[1] = ch;
	n = write( CHAN_SIO, msg, 3 );	  /* shouldn't happen! */
	if( n != 3 ) {
		sprint( buf, "User %c, write #3 returned %d\n", ch, n );
		cwrites( buf );
	}

	// this should really get us out of here
	return( 42 );
}

#endif

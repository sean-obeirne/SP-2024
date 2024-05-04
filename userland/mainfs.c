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

	ph("FILESYSTEM");

	const char *path = "";
	const char *filename = "";
	// DirectoryEntry *root = _fs_find_entry("/");
	DirectoryEntry *de = NULL;
	show_header_info(true);
	filename = "/test/file";
	DirectoryEntry *entry = _fs_create_file(filename);
	filename = "/test/file2";
	DirectoryEntry *entry2 = _fs_create_file(filename);
	filename = "/you/will/be/impressed";
	DirectoryEntry *entry3 = _fs_create_file(filename);
	filename = "/told/you/so";
	DirectoryEntry *entry4 = _fs_create_dir(filename);
	filename = "/told/you/LOL";
	DirectoryEntry *entry5 = _fs_create_file(filename);
	filename = "/told/myself/nothing";
	DirectoryEntry *entry6 = _fs_create_dir(filename);
	plnn();
	dr();
	plnn();
	filename = "/told/you/LOL";
	DirectoryEntry *found = _fs_find_entry(filename);
	_fs_print_entry(found);
	pln();
	
	plw('@');
	
	exit ( 42 );
	return ( 42 );
}

#endif

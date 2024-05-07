/*
** File: fshelper.h
** Description: This file includes a bunch of helper functions to facilitate
** the implementation of a filesystem. Also defines useful constants
**
** @author Sean O'Beirne
*/

#ifndef FS_HELPER_H_
#define FS_HELPER_H_
#if 1  // BEG variables

// #include <stdint.h> // Include necessary standard headers
#include "common.h"
#include "filesystem.h"
#include "ramdisk.h"

// Define constants, macros, and data structures specific to FAT32 filesystem


// Print debugging info, such as current operation or success status
////////////////
// #define DEBUG
////////////////
// Pause time definitions
#define SLEEP_FACTOR 2
#define STEP 1
#define FIVER 5
#define CSTEP 1 * SLEEP_FACTOR
#define MOMENT 3 * SLEEP_FACTOR
#define SHORT_PAUSE 20 * SLEEP_FACTOR
#define DEBUG_DELAY 50 * SLEEP_FACTOR
#define LONG_PAUSE 100 * SLEEP_FACTOR
#define INF_PAUSE 1000000 * SLEEP_FACTOR

// Useful ASCII codes
#define MAX_ASCII_CHAR 127 // 128+ seems invalid
#define FILLED_CIRCLE 7
#define OPEN_CIRCLE 9

typedef struct DirectoryEntry DirectoryEntry;
typedef struct DeconstructedPath DeconstructedPath;
typedef enum EntryType EntryType;
#endif // END Variables

#if 1  // BEG functions
void show_header_info(bool_t horrizontal);

void phn(const char *header, int ticks);
void ph(const char *header);
void pl(void);
void pl_debug(void);
void pln(void);
void plnn(void);
void plw(char line_char);
void plh(const char *header, char line_char);
void phl(const char *header);
void pvl(const char *header, char line_char, int indent);
void nl(void);

void box_h(int max_item_size);
void box_v(char *to_print);
void box_pad_right(int longest_line);
void print_chars(void);
void d(void);

void init_fs_buffer(void);
void clear_fs_buffer(void);
int dump_fs_buffer(void);
void pb(void);
void dr(void);
void dump_fat(void);
bool_t is_root(const char *path);

void get_path( void );
void parse_input(int in_len); // from buffer
void run_command(char **args, int word_count);
void merge_path(char *path);

char *strip_path(const char *path);
void clean_nwd( void );
void parse_path(const char *path);
void test_parse_path(const char *path);
void print_parsed_path(void);

uint16_t allocate_cluster(void);
int get_next_cluster(int current_cluster);
void update_fat_entry(uint16_t cluster, uint16_t value);

DirectoryEntry *create_sub_entry(DirectoryEntry *parent, const char *filename, EntryType type);
int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert);
DirectoryEntry *find_or_create_entry( void );

int add_fat_entry(uint32_t next_cluster);

int get_subdirectory_count(DirectoryEntry *parent);
int dir_contains(DirectoryEntry *parent, const char *target);
void adjust_cwd( DeconstructedPath *cwd, const char *path);
int cd_parent( void );

#endif // END functions

#endif /* FS_HELPER_H_ */

#if 1  // TODO Useful REGEX queries
/*
Useful REGEX queries:

find all delays
(STEP|FIVER|CSTEP|MOMENT|SHORT_PAUSE|DEBUG_DELAY|LONG_PAUSE|INF_PAUSE)

find non-STEP delays
__delay\((?!STEP\)).*?\)

*/
#endif // TODO Useful REGEX queries
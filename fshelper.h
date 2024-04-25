#ifndef FS_HELPER_H_
#define FS_HELPER_H_

// #include <stdint.h> // Include necessary standard headers
#include "common.h"
#include "filesystem.h"
#include "ramdisk.h"

// Define constants, macros, and data structures specific to FAT32 filesystem
// Print general debugging info, such as current operation or success status
////////////////
#define DEBUG
////////////////

// Constants for FAT32 filesystem implementation
#define MAX_FAT_ENTRIES 20
#define FAT_EOC 0xFFFF
#define FAT_FREE 0x0
#define FAT_IN_USE 0x1
#define DISK_SIZE 16 // disk is this many blocks
#define BLOCK_SIZE 4096 // TODO SEAN: make this pull from somewhere
#define TOTAL_SIZE (DISK_SIZE * BLOCK_SIZE)
#define SECTOR_SIZE 512 // 8 sectors = 1 block
#define ROOT_DIRECTORY_ENTRIES 32
// #define ROOT_DIRECTORY_ENTRIES 4
#define MAX_FILENAME_LENGTH 255
#define MAX_PATH_LENGTH 1023
#define MAX_DEPTH 16
#define FS_BUFFER_SIZE 4096

// Pause time definitions
#define SLEEP_FACTOR 3
#define STEP 0
#define FIVER 5
#define CSTEP 1 * SLEEP_FACTOR
#define MOMENT 3 * SLEEP_FACTOR
#define SHORT_PAUSE 20 * SLEEP_FACTOR
#define DEBUG_DELAY 50 * SLEEP_FACTOR
#define LONG_PAUSE 100 * SLEEP_FACTOR
#define INF_PAUSE 1000000 * SLEEP_FACTOR

// Useful ASCII codes
#define FILLED_CIRCLE 7
#define OPEN_CIRCLE 9

typedef struct DirectoryEntry DirectoryEntry;
typedef struct DeconstructedPath DeconstructedPath;
typedef enum EntryAttribute EntryAttribute;



// Print general debugging info, such as current operation or success status
// #define DEBUG

void phn(const char *header, int ticks);
void ph(const char *header);
void pl(void);
void pln(void);
void plw(char line_char);
void plh(const char *header, char line_char);
void phl(const char *header);
void pvl(const char *header, char line_char, int indent);
void print_chars(void);
void init_fs_buffer(void);
void clear_fs_buffer(void);
int dump_fs_buffer(void);
int dump_root(void);
void dump_fat(void);
char *strip_path(const char *path);
void parse_path(const char *path, DeconstructedPath *dp);
void test_parse_path(const char *path);
void print_parsed_path(DeconstructedPath dp);
int create_sub_entry(DirectoryEntry *parent, const char *filename, EntryAttribute type);
int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert);
int add_fat_entry(uint32_t next_cluster);
int get_subdirectory_count(DirectoryEntry *parent);
int dir_contains(DirectoryEntry *parent, const char *target);

#endif /* FS_HELPER_H_ */


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
#define SLEEP_FACTOR 15
#define STEP 1 * SLEEP_FACTOR
#define MOMENT 20 * SLEEP_FACTOR
#define DEBUG_DELAY 50 * SLEEP_FACTOR
#define SHORT_PAUSE 100 * SLEEP_FACTOR
#define LONG_PAUSE 500 * SLEEP_FACTOR
#define INF_PAUSE 1000000 * SLEEP_FACTOR

// Print general debugging info, such as current operation or success status
// #define DEBUG

void phn(const char *header);
void ph(const char *header);
void pl(void);
void pln(void);
void plw(char line_char);
void plh(const char *header, char line_char);
void phl(const char *header);
void pvl(const char *header, char line_char);
void init_fs_buffer(void);
void clear_fs_buffer(void);
int dump_fs_buffer(void);
int dump_root(void);
void dump_fat(void);
void strip_path(const char *path, char *final_element);
void parse_path(const char *path, DeconstructedPath *dp);
void test_parsed_path(const char *path);
void print_parsed_path(DeconstructedPath dp);
int create_subentry(DirectoryEntry *parent, const char *filename, EntryAttribute type);
int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert);
int add_fat_entry(uint32_t next_cluster);
int get_subdirectory_count(DirectoryEntry *parent);
int dir_contains(DirectoryEntry *parent, const char *target);

#endif /* FS_HELPER_H_ */


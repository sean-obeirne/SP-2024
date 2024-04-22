#ifndef FS_HELPER_H_
#define FS_HELPER_H_

// #include <stdint.h> // Include necessary standard headers
#include "common.h"
#include "filesystem.h"
#include "ramdisk.h"

// Define constants, macros, and data structures specific to FAT32 filesystem

// Print general debugging info, such as current operation or success status
// #define DEBUG

void ph(const char *header);
void pl(void);
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
void old_parse_path(const char *path, char **dir_names, char *file_name, int *num_dirs);
void parse_path(const char *path, DeconstructedPath *dp);
DirectoryEntry *test_parse_path(const char *path);
int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert);
int add_fat_entry(uint32_t next_cluster);
int get_subdirectory_count(DirectoryEntry *parent);
int dir_contains(DirectoryEntry *parent, const char *target);

#endif /* FS_HELPER_H_ */


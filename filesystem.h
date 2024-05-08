/*
** File: fshelper.h
** Description: This file includes the full set of public filesystem functions
** Also defines structs for the filesystem implementation
**
** @author Sean O'Beirne
*/

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
#if 1  // BEG Definitions

#include "common.h"
#include "ramdisk.h"
#include "fshelper.h"

// FAT constants
#define MAX_FAT_ENTRIES 64 // total entries in FAT
#define FAT_EOC 0xFFFF // FAT end of cluster value
#define FAT_FREE 0x0 // FAT cluster is free
#define FAT_IN_USE 0x1 // FAT cluster is not free

// Disk constants
#define DISK_SIZE 16 // disk is this many blocks
#define BLOCK_SIZE SZ_PAGE // set block_size to size of page for consistentcy
#define TOTAL_SIZE (DISK_SIZE * BLOCK_SIZE) // num of bytes in total
#define ROOT_DIRECTORY_ENTRIES 8 // how many entries can reside in root?

// String constants
#define MAX_COMMANDS 8 // number of words in command as a max
#define MAX_INPUT 255 // number of characters to allow as input
#define MAX_FILENAME_LENGTH 31 // Max length of a filename string
#define MAX_PATH_LENGTH 255 // max path length at any given time


typedef enum EntryType {
    FILE = 0x01,      // Type for files
    DIRECTORY = 0x02  // Type for directories
} EntryType;

typedef enum PathType {
    ABSOLUTE = 0x01, 	// Absolute path
    RELATIVE = 0x02 	// Relative path
} PathType;

typedef enum OpType {
    FIND = 0x01, 	// Find operation
    CREATE = 0x02 	// Create operation
} OpType;

// Define DirectoryEntry structure
typedef struct DirectoryEntry {
    char filename[MAX_FILENAME_LENGTH + 1];   // Name of the file or directory
    uint32_t size;					// Size of the file in bytes
    EntryType type;					// File or directory
    uint32_t cluster;				// Starting cluster of the file's data
    struct DirectoryEntry *next;	// Pointer to the next directory entry
	uint8_t depth;					// How many directories are we from root?
    struct Directory *subdirectory;	// Pointer to the subdirectory if applicable
	char path[MAX_PATH_LENGTH + 1];	// Path for this entry
} DirectoryEntry;

typedef struct Directory {
    DirectoryEntry *files[ROOT_DIRECTORY_ENTRIES]; // Array of dir entries
    uint32_t num_files;	// Number of files in the directory
} Directory;

typedef struct DeconstructedPath {
    char filename[MAX_FILENAME_LENGTH];	// String to store file name
	char path[MAX_PATH_LENGTH];			// String to store full path string
    uint8_t num_dirs;					// Number of directory names
    char *dirs[MAX_FILENAME_LENGTH];	// Array to store directory names
    char *paths[MAX_FILENAME_LENGTH];	// Array to store path entries
	PathType path_type;					// Absolute or Relative?
	EntryType entry_type;				// File or Directory?
	OpType op_type;						// Create or find?
} DeconstructedPath;

typedef struct FATEntry {
	uint32_t next_cluster;	// next cluster for this FATEntry
	uint8_t status;			// status (FAT_EOC, FAT_FREE, FAT_IN_USE)
} FATEntry;

typedef struct FAT {
	FATEntry entries[MAX_FAT_ENTRIES]; // Array of FAT entries
} FAT;

typedef struct FileSystem {
	uint16_t bytes_per_cluster;			// how many bytes are in this cluster?
	uint16_t reserved_cluster_count;	// how many clusters reserved for FS?
	uint32_t total_clusters;			// total num of clusters

	// FAT Information
	FAT *fat;
	uint16_t fat_entry_size;

	// File System Metadata
	char volume_label[12];
	char file_system_type[8];

	// Disk (RAMdisk for now)
	StorageInterface disk;

	// General purpose cache or buffer
	char *buffer;

	// Current working directory (as path string)
	char cwd[MAX_PATH_LENGTH + 1];

	// Mounted status
	bool_t mounted;
} FileSystem;
#endif // END Definitions

#if 1  // BEG Directories

/*
** Open Directory
** @param path Path of the directory to open.
** @return Pointer to the opened Directory structure, NULL on failure.
*/
Directory *open_dir(const char *path);

/*
** Close Directory
** @param dir Pointer to the Directory structure to close.
** @return 0 on success, -1 on failure.
*/
int close_dir(Directory *dir);

/*
** List Directory Contents
** @param dir Pointer to the Directory structure.
** @param box to set if you would like a box for output
** @return 0 on success, -1 on failure.
*/
int list_dir_contents(const char *dir_name, bool_t box);

/*
** Change Directory
** @param path Path of the directory to change to.
** @return 0 on success, -1 on failure.
*/
int change_dir(const char *path);

/*
** Move or Rename Directory
** @param old_path Current path of the directory.
** @param new_path New path for the directory.
** @return 0 on success, -1 on failure.
*/
int move_dir(const char *old_path, const char *new_path);

#endif // END Directories

#if 1  // BEG Filesystem

/**
** _fs_shell - callback routine for console input
**
** Called by the CIO module when a key is pressed on the
** console keyboard.  Depending on the key, it will print
** statistics on the console display, or will cause the
** user fs_shell process to be dispatched.
**
** This code runs as part of the CIO ISR.
*/
void _fs_shell( int code );

/*
 ** Init the filesystem.
 ** @return 0 on success, -1 on failure.
 */
int _fs_init(void);

/*
 ** Mount the filesystem.
 ** @return 0 on success, -1 on failure.
 */
int _fs_mount(void);

/*
 ** Read data from a file in the filesystem.
 ** @param filename Name of the file to read.
 ** @param buffer Pointer to the buffer to store the read data.
 ** @param size Number of bytes to read.
 ** @param offset Offset within the file to start reading from.
 ** @return Number of bytes read on success, -1 on failure.
 */
int _fs_read_file(const char *filename/*, void *buffer, size_t size, off_t offset*/);

/*
 ** Write data to a file in the filesystem.
 ** @param filename Name of the file to write to.
 ** @param buffer Pointer to the buffer containing the data to write.
 ** @param size Number of bytes to write.
 ** @param offset Offset within the file to start writing from.
 ** @return Number of bytes written on success, -1 on failure.
 */
int _fs_write_file(const char *path, const void *data/*, size_t size, off_t offset*/);

/*
 ** Find an entry in the filesystem.
 ** @param path Path of the entry to find.
 ** @return new entry, NULL on failure
 */
DirectoryEntry *_fs_find_entry(const char *path);

/*
 ** Create a new file in the filesystem.
 ** @param path Path of the entry to create.
 ** @return new file, NULL on failure
 */
DirectoryEntry *_fs_create_file(const char *path);

/*
 ** Create a new directory in the filesystem.
 ** @param path Path of the entry to create.
 ** @return new directory, NULL on failure
 */
DirectoryEntry *_fs_create_dir(const char *path);

/*
 ** Delete a file from the filesystem.
 ** @param filename Name of the file to delete.
 ** @return 0 on success, -1 on failure.
 */
int _fs_delete_entry(const char *filename);

/*
 ** Rename a file in the filesystem.
 ** @param old_filename Current name of the file.
 ** @param new_filename New name for the file.
 ** @return 0 on success, -1 on failure.
 */
int _fs_rename_entry(const char *old_filename, const char *new_filename);

/*
 ** Open a file in the filesystem for reading or writing.
 ** @param filename Name of the file to open.
 ** @param mode Mode in which to open the file ("r" for read, "w" for write).
 ** @return File descriptor on success, -1 on failure.
 */
int _fs_open_file(const char *filename, const char *mode);

/*
 ** Close an open file in the filesystem.
 ** @param fd File descriptor of the file to close.
 ** @return 0 on success, -1 on failure.
 */
int _fs_close_file(const char *filename);

/*
 ** Print a DirectoryEntry *
 ** @param filename of the file to print.
 ** @param print_children true for recursion
 ** @return 0 on success, -1 on failure.
 */
int _fs_print_entry(DirectoryEntry *entry);

/*
 ** Set the permissions of a file in the filesystem.
 ** @param filename Name of the file to set permissions for.
 ** @param permissions New permissions for the file.
 ** @return 0 on success, -1 on failure.
 */
// int _fs_set_permissions(const char *filename, mode_t permissions);


void _fs_initialize_directory_entry(DirectoryEntry *entry, const char *filename, uint32_t size, EntryType type, uint32_t cluster, DirectoryEntry *next, uint8_t depth, const char *path);

#endif // END Filesystem
#endif /* FILESYSTEM_H_ */


#ifndef FAT32_H_
#define FAT32_H_
#if 1  // END Definitions

#include "common.h"
#include "ramdisk.h"
#include "fshelper.h"

// Define constants, macros, and data structures specific to FAT32 filesystem


typedef enum EntryType {
    FILE = 0x01,      // Attribute for files
    DIRECTORY = 0x02  // Attribute for directories
} EntryType;

typedef enum PathType {
    ABSOLUTE = 0x01, 	// Absolute path
    RELATIVE = 0x02 	// Relative path
} PathType;

typedef enum OpType {
    FIND = 0x01, 	// Absolute path
    CREATE = 0x02 	// Relative path
} OpType;

// Define DirectoryEntry structure
typedef struct DirectoryEntry {
    char filename[MAX_FILENAME_LENGTH + 1];   // Name of the file or directory
    uint32_t size;                            // Size of the file in bytes
    EntryType type;                           // File or directory
    uint32_t cluster;                         // Starting cluster of the file's data
    struct DirectoryEntry *next;              // Pointer to the next directory entry in the linked list
	uint8_t depth;
    struct Directory *subdirectory;           // Pointer to the subdirectory (if it's a directory)
	char path[MAX_PATH_LENGTH + 1];
} DirectoryEntry;

typedef struct Directory {
    DirectoryEntry *files[ROOT_DIRECTORY_ENTRIES]; // Array of directory entries (files)
    uint32_t num_files; // Number of files in the directory
    // Add any other metadata or properties you need for directories
} Directory;

typedef struct DeconstructedPath {
    char filename[MAX_FILENAME_LENGTH];	// String to store file name
	char path[MAX_PATH_LENGTH];			// String to store full path string
    uint8_t num_dirs;  					// Number of directory names
    char *dirs[MAX_FILENAME_LENGTH];  	// Array to store directory names
    char *paths[MAX_FILENAME_LENGTH];  	// Array to store path entries
	PathType path_type;
	EntryType entry_type;
	OpType op_type;
} DeconstructedPath; // TODO SEAN: to fix, check for 'first /' not '/ first

typedef struct FATEntry {
	uint32_t next_cluster;
	uint8_t status;
} FATEntry;

typedef struct FAT {
	FATEntry entries[MAX_FAT_ENTRIES]; // Array of FAT entries
} FAT;

typedef struct FileSystem {
	// Boot Sector Information
	uint16_t bytes_per_cluster;
	uint16_t reserved_cluster_count;
	uint32_t total_clusters;

	// FAT Information
	FAT *fat;
	uint16_t fat_entry_size;

	// File System Metadata
	char volume_label[12];
	char file_system_type[8];

	// Disk (RAMdisk)
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
** Print Working Directory
*/
void pwd( void );

/*
** Create Directory
** @param path Path of the directory to create.
** @return 0 on success, -1 on failure.
*/
int create_dir(const char *path);

/*
** Delete Directory
** @param path Path of the directory to delete.
** @return 0 on success, -1 on failure.
*/
int delete_dir(const char *path);

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
** Get Current Directory
** @return Path of the current directory.
*/
const char *get_current_dir( void );

/*
** Move or Rename Directory
** @param old_path Current path of the directory.
** @param new_path New path for the directory.
** @return 0 on success, -1 on failure.
*/
int move_dir(const char *old_path, const char *new_path);

/*
** Check if Directory Exists
** @param path Path of the directory to check.
** @return 1 if directory exists, 0 if not.
*/
int dir_exists(const char *path);


/*
** Get Directory Metadata
** @param path Path of the directory.
** @param metadata Pointer to the DirectoryMetadata structure to store metadata.
** @return 0 on success, -1 on failure.
*/
// int get_directory_metadata(const char *path, DirectoryMetadata *metadata);

/*
** Traverse Directory Tree
** @param root_path Root path of the directory tree to traverse.
** @param callback Pointer to the callback function to be called for each directory.
** @return 0 on success, -1 on failure.
*/
DirectoryEntry *traverse_directory(DirectoryEntry *directory_entry, int depth, const char *path_to_search);
// void traverse_directory(Directory *directory, int depth);

#endif // END Directories

#if 1  // BEG Filesystem

/**
** _fs_shell - callback routine for console input
**
** Called by the CIO module when a key is pressed on the
** console keyboard.  Depending on the key, it will print
** statistics on the console display, or will cause the
** user shell process to be dispatched.
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

// Add more function prototypes as needed for your FAT32 filesystem implementation

#endif // END Filesystem

#endif /* FAT32_H_ */


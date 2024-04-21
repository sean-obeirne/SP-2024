#ifndef FAT32_H_
#define FAT32_H_

// #include <stdint.h> // Include necessary standard headers
#include "common.h"
#include "ramdisk.h"

// Define constants, macros, and data structures specific to FAT32 filesystem

// Print general debugging info, such as current operation or success status

///////////////
#define DEBUG
///////////////
#define DEBUG_DELAY 25


// Constants for FAT32 filesystem
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
// #define ROOT_LEN 512 // how many files can be in root?


typedef enum {
    FILE_ATTRIBUTE = 0x01,      // Attribute for files
    DIRECTORY_ATTRIBUTE = 0x02  // Attribute for directories
} EntryAttribute;

// Define DirectoryEntry structure
typedef struct DirectoryEntry {
    char filename[MAX_FILENAME_LENGTH + 1];   // Name of the file or directory
    uint32_t size;                            // Size of the file in bytes
    EntryAttribute type;                      // Attribute of the file (e.g., file or directory)
    uint32_t cluster;                         // Starting cluster of the file's data
    struct DirectoryEntry *next;              // Pointer to the next directory entry in the linked list
    struct Directory *subdirectory;           // Pointer to the subdirectory (if it's a directory)
} DirectoryEntry;

typedef struct Directory {
    char name[MAX_FILENAME_LENGTH]; // Name of the directory
    DirectoryEntry files[ROOT_DIRECTORY_ENTRIES]; // Array of directory entries (files)
    uint32_t num_files; // Number of files in the directory
    // Add any other metadata or properties you need for directories
} Directory;

typedef struct {
	char path[MAX_PATH_LENGTH];
    char *paths[MAX_FILENAME_LENGTH];  // Array to store path entries
    char *dirs[MAX_FILENAME_LENGTH];  // Array to store directory names
    char file_name[MAX_FILENAME_LENGTH];  // String to store file name
    int num_dirs;  // Number of directory names
} DeconstructedPath;

typedef struct {
	uint32_t next_cluster;
	uint8_t status;
} FATEntry;

typedef struct FAT {
	FATEntry entries[MAX_FAT_ENTRIES]; // Array of FAT entries
									   // Additional fields or metadata related to the FAT table
} FAT;

typedef struct {
	// Define your filesystem structures here
	// e.g., directory entry structure, FAT entry structure, etc.
	// Boot Sector Information
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t number_of_fats;
	uint32_t total_sectors;
	uint32_t fat_size_sectors;
	uint32_t root_directory_cluster;

	// FAT Information
	FAT *fat;
	uint16_t fat_entry_size;

	// Root Directory Information
	uint16_t root_directory_entries;
	DirectoryEntry *root_directory;

	// Current Directory Information
	uint32_t current_directory_cluster;

	// File System Metadata
	char volume_label[12];
	char file_system_type[8];

	// Disk Information
	StorageInterface disk;

	// Cache or Buffer
	void *buffer;

	// Mounted status
	bool_t mounted;

	// Error Handling
	int last_error;
} FileSystem;


/*********DIRECTORUIES*********/
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
** @return 0 on success, -1 on failure.
*/
int list_dir_contents(const char *dir_name);

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
int traverse_directory_tree(const char *root_path, void (*callback)(const char *path));

/*************************************************************/

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
 ** Find a DirectoryEntry from root.
 ** @param filename of the file to find.
 ** @return DirectoryEntry with filename in root
 */
DirectoryEntry *_fs_find_entry(const char *filename);

/*
 ** Find a DirectoryEntry
 ** @param path of the file to find.
 ** @return DirectoryEntry with path "path"
 */
DirectoryEntry *_fs_find_entry_from_path(const char *path);

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
int _fs_write_file(const char *filename, const void *data/*, size_t size, off_t offset*/);

/*
 ** Create a new file in the filesystem.
 ** @param filename Name of the file to create.
 ** @return 0 on success, -1 on failure.
 */
int _fs_create_entry(const char *filename, EntryAttribute type);

/*
 ** Delete a file from the filesystem.
 ** @param filename Name of the file to delete.
 ** @return 0 on success, -1 on failure.
 */
int _fs_delete_file(const char *filename);

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
 ** Find a DirectoryEntry * for a given filename.
 ** @param filename of the file to print.
 ** @return 0 on success, -1 on failure.
 */
int _fs_print_entry(const char *filename);

/*
 ** Set the permissions of a file in the filesystem.
 ** @param filename Name of the file to set permissions for.
 ** @param permissions New permissions for the file.
 ** @return 0 on success, -1 on failure.
 */
// int _fs_set_permissions(const char *filename, mode_t permissions);

// Add more function prototypes as needed for your FAT32 filesystem implementation


#endif /* FAT32_H_ */


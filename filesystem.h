#ifndef FAT32_H_
#define FAT32_H_

// #include <stdint.h> // Include necessary standard headers
#include "common.h"

// Define constants, macros, and data structures specific to FAT32 filesystem

// Constants for FAT32 filesystem
#define MAX_FAT_ENTRIES 1000
#define FAT_EOC 0xFFFF
#define DISK_SIZE 1024 // disk is this many blocks
#define BLOCK_SIZE 4096 // TODO SEAN: make this pull from somewhere
#define TOTAL_SIZE (DISK_SIZE * BLOCK_SIZE)
#define SECTOR_SIZE 512 // 8 sectors = 1 block
#define ROOT_DIRECTORY_ENTRIES 32
// #define ROOT_DIRECTORY_ENTRIES 4
#define MAX_FILENAME_LENGTH 255
#define FS_BUFFER_SIZE 4096
// #define ROOT_LEN 512 // how many files can be in root?


typedef struct {
    char filename[MAX_FILENAME_LENGTH + 1];  // Name of the file or directory
    uint32_t size;                       // Size of the file in bytes
    uint8_t attributes;                  // Attributes of the file (e.g., read-only, hidden, directory)
    uint32_t block;                      // Starting block of the file's data
    // Add more fields as needed for your filesystem implementation
} DirectoryEntry;

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
    void *disk;

    // Cache or Buffer
    void *buffer;

    // Mounted status
    bool_t mounted;

    // Error Handling
    int last_error;
} FileSystem;

/*
** Dump root directory to CIO
** @return 0 on success, -1 on failure.
*/
int dump_root( void );


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
int _fs_write_file(const char *filename, const void *data/*, size_t size, off_t offset*/);

/*
** Create a new file in the filesystem.
** @param filename Name of the file to create.
** @return 0 on success, -1 on failure.
*/
int _fs_create_file(const char *filename);

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
int _fs_rename_file(const char *old_filename, const char *new_filename);

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
** Set the permissions of a file in the filesystem.
** @param filename Name of the file to set permissions for.
** @param permissions New permissions for the file.
** @return 0 on success, -1 on failure.
*/
// int _fs_set_permissions(const char *filename, mode_t permissions);

// Add more function prototypes as needed for your FAT32 filesystem implementation


#endif /* FAT32_H_ */


#ifndef FAT32_H_
#define FAT32_H_

// #include <stdint.h> // Include necessary standard headers
#include "common.h"

// Define constants, macros, and data structures specific to FAT32 filesystem

// Constants for FAT32 filesystem
#define MAX_FAT_ENTRIES 1000
#define FAT_EOC 0xFFFF
#define BLOCK_SIZE 4096 // TODO SEAN: make this pull from somewhere
#define SECTOR_SIZE 512
#define MAX_FILENAME_LENGTH 255
// #define ROOT_LEN 512 // how many files can be in root?

// Example data structures for FAT32 filesystem
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
    uint32_t *fat;
    uint16_t fat_entry_size;

    // Root Directory Information
    uint8_t *root_directory;
    uint16_t root_directory_entries;

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

// Function prototypes for FAT32 filesystem operations

/**
 * Initialize the FAT32 filesystem.
 * @param fs Pointer to the filesystem structure to initialize.
 * @param disk Pointer to the disk device.
 * @return 0 on success, -1 on failure.
 */
int _fs_init( void );

/**
 * Mount the FAT32 filesystem.
 * @param fs Pointer to the filesystem structure to mount.
 * @return 0 on success, -1 on failure.
 */
int _fs_mount( void );

/**
 * Read data from a file in the FAT32 filesystem.
 * @param fs Pointer to the mounted filesystem structure.
 * @param filename Name of the file to read.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size Number of bytes to read.
 * @param offset Offset within the file to start reading from.
 * @return Number of bytes read on success, -1 on failure.
 */
int _fs_read_file(const char *filename, void *buffer, uint32_t size, int32_t offset);

/**
 * Write data to a file in the FAT32 filesystem.
 * @param fs Pointer to the mounted filesystem structure.
 * @param filename Name of the file to write.
 * @param buffer Pointer to the data to write.
 * @param size Number of bytes to write.
 * @param offset Offset within the file to start writing to.
 * @return Number of bytes written on success, -1 on failure.
 */
int _fs_write_file(const char *filename, const void *buffer, uint32_t size, int32_t offset);

// Add more function prototypes as needed for your FAT32 filesystem implementation

#endif /* FAT32_H_ */


#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "kmem.h" // Include memory management

// #include <stdio.h> // Include necessary standard headers

// Declare global variables for FileSystem and disk
static FileSystem fs;
static void *disk;

int _fs_init( void ) {
    // Implement FAT32 initialization logic here
    // Initialize the filesystem structure and perform any necessary setup
    
    // Initialize the file system struct with appropriate values
    fs.bytes_per_sector = 512;
    fs.sectors_per_cluster = 8;
    fs.reserved_sector_count = 1;
    fs.number_of_fats = 2;
    fs.total_sectors = 2880;
    fs.fat_size_sectors = 0;
    fs.root_directory_cluster = 2;

    // FAT Information
    fs.fat = _km_page_alloc( 10 );
    if( fs.fat == NULL ){
      return -1;
    }
    
    fs.fat_entry_size = 4;

    // Root Directory Information
    fs.root_directory = _km_page_alloc( 10 );
    if( fs.root_directory == NULL ){
      //TODO SEAN: free fs.fat
      return -1;
    }
    
    fs.root_directory_entries = 32;

    // Current Directory Information
    fs.current_directory_cluster = fs.root_directory_cluster ;

    // File System Metadata
    for (int i = 0; i < 12; i++) {
        fs.volume_label[i] = "VLABEL"[i];
    }
    fs.volume_label[12] = '\0';

    for (int i = 0; i < 8; i++) {
        fs.file_system_type[i] = "FAT32"[i];
    }
    fs.file_system_type[8] = '\0';

    // Disk Information
    // fs.disk = disk;
    fs.disk = NULL;

    // Cache or Buffer
    fs.buffer = _km_page_alloc( 10 );
    if( fs.root_directory == NULL ){
      //TODO SEAN: free fs.fat
      //TODO SEAN: free fs.root_directory
      return -1;
    }

    // Error Handling
    fs.last_error = 0;

    // Return 0 on success
    return 0;
}

int _fs_mount(FileSystem *fs) {
    // Implement FAT32 mounting logic here
    // Mount the filesystem using the provided filesystem structure
    return 0; // Return 0 on success
}

int _fs_read_file(FileSystem *fs, const char *filename, void *buffer, uint32_t size, int32_t offset) {
    // Implement FAT32 file read logic here
    // Read data from the specified file into the buffer
    return -1; // Return -1 for now as this is a stub
}

int _fs_write_file(FileSystem *fs, const char *filename, const void *buffer, uint32_t size, int32_t offset) {
    // Implement FAT32 file write logic here
    // Write data from the buffer to the specified file
    return -1; // Return -1 for now as this is a stub
}

// Implement additional filesystem operations as needed



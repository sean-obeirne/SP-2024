#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management

// #include <stdio.h> // Include necessary standard headers


typedef struct {
  uint32_t next_cluster;
  uint8_t status;
} FATEntry;

typedef struct FAT {
    FATEntry entries[MAX_FAT_ENTRIES]; // Array of FAT entries
    // Additional fields or metadata related to the FAT table
} FAT;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];  // Name of the file or directory
    uint32_t size;                       // Size of the file in bytes
    uint8_t attributes;                  // Attributes of the file (e.g., read-only, hidden, directory)
    uint32_t cluster;                    // Starting cluster of the file's data
    // Add more fields as needed for your filesystem implementation
} DirectoryEntry;

// Declare global variables for FileSystem and disk
static FileSystem fs;
static void *disk;

int _fs_init( void ) {
    // Implement FAT32 initialization logic here
    // Initialize the filesystem structure and perform any necessary setup
    
    // Initialize the file system struct with appropriate values
    fs.bytes_per_sector = SECTOR_SIZE;
    fs.sectors_per_cluster = 8;
    fs.reserved_sector_count = 1;
    fs.number_of_fats = 2;
    fs.total_sectors = 2880;
    fs.fat_size_sectors = 16; // fat size in sectors
    fs.root_directory_cluster = 2; // root is at cluster _
//Q? do clusters=blocks=pages? and do sectors=segments?

    // FAT Information
    fs.fat = _km_page_alloc( fs.fat_size_sectors / fs.sectors_per_cluster );
    // __cio_printf("THIS VALUE: %d IS IT", fs.fat_size_sectors / fs.sectors_per_cluster);
    if( fs.fat == NULL ){
      return -1;
    }
    
    fs.fat_entry_size = 4;

    // Root Directory Information
    fs.root_directory_entries = 32;
    // fs.root_directory = _km_page_alloc( fs.fat_entry_size * fs.root_directory_entries );
    fs.root_directory = _km_page_alloc( (sizeof(DirectoryEntry) * fs.root_directory_entries )); // TODO SEAN: do not allocate pages, but bytes
    if( fs.root_directory == NULL ){
      //TODO SEAN: free fs.fat
      return -1;
    }
    
    // Current Directory Information
    fs.current_directory_cluster = fs.root_directory_cluster;

    // File System Metadata
    for (int i = 0; i < 12; i++) {
        fs.volume_label[i] = "VLABEL"[i];
    }
    fs.volume_label[12] = '\0';

    for (int i = 0; i < 8; i++) {
        fs.file_system_type[i] = "FAT32"[i];
    }
    fs.file_system_type[8] = '\0';

  // i have to find the device entry point
    // Disk Information
    // fs.disk = disk;
    fs.disk = NULL;

    // Cache or Buffer
    fs.buffer = _km_page_alloc( BLOCK_SIZE );
    if( fs.buffer == NULL ){
      //TODO SEAN: free fs.fat
      //TODO SEAN: free fs.root_directory
      return -1;
    }

    // Mount status
    fs.mounted = false;

    // Error Handling
    fs.last_error = 0;

    __cio_puts("\nCREATED FS\n");

    // Return 0 on success
    return 0;
}

int read_block(int block_number, void *buffer){
  if (block_number < 0 || block_number >= fs.sectors_per_cluster) {
      __cio_puts("\nREAD_BLOCK FAILED\n");
        return -1; // Invalid block number
  }

  for(int i = 0; i < BLOCK_SIZE / SECTOR_SIZE; i++){ // 8 runs
    read_sector( i, buffer );
    __cio_printf("");
    __cio_write( fs.fat, 10 );
  }

  return 0; // Return 0 on success
}

int read_sector( int sector_number, void *buffer ){
  __memset(buffer, SECTOR_SIZE, 48);
  return 0; // Return 0 on success
}

int _fs_mount( void ) {
    // Implement FAT32 mounting logic here
    // Mount the filesystem using the provided filesystem structure
    return 0; // Return 0 on success
}

int _fs_read_file(const char *filename, void *buffer, uint32_t size, int32_t offset) {
    // Implement FAT32 file read logic here
    // Read data from the specified file into the buffer
    
    return -1; // Return -1 for now as this is a stub
}

int _fs_write_file(const char *filename, const void *buffer, uint32_t size, int32_t offset) {
    // Implement FAT32 file write logic here
    // Write data from the buffer to the specified file
    return -1; // Return -1 for now as this is a stub
}

// Implement additional filesystem operations as needed



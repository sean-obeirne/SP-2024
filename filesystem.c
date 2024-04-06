#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management


// Declare global variables for FileSystem and disk
static FileSystem fs;
static unsigned char disk_image[];

// Declare fs.buffer as a global variable
// static char fs.buffer[FS_BUFFER_SIZE];

// Function to initialize the fs.buffer
void init_fs_buffer( void ) {
    // Fill the fs.buffer with zeros
    __memset(fs.buffer, 0, FS_BUFFER_SIZE);
}

// Function to set the contents of the fs.buffer
void set_fs_buffer(const char *data, uint32_t size) {
    if (data == NULL || size > FS_BUFFER_SIZE) {
        return; // Invalid input, do nothing
    }
    // Copy data into the fs.buffer
    __memcpy(fs.buffer, data, size);
}

void clear_fs_buffer( void ) {
    // Fill the fs.buffer with zeros
    __memset(fs.buffer, 0, BLOCK_SIZE);
}

int dump_root(){
  __cio_printf("Dumping Root:\n");
  for(int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++){
    if( __strcmp(fs.root_directory[i].filename, "") != 0 ){
	    __delay( 20 );
      __cio_printf("file %d: filename:%s\n", i, fs.root_directory[i].filename);
    }
  }
	__delay( 100 );
  return 0;
}




int read_block(int block_number){
  if (block_number < 0 || block_number >= DISK_SIZE) {
        return -1; // Invalid block number
  }

  int start_index = block_number * BLOCK_SIZE;
  for(int i = 0; i < BLOCK_SIZE; i++){ // 8 runs
    ((char *)fs.buffer)[i] = disk_image[start_index + i];
    // __cio_printf("fs buffer: %s", (char *)fs.buffer);
  }

  return 0; // Return 0 on success
}

int write_block(int block_number) {
    // Check if block_number is valid
    if (block_number < 0 || block_number >= DISK_SIZE) {
        return -1; // Invalid block number
    }

    // Calculate the offset in bytes for the block
    // uint32_t byte_offset = block_number * BLOCK_SIZE + offset;

    // Write data to the disk image
    // for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
    //     disk_image[offset + i] = *((unsigned char *)data + i);
    // }

    // Write data to the disk image
    for (int i = 0; i < BLOCK_SIZE; i++) {
        // __cio_printf("We are assigning disk_image: %d", byte_offset);
        // disk_image[i] = *((unsigned char *)data + i);
        disk_image[i] = *((unsigned char *)fs.buffer + i);
    }

    return 0; // Write operation successful
}

int read_sector( int sector_number, void *buffer ){
  __memset(buffer, SECTOR_SIZE, 48);
  return 0; // Return 0 on success
}

int add_directory_entry(const char *filename) {
    // Search for an empty slot in the root directory
    int empty_slot_index = -1;
    for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
        if (fs.root_directory[i].filename[0] == '\0') {
            empty_slot_index = i;
            break;
        }
    }
    // If no empty slot found, return an error
    if (empty_slot_index == -1) {
        return -1; // Root directory is full
    }
    
    // Populate the empty slot with the new entry details
    __strcpy(fs.root_directory[empty_slot_index].filename, filename);
    fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
    // Assign cluster index based on the empty slot index
    fs.root_directory[empty_slot_index].block = empty_slot_index;

    return 0; // Success
}

int remove_directory_entry(const char *filename) {
    // Search for the directory entry with the given filename
    int entry_index = -1;
    for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
        if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
            entry_index = i;
            break;
        }
    }
    
    // If the entry is not found, return an error
    if (entry_index == -1) {
        return -1; // Directory entry not found
    }
    
    // Populate the empty slot with the new entry details
    __memset(fs.root_directory[entry_index].filename, MAX_FILENAME_LENGTH, '\0');
    fs.root_directory[entry_index].size = 0;
    fs.root_directory[entry_index].attributes = 0;
    fs.root_directory[entry_index].block = 0;
    // __strcpy(fs.root_directory[empty_slot_index].filename, filename);
    // fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
    // Assign cluster index based on the empty slot index
    // fs.root_directory[empty_slot_index].block = empty_slot_index;

    return 0; // Success
}


/* FILESYSTEM OPERATIONS */
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
    fs.fat = (FAT*)_km_page_alloc( fs.fat_size_sectors * SECTOR_SIZE );
    // __cio_printf("THIS VALUE: %d IS IT", fs.fat_size_sectors / fs.sectors_per_cluster);
    if( fs.fat == NULL ){
      return -1;
    }
    
    fs.fat_entry_size = 4;

    // Root Directory Information
    fs.root_directory_entries = 32;
    // fs.root_directory = _km_page_alloc( fs.fat_entry_size * fs.root_directory_entries );
    fs.root_directory = (DirectoryEntry *) _km_page_alloc( ((sizeof(DirectoryEntry) * ROOT_DIRECTORY_ENTRIES) / BLOCK_SIZE)); // TODO SEAN: do not allocate pages, but bytes

    if( fs.root_directory == NULL ){
      //TODO SEAN: free fs.fat
      return -1;
    }
      // __cio_puts("HELLOOOOO\n");
    
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
    // init_fs_buffer();
    // clear_fs_buffer();
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

    __cio_puts("CREATED FS\n");

    // Return 0 on success
    return 0;
}

int _fs_mount( void ) {
    // Implement FAT32 mounting logic here
    // Mount the filesystem using the provided filesystem structure
    return 0; // Return 0 on success
}

int _fs_create_file(const char *filename){
  return add_directory_entry(filename);
}

int _fs_rename_file(const char *old_filename, const char *new_filename){
  return -1;
}

int _fs_delete_file(const char *filename){
  return remove_directory_entry(filename);
}

int _fs_open_file(const char *filename, const char *mode){
  return -1;
}

int _fs_read_file(const char *filename) {
    // Implement FAT32 file read logic here
    // Read data from the specified file into fs.buffer
    // Traverse the FAT to find the file's data blocks

    // Search for the file in the root directory
    DirectoryEntry *entry = NULL;
    // find file from filename
    for(int i = 0; i < 4; i++){
      // _fs_dump_root();
      __cio_puts(fs.root_directory[i].filename);
      __cio_printf("Filename %s from root_directory, also %s found!\n", fs.root_directory[i].filename, filename);
      __cio_printf("root entry filename: %s\n", fs.root_directory[i].filename);
      if(__strcmp(fs.root_directory[i].filename, filename) == 0){
            entry = &fs.root_directory[i];
            __cio_printf("\n\nentry: %s", entry->filename);
            __strcpy(entry->filename, "test.txt");            // __cio_printf("Filename %s found!\n", entry);
            __cio_printf("\n\nentrynow: %s", entry->filename);
            break;
      }
      // __cio_printf("i in root directory entries: %d\n", i);

    }
    if (entry == NULL) {
        // File not found, handle error
        __cio_printf("WE ARE HERE IN NULL ENTRY\n");
        // __cio_printf("Reading... filename %s NOT found!\n", entry);
        // __cio_printf("Filename %s NOT found!\n", entry);
        return -1;
    }

  int start_block = entry->block;
  int current_block = start_block;
  while (current_block != FAT_EOC) {
      // Calculate the block number corresponding to the current cluster
      int block_number = current_block;
      
      // Read the block from the disk image into the buffer
      read_block(block_number);

      // Move buffer to the next position
      // fs.buffer += BLOCK_SIZE;

      // Move to the next cluster in the FAT
      current_block = fs.fat->entries[current_block].next_cluster;
  }
  return -1; // Return -1 for now as this is a stub
}

int _fs_write_file(const char *filename) {
    // Find the directory entry for the specified filename
    DirectoryEntry *entry = NULL;
    for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
        if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
            entry = &fs.root_directory[i];
            // __cio_printf("Filename %s found!\n", entry);
            break;
        }
    }

    // If the file doesn't exist, return an error
    if (entry == NULL) {
        __cio_printf("Writing... filename %s NOT found!\n", entry);
        add_directory_entry(filename);
        return -1; // File not found
    }

    // Retrieve the starting cluster of the file
    uint32_t current_block = entry->block;
    // current_block = 0;

    // Write data blocks into the file
    const uint8_t *data_ptr = (const uint8_t *)fs.buffer;
    while (current_block != FAT_EOC) {
        // Calculate the block number corresponding to the current cluster
        int block_number = current_block;

        // Write data from the buffer to the disk image
        write_block(block_number);
        // __cio_printf("Write block called on block #%d: ", block_number);
        read_block(block_number);
        // __cio_printf("Read block called on block #%d: ", block_number);
        // __cio_puts("WRITING\n");
        // __cio_puts((char *)fs.buffer);
        // __cio_puts("\n");
        // Move to the next cluster in the FAT
        uint32_t next_block = fs.fat->entries[current_block].next_cluster;
        if (next_block != FAT_EOC) {
            fs.fat->entries[current_block].next_cluster = FAT_EOC;
            current_block = next_block;
        } else {
            break; // Already at the end of the file
        }

        // Move buffer to the next position
        data_ptr += BLOCK_SIZE;
    }

    return 0; // Write operation successful
}

int _fs_close_file(const char *filename){
  return -1;
}

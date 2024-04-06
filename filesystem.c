#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management


// Declare global variables for FileSystem and disk
static FileSystem fs;
// static unsigned char disk_image[DISK_SIZE];
static unsigned char disk_image[BLOCK_SIZE * DISK_SIZE];


// Function to initialize the fs.buffer
void init_fs_buffer( void ) {
    // Fill the fs.buffer with zeros
    __memset(fs.buffer, 0, BLOCK_SIZE);
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

int dump_fs_buffer( void ){
  __cio_printf("Dumping Buffer:\n");
  __cio_printf("%s\n", (char *)fs.buffer);
  return 0;
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

int dump_disk_image( int block_number ){
  __cio_puts("Dumping Disk Image...\n");
  __cio_printf("%s\n", disk_image);

  char finished_string[BLOCK_SIZE];
  int finished_i = 0;
  int num_succ = 0;
  int num_fail = 0;
  for(int i = block_number * BLOCK_SIZE; i < (block_number+1) * BLOCK_SIZE; i++){
    if (disk_image[i] != 0) {
      finished_string[finished_i] = disk_image[i];
      // __memcpy(finished_string, disk_image, 1);
      // __cio_printf("%s", finished_string);
      finished_i += 1;
      num_succ += 1;
    } else{
      num_fail += 1;
    }
    // __delay(10);disk_image
  }
  __cio_printf("Number of succeeded reads: %d\n", num_succ);
  __cio_printf("Accumulated string: %s\n", finished_string);
  __cio_printf("Number of failed reads: %d\n", num_fail);
  return 0;
}

void wipe_disk( void ){
    __memset(disk_image, 0, BLOCK_SIZE * DISK_SIZE);
    // Set all elements of the disk_image array to 0
    // for (int i = 0; i < BLOCK_SIZE * DISK_SIZE; i++) {
    //     disk_image[i] = 0;
    // }
    // return 0;
}

int read_block(int block_number){
  if (block_number < 0 || block_number >= DISK_SIZE) {
        return -1; // Invalid block number
  }

  int start_index = block_number * BLOCK_SIZE;
  // Copy data from disk image to file system buffer
  __memcpy(fs.buffer, &disk_image[start_index], BLOCK_SIZE);
  // __cio_printf("fs buffer: %s\n", (char *)fs.buffer);

  return 0; // Return 0 on success
}

int write_block(int block_number, const uint8_t *data_ptr ) {
    // Check if block_number is valid
    if (block_number < 0 || block_number >= DISK_SIZE) {
        return -1; // Invalid block number
    }
    // Check if data_ptr is not NULL
    if (data_ptr == NULL) {
        return -1; // Invalid data pointer
    }

    // Calculate the offset in bytes for the block
    // uint32_t byte_offset = block_number * BLOCK_SIZE + offset;

    // Write data to the disk image
    // for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
    //     disk_image[offset + i] = *((unsigned char *)data + i);
    // }
    int start_index = block_number * BLOCK_SIZE;
    // Write data to the disk image
    for (int i = 0; i < BLOCK_SIZE; i++) {
        // __cio_printf("We are assigning disk_image: %d", byte_offset);
        // disk_image[i] = *((unsigned char *)data + i);
        disk_image[start_index + i] = data_ptr[i];
        if (data_ptr[i] == '\0'){
          break;
        }
        __cio_printf("NEXT CHAR: %c", disk_image[start_index + i]);
        __cio_printf(" VS: %c\n", data_ptr[i]);
        // __delay(10);
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
        __cio_puts("ROOT DIRECTORY FULL\n");
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

    // Set all FAT entries to FAT_EOC
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        fs.fat->entries[i].next_cluster = FAT_EOC;
    }

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

    // wipe_disk();

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
  int add_entry = add_directory_entry(filename);
  if(add_entry == -1){
    __cio_printf("FAILED TO CREATE FILE");
    return -1;
  }
  return 0;
}

int _fs_rename_file(const char *old_filename, const char *new_filename){
  return -1;
}

int _fs_delete_file(const char *filename){
  return remove_directory_entry(filename);
}

DirectoryEntry *_fs_find_file(const char *filename) {
    // Search for the file in the root directory
    DirectoryEntry *entry = NULL;
    for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
        if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
            entry = &fs.root_directory[i];
            break;
        }
    }
    __cio_printf("Entry for file %s found, size = %d, block = %d\n", entry->filename, entry->size, entry->block);

    return entry;
}

int _fs_open_file(const char *filename, const char *mode){
  if (mode == NULL || (mode[0] != 'r' && mode[0] != 'w')) {
      // Invalid mode
      return -1;
  }

  DirectoryEntry *entry = _fs_find_file(filename);
  if (entry == NULL) {
      // File not found
      return -1;
  }
  // uint32_t size = entry->size;

  return 0;
}

int _fs_read_file(const char *filename) {
    // Read data from the specified file into fs.buffer
    
    __cio_puts("READING... ");

    // Search for the file in the root directory
    DirectoryEntry *entry = _fs_find_file(filename);
    if (entry == NULL) {
        __cio_printf("filename %s NOT found!\n", filename);
        return -1;
    }

    // Traverse the FAT to find the file's data blocks
  int start_block = entry->block;
  int current_block = start_block;
  while (current_block != FAT_EOC) {
      // Calculate the block number corresponding to the current cluster
      
      // Read the block from the disk image into the buffer
      read_block(current_block);

      // Move buffer to the next position
      // fs.buffer += BLOCK_SIZE;

      // Move to the next cluster in the FAT
      current_block = fs.fat->entries[current_block].next_cluster;
  }
  __cio_printf("Read complete\n");
  dump_fs_buffer();
  return 0; // Return -1 for now as this is a stub
}

int _fs_write_file(const char *filename, const void *data) {
    // Find the directory entry for the specified filename
    DirectoryEntry *entry = _fs_find_file(filename);

    // If the file doesn't exist, return an error
    if (entry == NULL) {
        __cio_printf("Writing... filename %s NOT found! Creating...\n", entry);
        __delay( 200 );
        _fs_create_file(entry->filename);
        // return -1; // File not found
    }

    // Retrieve the starting cluster of the file
    uint32_t start_block = entry->block;
    uint32_t current_block = start_block; 
    // current_block = 0;

    // Write data blocks into the file
    const uint8_t *data_ptr = (const uint8_t *)data;
    while (current_block != FAT_EOC) {
        // Calculate the block number corresponding to the current cluster
        int block_number = current_block;

        // Write data from the buffer to the disk image
        // write_block(block_number, data_ptr); 
        write_block(1, data_ptr); //TODO SEAN: this only works with specific block numbers
        // __cio_printf("Write block called on block #%d: ", block_number);
        // read_block(block_number);
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
  return 0;
}

int _fs_print_file(const char *filename){
  
  return 0;
}
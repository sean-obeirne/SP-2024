#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management


// Declare global variables for FileSystem and disk
static FileSystem fs;
static char *cwd = "/"; // init cwd as root
// static unsigned char disk_image[DISK_SIZE];
// static unsigned char disk_image[BLOCK_SIZE * DISK_SIZE];
static unsigned char disk_image[TOTAL_SIZE];


Directory *open_dir(const char *path) {
    // Implement logic to open a directory
    return NULL;
}

int close_dir(Directory *dir) {
    // Implement logic to close a directory
    return 0;
}

int list_dir_contents(const char *path) {
	__cio_puts("Listing...\n");
	DirectoryEntry *entry = _fs_find_entry(path);
	Directory *dir = (Directory *)entry->subdirectory;
	__cio_printf("Contents of %s:\n", entry->filename);
	if (entry->type == 1){ // file
		__cio_printf("nevermind... %s is a file\n", entry->filename);
		_fs_print_entry(path);
	}
	else{
		// Print directory contents
		for (uint32_t i = 0; i < dir->num_files; i++) {
			// Print filename
			__cio_printf("%s is a dir\n", entry->filename);
			__cio_puts(dir->files[i].filename);
			__cio_puts("\n");
		}

		return 0; // Success
	}
    return NULL;
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////HELPER FUNCTIONS/////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

void init_fs_buffer( void ) {
	fs.buffer = (char *)_km_page_alloc(1);
}

void clear_fs_buffer( void ) {
	// Fill the fs.buffer with zeros
	__memclr(fs.buffer, BLOCK_SIZE);
}

int dump_fs_buffer( void ){
	__cio_printf("Dumping Buffer:\n");
	__cio_printf("%s\n", (char *)fs.buffer);
	__delay(20);
	return 0;
}

int dump_root( void ){
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

char *dump_disk_image_block( int block_number ){
	// __cio_puts("Dumping Disk Image Block...\n");
	// __cio_printf("%s\n", disk_image);

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
	// __cio_printf("Number of succeeded reads: %d\n", num_succ);
	// __cio_printf("Accumulated string: %s\n", finished_string);
	// __cio_printf("Number of failed reads: %d\n", num_fail);
	return 0;
}

char *dump_disk_image( void ){
	// __cio_puts("Dumping Disk Image Block...\n");
	// __cio_printf("%s\n", disk_image);

	char *finished_string = (char *)_km_page_alloc(DISK_SIZE);
	if (finished_string == NULL) {
		return NULL;
	}
	// Zero out the allocated memory
	__memset(finished_string, 0, TOTAL_SIZE + 1);
	// __cio_puts(finished_string);


	int finished_i = 0;
	int num_succ = 0;
	int num_fail = 0;
	for(int i = 0 * BLOCK_SIZE; i < TOTAL_SIZE; i++){
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
	// __cio_puts("HELLO?????");
	// __cio_puts(finished_string);
	finished_string[finished_i] = '\0';
	// __cio_printf("Number of succeeded reads: %d\n", num_succ);
	// __cio_printf("Accumulated string: %s\n", finished_string);
	// __cio_printf("Number of failed reads: %d\n", num_fail);
	return finished_string;
	/*
	// __cio_puts("Dumping Disk Image...\n");
	// __cio_printf("%s\n", disk_image);
	for(int i = 0; i < TOTAL_SIZE; i++){
	dump_disk_image_block(i);
	}
	return 0;*/
}

void wipe_disk( void ){
	// __memset(disk_image, 0, TOTAL_SIZE);
	__memclr(disk_image, TOTAL_SIZE);
	// Set all elements of the disk_image array to 0
	// for (int i = 0; i < BLOCK_SIZE * DISK_SIZE; i++) {
	//     disk_image[i] = 0;
	// }
	// return 0;
}

// read block block_number to fs.buffer
int read_block(int block_number){
	if (block_number < 0 || block_number >= DISK_SIZE) {
		return -1; // Invalid block number
	}

	int start_index = block_number * BLOCK_SIZE;
	// Copy data from disk image to file system buffer
	__memcpy(fs.buffer, &disk_image[start_index], BLOCK_SIZE);
	// __cio_printf("fs buffer: %s\n", (char *)fs.buffer);

	// this block is now in fs.buffer, use appropriately
	// dump_fs_buffer();

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
		// __cio_printf("NEXT CHAR: %c", disk_image[start_index + i]);
		// __cio_printf(" VS: %c\n", data_ptr[i]);
		// __delay(10);
	}

	return 0; // Write operation successful
}

int read_sector( int sector_number, void *buffer ){
	__memset(buffer, SECTOR_SIZE, 48);
	return 0; // Return 0 on success
}

// Function to generate a file larger than 4096 bytes
void generate_large_file(int num_blocks) {
	char output_char = 65;
	// Write blocks of data to the file
	for (uint32_t i = 0; i < num_blocks; i++) { // num blocks
		for (uint32_t j = 0; j < BLOCK_SIZE; j++) {
			disk_image[i * BLOCK_SIZE + j] = output_char; // Write arbitrary data (e.g., 'A') to simulate file content
		}
		output_char += 1;
	}

	// Write remaining bytes if FILE_SIZE is not a multiple of BLOCK_SIZE
	uint32_t remaining_bytes = 0;
	for (uint32_t i = 0; i < remaining_bytes; i++) {
		disk_image[(num_blocks) * BLOCK_SIZE + i] = 'C'; // Write arbitrary data (e.g., 'A') to simulate file content
	}
}

// Function to parse path into directory names and file name
void parse_path(const char *path, char **dir_names, char *file_name, int *num_dirs) {
    // Initialize file name
    file_name[0] = '\0';
	*num_dirs = 0;
	__cio_printf("NUM DIRS: %d\n", *num_dirs);

    // Check if path is empty or null
    if (path == NULL || *path == '\0') {
        return;
    }

    // Find the last occurrence of directory separator ("/" or "\")
    const char *last_separator = NULL;
    const char *current_char = path;
    while (*current_char != '\0') {
        if (*current_char == '/' || *current_char == '\\') {
            last_separator = current_char;
        }
        current_char++;
    }

    // If no separator found, the path is just a filename
    if (last_separator == NULL) {
        while (*path != '\0') {
            *file_name = *path;
            file_name++;
            path++;
        }
        *file_name = '\0'; // Null-terminate the file name
        return;
    }

    // Extract directory names and file name
    // Initialize directory count
    int dir_count = 0;

    // Copy characters before the last separator into directory names
    const char *start_dir = path;
    const char *end_dir = last_separator;
    while (start_dir < end_dir) {
        // Find the next directory separator
        const char *next_separator = start_dir;
        while (*next_separator != '/' && *next_separator != '\\' && *next_separator != '\0') {
            next_separator++;
        }

        // Calculate the length of the current directory name
        uint32_t dir_length = next_separator - start_dir;

        // Allocate memory for the directory name
		// __cio_printf("dir length: %d\n", dir_length + 1);
        dir_names[dir_count] = (char *)_km_page_alloc((dir_length + 1));
        if (dir_names[dir_count] == NULL) {
            // Memory allocation failed, return
			__cio_printf("Failed to allocate memory for directories in path %s\n", path);
            return;
        }
		// Allocate memory for the directory name
		int dir_name_length = next_separator - start_dir;  // Calculate the length of the directory name
		char *dir_name_ptr = (char *)_km_page_alloc((dir_name_length + 1) * sizeof(char));  // Allocate memory (+1 for null terminator)
		if (dir_name_ptr == NULL) {
			__cio_printf("Failed to allocate memory for dir_name_ptr %s\n", *dir_name_ptr);
			return;
		}

        // Copy characters to the directory name
		const char *dir_ptr = start_dir;
		char *current_dir_char = dir_name_ptr;
		while (dir_ptr < next_separator) {
			*current_dir_char = *dir_ptr;
			current_dir_char++;
			dir_ptr++;
		}
		*current_dir_char = '\0';  // Null-terminate the directory name

		// Save the directory name pointer to the directory double pointer
		dir_names[dir_count] = dir_name_ptr;
		// __cio_printf("DIR NAMES: %s\n", dir_names[num_dirs]);

        // Move to the next directory name
        dir_count++;
		*num_dirs += 1;
        start_dir = next_separator + 1;
    }


    // Copy characters after the last separator into file name
    const char *file_ptr = last_separator + 1;
    while (*file_ptr != '\0') {
        *file_name = *file_ptr;
        file_name++;
        file_ptr++;
    }
    *file_name = '\0'; // Null-terminate the file name
}

int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert){
	if(dest == NULL || insert == NULL){
		__cio_printf("Invalid parameters to add_sub_entry()\n");
		return -1;
	}
	if(dest->type != 2){
		// __cio_printf("%s\n", dest->subdirectory->files[0].filename);
		__cio_printf("Adding DirectoryEntry %s to a file, %s, aborting...\n", dest->filename, insert->filename);
		return -1;
	}
	__cio_puts("Adding sub entry...");
	dest->subdirectory->files[0] = *insert;
	dest->subdirectory->num_files += 1;
	__cio_printf("subdir created, %s now contains %s\n", dest->filename, dest->subdirectory->files[0].filename);
	return 0;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////FILE SYSTEM//////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

/* FILESYSTEM OPERATIONS */
int _fs_init( void ) {
	__cio_puts("\nFilesystem Implementation:\n");
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
	DirectoryEntry root_entry;
    __strcpy(root_entry.filename, "/");
    root_entry.size = 0;
    root_entry.type = DIRECTORY_ATTRIBUTE;
    root_entry.block = 0;
	fs.root_directory[0] = root_entry;

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
	init_fs_buffer();
	if( fs.buffer == NULL ){
		//TODO SEAN: free fs.fat
		//TODO SEAN: free fs.root_directory
		return -1;
	}
	clear_fs_buffer();

	// Mount status
	fs.mounted = false;

	// Error Handling
	fs.last_error = 0;

	wipe_disk();

	// TEST A BIG FILE
	// __delay(20);
	// generate_large_file(4);
	// __delay(20);
	// read_block(0);
	// __delay(20);
	// read_block(1);
	// __delay(20);
	// read_block(2);
	// __delay(20);
	// read_block(3);
	// __delay(20);

	__cio_puts("CREATED FS\n");

	// Return 0 on success
	return 0;
}

int _fs_mount( void ) {
	// Implement FAT32 mounting logic here
	// Mount the filesystem using the provided filesystem structure
	return 0; // Return 0 on success
}

DirectoryEntry *_fs_find_entry(const char *filename) {
	// Search for the file in the root directory
	DirectoryEntry *entry = NULL;
	for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
		if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
			entry = &fs.root_directory[i];
			break;
		}
	}
	// __cio_printf("Entry for file %s found, size = %d, block = %d\n", entry->filename, entry->size, entry->block);

	return entry;
}

int _fs_create_entry_from_path(const char *path, EntryType type){
	__cio_puts("Creating (from path)...\n");


	char *dirs[MAX_PATH_LENGTH]; // Double pointer to store directory names
	char filename[MAX_FILENAME_LENGTH + 1]; // Buffer to store file name
	int num_dirs = 0;

	parse_path(path, dirs, filename, &num_dirs);
	// __cio_puts("PATH, DIR_NAME, FILE_NAME:\n");
	// __cio_puts(path);
	// __cio_puts("\n");
	// __cio_puts(dirs[2]);
	// __cio_puts("\n");
	// __cio_puts(filename);
	// __cio_puts("\n");

	DirectoryEntry *parent_dir_entry = &fs.root_directory[0];

	for(int i = 1; i < num_dirs; i++){
		// Check if the directory already exists
		DirectoryEntry *existing_dir_entry = _fs_find_entry(dirs[i]);

		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (existing_dir_entry == NULL) {
			if(_fs_create_entry(parent_dir_entry->filename, DIRECTORY_ENTRY)){
				existing_dir_entry = _fs_find_entry(parent_dir_entry->filename);
			}
			__cio_printf("Ok but here is the filename: %s\n", parent_dir_entry->filename);
			if (existing_dir_entry == NULL) {
				// Failed to create the directory entry
				__cio_printf("Failed to create parent dir %s\n", *existing_dir_entry);
				return -1;
			}
		}

		// add_sub_entry(_fs_find_entry("subdir"), _fs_find_entry("file.txt"));
		// if (parent_dir_entry->type == 2){
		// 	add_sub_entry(parent_dir_entry, existing_dir_entry);
		// }

		// Update the parent directory entry to the current directory entry
		parent_dir_entry = existing_dir_entry;
		if(_fs_find_entry(dirs[i]) != NULL)
			continue; // directory exists
		else{
			_fs_create_entry(dirs[i], DIRECTORY_ENTRY);
		}
	}

	// No duplicate filenames
	// if(_fs_find_entry(filename) != NULL){
	// 	__cio_printf("File %s already exists\n", filename);
	// 	return -1;
	// }

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
		return -1;
	}

	// Populate the empty slot with the new entry details
	DirectoryEntry *new_entry = &fs.root_directory[empty_slot_index];
	__strcpy(new_entry->filename, filename);
	new_entry->size = 0;
	new_entry->type = (type == FILE_ENTRY) ? FILE_ATTRIBUTE : DIRECTORY_ATTRIBUTE;
	new_entry->block = empty_slot_index; // Allocate a block for the new entry's data


	// Populate the empty slot with the new entry details
	// __strcpy(fs.root_directory[empty_slot_index].filename, filename);
	// fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
	// fs.root_directory[empty_slot_index].block = empty_slot_index;
	__cio_printf("Successfully created DirectoryEntry %s, a %s created at block %d\n", fs.root_directory[empty_slot_index].filename, fs.root_directory[empty_slot_index].type == 1 ? "file" : "dir", fs.root_directory[empty_slot_index].block);
	return 0;
}

int _fs_create_entry(const char *filename, EntryType type){
	__cio_puts("Creating...\n");
	// No duplicate filenames
	if(_fs_find_entry(filename) != NULL){
		__cio_printf("File %s already exists\n", filename);
		return -1;
	}

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
		return -1;
	}

	// Populate the empty slot with the new entry details
    DirectoryEntry *new_entry = &fs.root_directory[empty_slot_index];
    __strcpy(new_entry->filename, filename);
    new_entry->size = 0;
    new_entry->type = (type == FILE_ENTRY) ? FILE_ATTRIBUTE : DIRECTORY_ATTRIBUTE;
    new_entry->block = empty_slot_index; // Allocate a block for the new entry's data
	if (type == DIRECTORY_ENTRY){
		Directory *subdirectory = (Directory *)_km_page_alloc(1);
		if (subdirectory == NULL) {
			// Handle error if memory allocation fails
			__cio_printf("failed to create subdirectory\n");
			return -1;
		}
		new_entry->subdirectory = subdirectory;
	}



	// Populate the empty slot with the new entry details
	// __strcpy(fs.root_directory[empty_slot_index].filename, filename);
	// fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
	// fs.root_directory[empty_slot_index].block = empty_slot_index;
	__cio_printf("Successfully created DirectoryEntry %s, a %s created at block %d\n", fs.root_directory[empty_slot_index].filename, fs.root_directory[empty_slot_index].type == 1 ? "file" : "dir", fs.root_directory[empty_slot_index].block);
	return 0;
}

int _fs_rename_entry(const char *old_filename, const char *new_filename){
	__cio_puts("Renaming...\n");
	DirectoryEntry *src = _fs_find_entry(old_filename);
	DirectoryEntry *dest = _fs_find_entry(new_filename);
	if(dest != NULL){
		__cio_printf("File %s already exists\n", new_filename);
		return -1;
	}
	__memcpy(src->filename, new_filename, MAX_FILENAME_LENGTH);
	return 0;
}

int _fs_delete_entry(const char *filename){
	__cio_puts("Deleting...\n");
	DirectoryEntry *entry = _fs_find_entry(filename);
	if(entry == NULL){
		__cio_printf("File %s does not exist", filename);
		return -1;
	}

	__memclr(entry, sizeof(DirectoryEntry));

	return 0; // Success
}

int _fs_open_file(const char *filename, const char *mode){
	if (mode == NULL || (mode[0] != 'r' && mode[0] != 'w')) {
		// Invalid mode
		return -1;
	}

	DirectoryEntry *entry = _fs_find_entry(filename);
	if (entry == NULL) {
		// File not found
		return -1;
	}
	// uint32_t size = entry->size;

	return 0;
}

int _fs_read_file(const char *filename) {
	__cio_puts("Reading...\n");
	// Read data from the specified file into fs.buffer

	// __cio_puts("READING... \n");

	// Search for the file in the root directory
	DirectoryEntry *entry = _fs_find_entry(filename);
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
		__cio_printf("reading block %d\n", current_block);
		__delay(20);
		read_block(current_block);
		dump_fs_buffer();
		clear_fs_buffer();

		// Move buffer to the next position
		// fs.buffer += BLOCK_SIZE;

		// Move to the next cluster in the FAT
		current_block = fs.fat->entries[current_block].next_cluster;
	}
	// __cio_printf("Read complete\n");
	// dump_fs_buffer();
	return 0; // Return -1 for now as this is a stub
}

int _fs_write_file(const char *filename, const void *data) {
	__cio_puts("Writing...\n");

	// Find the directory entry for the specified filename
	DirectoryEntry *entry = _fs_find_entry(filename);

	// If the file doesn't exist, return an error
	if (entry == NULL) {
		// __cio_printf("Filename %s NOT found! Creating...\n", entry);
		// __delay( 200 );
		// _fs_create_entry(entry->filename, FILE_ENTRY);
		__cio_printf("Filename %s NOT found!\n", entry);
		return -1;
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
		__cio_printf("Writing to block number %d\n", block_number);
		// __delay( 200);

		// Write data from the buffer to the disk image
		// write_block(block_number, data_ptr); 
		write_block(block_number, data_ptr); //TODO SEAN: this only works with specific block numbers
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

int _fs_print_entry(const char *filename){
	__cio_puts("Printing...\n");
	DirectoryEntry *entry = _fs_find_entry(filename);
	if(entry == NULL){
		__cio_printf("File \"%s\" not found\n");
		return -1;
	}
	__cio_printf("This directory entry has filename %s, size %d, at block %d\nDumping buffer...\n", entry->filename, entry->size, entry->block);
	read_block(entry->block);
	__cio_printf("%s", fs.buffer);
	return 0;
}

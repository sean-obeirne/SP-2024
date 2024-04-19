#include "filesystem.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management
#include "ramdisk.h" // Include memory management


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
	__delay(20);
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

void dump_fat( void ) {
    // Iterate over each entry in the FAT table
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Print the details of each FAT entry
        __cio_printf("FAT Entry %d: Next Cluster = %d, Status = %d\n", i, fs.fat->entries[i].next_cluster, fs.fat->entries[i].status);
		__delay(5);
    }
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
	
	// __cio_puts("Dumping Disk Image...\n");
	// __cio_printf("%s\n", disk_image);
	for(int i = 0; i < TOTAL_SIZE; i++){
		dump_disk_image_block(i);
	}
	return 0;
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

/*
// Function to parse a path into directory names and a file name
void parse_path(const char *path, char **dir_names, char *file_name, int *num_dirs) {
    // Check if the path is empty or NULL
    if (path == NULL || *path == '\0') {
        *num_dirs = 0;
        return;
    }

    *num_dirs = 0;
    int dir_len = 0;
    int index = 0;
    int file_index = -1;
    int num_slashes = 0;

    // Handle root directory case
    if (path[0] == '/') {
        index = 1;
        num_slashes++;
    }

    // Iterate through the path
    while (path[index] != '\0') {
        // Check for directory separator
        if (path[index] == '/') {
            // Increment number of slashes encountered
            num_slashes++;
            if (dir_len > 0) {
                // Allocate memory for directory name and copy it
                char *dir_name = dir_names[*num_dirs];
                while (dir_name != NULL && *dir_name != '\0') {
                    dir_name++;
                }
                *dir_name = '\0';
                (*num_dirs)++;
                dir_len = 0;
            }
        } else {
            if (num_slashes == 1 && file_index == -1) {
                file_index = index;
            }
            dir_names[*num_dirs][dir_len] = path[index];
            dir_len++;
        }
        index++;
    }

    // Handle the last directory or file
    if (dir_len > 0) {
        char *dir_name = dir_names[*num_dirs];
        while (dir_name != NULL && *dir_name != '\0') {
            dir_name++;
        }
        *dir_name = '\0';
        (*num_dirs)++;
    }

    // If there is a file name
    if (file_index != -1) {
        int i = 0;
        while (path[file_index] != '\0') {
            file_name[i] = path[file_index];
            i++;
            file_index++;
        }
        file_name[i] = '\0';
    }
}
*/
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

	dir_names[0] = "/\0";


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

int add_fat_entry(uint32_t next_cluster) {
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Check if the current entry is free
        if (fs.fat->entries[i].status == FAT_FREE) {
            // Set the next_cluster field to the provided value
            fs.fat->entries[i].next_cluster = next_cluster;
            
            // Update the status field to indicate that the entry is in use
            fs.fat->entries[i].status = FAT_IN_USE;
            
            // Return the index of the added entry
            return i;
        }
    }
    
    // No free entry found
    return -1;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//////////////////////FILE SYSTEM//////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

/* FILESYSTEM OPERATIONS */
int _fs_init( void ) {
	__cio_puts("\nFilesystem Implementation:\n");
	// Implement FAT32 initialization logic here
	// Initialize the filesystem structure and perform any necessary setup

	// Initialize the file system struct with appropriate values
	fs.bytes_per_sector = SECTOR_SIZE;
	fs.sectors_per_cluster = 8;
	fs.reserved_sector_count = 1;
	fs.number_of_fats = 1;
	fs.total_sectors = 2880;
	fs.fat_size_sectors = 16; // fat size in sectors
	fs.root_directory_cluster = 2; // root is at cluster _
				       //Q? do clusters=blocks=pages? and do sectors=segments?

	// Initialize storage interface with RAM disk backend
    StorageInterface disk;
    int result = storage_init(&disk, DISK_SIZE);
    if (result < 0) {
        __cio_printf("Error initializing storage\n");
        return -1;
    }
	fs.disk = disk;

	// Request free space on the RAM disk for the FAT
    fs.fat = (FAT *)disk.request_space(fs.fat_size_sectors * fs.bytes_per_sector);
    if (fs.fat < 0) {
        __cio_printf("Error requesting free space for FAT\n");
        return -1;
    }

	// Initialize FAT
	for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Set the next_cluster field to FAT_EOC to mark the cluster as end-of-chain
        fs.fat->entries[i].next_cluster = FAT_EOC;
        // Set the status field to indicate that the cluster is free
        fs.fat->entries[i].status = FAT_FREE;
    }

	// dump_fat();
	// add_fat_entry(69);
	// dump_fat();

	// __cio_printf("EXPECTED: fs.fat: %d to %d\n", fs.fat, fs.fat + 8192);
	__cio_printf("fs.fat: %d to %d\n", (int)fs.fat, (int)((char*)fs.fat + (fs.fat_size_sectors * fs.bytes_per_sector)));
	// __delay(100);
	fs.fat_entry_size = 4;

	// Root Directory Information
	fs.root_directory_entries = 32;
	fs.root_directory = (DirectoryEntry *) disk.request_space(sizeof(DirectoryEntry) * ROOT_DIRECTORY_ENTRIES); // TODO SEAN: do not allocate pages, but bytes

	if( fs.root_directory == NULL ){
		//TODO SEAN: free fs.fat
		return -1;
	}
	DirectoryEntry root_entry;
    __strcpy(root_entry.filename, "/");
    root_entry.size = 0;
    root_entry.type = DIRECTORY_ATTRIBUTE;
    root_entry.cluster = 0;
	fs.root_directory[0] = root_entry;
	add_fat_entry(0);

	// dump_fat();

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

DirectoryEntry *_fs_find_entry_from_path(const char *path) {

	char *dirs[MAX_PATH_LENGTH]; // Double pointer to store directory names
	for (int i = 0; i < MAX_PATH_LENGTH; i++) {
		dirs[i] = (char *)_km_page_alloc(1); // Assuming _km_alloc allocates memory
	}

	char filename[MAX_FILENAME_LENGTH + 1]; // Buffer to store file name
	int num_dirs = 0;

	parse_path(path, dirs, filename, &num_dirs);
	__cio_printf("Path: %s\n", path);
	__cio_printf("dirs[0]: %s\n", dirs[0]);
	__cio_printf("dirs[1]: %s\n", dirs[1]);
	__cio_printf("dirs[2]: %s\n", dirs[2]);
	// __cio_printf("dirs[3]: %s\n", dirs[3]);
	// __cio_printf("dirs: %s\n", dirs);
	__cio_printf("filename: %s\n", filename);
	__cio_printf("num_dirs: %d\n", num_dirs);
	__cio_printf("  Path: %s\n", path);

	DirectoryEntry *curr_parent_dir_entry = &fs.root_directory[0];
	DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[0]);

	__cio_printf("======PAY ATTENTION======\n");
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	__delay(2000);

	// __cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	// __cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry, curr_parent_dir_entry->filename);
	// __delay(50);
	return NULL;
	for(int i = 1; i < num_dirs; i++){

		// Check if the directory already exists
		DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);

		__cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry->filename, curr_parent_dir_entry->filename);
		__delay(50);
		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (curr_dir_entry == NULL) {
			__cio_printf("Failed to find entry for directory %s", dirs[i]);
			return -1;
		}

		curr_parent_dir_entry = curr_dir_entry;
	}


	// Search for the file in the root directory
	DirectoryEntry *entry = NULL;
	for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
		if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
			entry = &fs.root_directory[i];
			break;
		}
	}
	// __cio_printf("Entry for file %s found, size = %d, block = %d\n", entry->filename, entry->size, entry->cluster);

	return entry;
}

DirectoryEntry *_fs_find_entry(const char *filename) {
	// Search for the file in the root directory
	DirectoryEntry *entry = NULL;
	for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
		if (__strcmp(fs.root_directory[i].filename, filename) == 0) {
			entry = &fs.root_directory[i];
			__cio_printf("ENTRY FOUND: %s", entry->filename);
			break;
		}
	}
	// __cio_printf("Entry for file %s found, size = %d, block = %d\n", entry->filename, entry->size, entry->cluster);

	return entry;
}

int _fs_create_entry_from_path(const char *path, EntryType type){
	__cio_puts("Creating (from path)...\n");


	char *dirs[MAX_PATH_LENGTH]; // Double pointer to store directory names
	for (int i = 0; i < MAX_PATH_LENGTH; i++) {
		dirs[i] = (char *)_km_page_alloc(1); // Assuming _km_alloc allocates memory
	}
	char filename[MAX_FILENAME_LENGTH + 1]; // Buffer to store file name
	int num_dirs = 0;

	// parse_path(path, dirs, filename, &num_dirs);
	// __cio_puts("PATH, DIR_NAME, FILE_NAME:\n");
	// __cio_puts(path);
	// __cio_puts("\n");
	// __cio_puts(dirs[2]);
	// __cio_puts("\n");
	// __cio_puts(filename);
	// __cio_puts("\n");

	parse_path(path, dirs, filename, &num_dirs);
	__cio_printf("Path: %s\n", path);
	__cio_printf("dirs[0]: %s\n", dirs[0]);
	__cio_printf("dirs[1]: %s\n", dirs[1]);
	__cio_printf("dirs[2]: %s\n", dirs[2]);
	// __cio_printf("dirs[3]: %s\n", dirs[3]);
	// __cio_printf("dirs: %s\n", dirs);
	__cio_printf("filename: %s\n", filename);
	__cio_printf("num_dirs: %d\n", num_dirs);


	DirectoryEntry *curr_parent_dir_entry = &fs.root_directory[0];

	// Check if the directory already exists
	// DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[1]);
	// DirectoryEntry *curr_dir_entry = _fs_find_entry("dir");

	// __cio_printf("======PAY ATTENTION1======\n");
	// __cio_printf("  Path: %s\n", path);
	// __cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	// __delay(20);

	for(int i = 1; i < num_dirs; i++){

		// Check if the directory already exists
		DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);

		__cio_printf("======PAY ATTENTION======\n");
		__cio_printf("  Path: %s\n", path);
		__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
		__delay(20);
		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (curr_dir_entry == NULL) {
			if(_fs_create_entry(dirs[i], DIRECTORY_ENTRY) != 0){
				__cio_printf("Failed to create entry for directory %s\n", dirs[i]);
				return -1;
			} else{
				__cio_printf("CREATED DirectoryEntry %s\n", dirs[i]);
			}
		}

		if (add_sub_entry(curr_parent_dir_entry, curr_dir_entry) < 0){
			// __cio_printf("Failed to add entry to parent directory %s\n", dirs[i]);
			__cio_printf("Failed to add entry to parent directory %s\n", curr_parent_dir_entry);
			return -1;
		}

		// add_sub_entry(_fs_find_entry("subdir"), _fs_find_entry("file.txt"));
		// if (parent_dir_entry->type == 2){
		// 	add_sub_entry(parent_dir_entry, existing_dir_entry);
		// }

		// Update the parent directory entry to the current directory entry
		// if(_fs_find_entry(curr_parent_dir_entry) == NULL){
		// 	// __cio_printf("PROBLEM WITH %s", dirs[i]->fi);
		// 	_fs_create_entry(dirs[i], DIRECTORY_ENTRY);
		// }
		curr_parent_dir_entry = curr_dir_entry;
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
	new_entry->cluster = empty_slot_index; // Allocate a block for the new entry's data


	// Populate the empty slot with the new entry details
	// __strcpy(fs.root_directory[empty_slot_index].filename, filename);
	// fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
	// fs.root_directory[empty_slot_index].cluster = empty_slot_index;
	__cio_printf("Successfully created DirectoryEntry %s, a %s created at block %d\n", fs.root_directory[empty_slot_index].filename, fs.root_directory[empty_slot_index].type == 1 ? "file" : "dir", fs.root_directory[empty_slot_index].cluster);
	return 0;
}

int _fs_create_entry(const char *filename, EntryType type) {
	__cio_puts("Creating...\n");
    // Check if the filename is valid
    if (filename == NULL || *filename == '\0') {
		__cio_printf("Invalid filename found\n");
        return -1; // Invalid filename
    }

    // Check if the filename already exists
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

    // Create an entry for the file or directory
    // For simplicity, let's assume each entry occupies a fixed size block on the RAM disk
    // and there's a fixed number of blocks reserved for entries

    // Request space for the entry on the RAM disk
    // DirectoryEntry *entry_address = (DirectoryEntry *)fs.disk.request_space(sizeof(DirectoryEntry));
    // if (entry_address == NULL) {
	// 	__cio_printf("Failed to allocate space for entry with filename %s", filename);
    //     return -1; // Unable to allocate space for the entry
    // }

    DirectoryEntry *new_entry = &fs.root_directory[empty_slot_index];

    // Initialize the entry
    __strcpy(new_entry->filename, filename);
    new_entry->type = type;
    new_entry->size = 0; // Initialize size to 0 for directories
	new_entry->cluster = fs.disk.write(new_entry, sizeof(DirectoryEntry));
	// TODO SEAN: need to add FAT entry

    // Optionally, update filesystem metadata, such as directory entries, etc.
    // Your implementation to update filesystem metadata here...

    return 0; // Entry creation successful
}

/*
int _fs_create_entry(const char *filename, EntryType type){

	

	// Populate the empty slot with the new entry details
    DirectoryEntry *new_entry = &fs.root_directory[empty_slot_index];
    __strcpy(new_entry->filename, filename);
    new_entry->size = 0;
    new_entry->type = (type == FILE_ENTRY) ? FILE_ATTRIBUTE : DIRECTORY_ATTRIBUTE;
    new_entry->cluster = empty_slot_index; // Allocate a block for the new entry's data
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
	// fs.root_directory[empty_slot_index].cluster = empty_slot_index;
	__cio_printf("Successfully created DirectoryEntry %s, a %s created at block %d\n", fs.root_directory[empty_slot_index].filename, fs.root_directory[empty_slot_index].type == 1 ? "file" : "dir", fs.root_directory[empty_slot_index].cluster);
	return 0;
}
*/
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
	int start_block = entry->cluster;
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
	uint32_t start_block = entry->cluster;
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
	__cio_printf("This directory entry has filename %s, size %d, at block %d\nDumping buffer...\n", entry->filename, entry->size, entry->cluster);
	read_block(entry->cluster);
	__cio_printf("%s", fs.buffer);
	return 0;
}

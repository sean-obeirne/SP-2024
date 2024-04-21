#include "filesystem.h" // Include the corresponding header file
#include "fshelper.h" // Include the corresponding header file
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management
#include "ramdisk.h" // Include memory management


// Declare global variables for FileSystem and disk
static FileSystem fs;
// static char *cwd = "/"; // init cwd as root
// static unsigned char disk_image[DISK_SIZE];
// static unsigned char disk_image[BLOCK_SIZE * DISK_SIZE];
static unsigned char disk_image[TOTAL_SIZE];


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////HELPER FUNCTIONS/////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


void pl(char line_char) {
	if(line_char == NULL){
		line_char = '-';
	}
	for(int i = 0; i < 80; i++){
		__cio_putchar(line_char);
	}
}

// Print header then print full line of line_char
void plh( const char *header, char line_char ){
	if(header != NULL){
		__cio_puts(header);
		if (header[__strlen(header)-1] != '\n'){
			__cio_putchar('\n');
		}
	}
	pl(line_char);
}

// Print header (or don't), then print 'half' line
void phl( const char * header ){
	if(header != NULL){
		__cio_puts(header);
		if (header[__strlen(header)-1] != '\n'){
			__cio_putchar('\n');
		}
	}
	__cio_puts("+--------------------------------------+\n");
}

// Print header (or don't), then print variable length line
void pvl( const char *header, char line_char ){
	if(header == NULL){
		__cio_printf("ERROR: Cannot print header, header is null\n");
		return;
	}
	if(line_char == NULL){
		line_char = '-';
	}
	// Print header
	__cio_puts(header);
	if (header[__strlen(header)-1] != '\n'){
		__cio_putchar('\n');
	}
	// Print dashes
	int num_dashes = __strlen(header);
	bool_t maxxed = num_dashes >= 80 ? true : false;
	while(num_dashes > 0){
		__cio_putchar(line_char);
		num_dashes--;
	}
	if(!maxxed)
		__cio_putchar('\n');
}

void init_fs_buffer( void ) {
	fs.buffer = (char *)_km_page_alloc(1);
}

void clear_fs_buffer( void ) {
	// Fill the fs.buffer with zeros
	__memclr(fs.buffer, BLOCK_SIZE);
}

int dump_fs_buffer( void ){
	pvl("Dumping Buffer:\n", '-');
	__delay(20);
	__cio_printf("%s\n", (char *)fs.buffer);
	__delay(20);
	return 0;
}

int dump_root( void ){
	pvl("Dumping Root:\n", '-');
	for(int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++){
		if( __strcmp(fs.root_directory[i].filename, "") != 0 ){
			__cio_printf("file %d: filename:%s\n", i, fs.root_directory[i].filename);
			__delay( 20 );
		}
	}
	return 0;
}

void dump_fat( void ) {
	phl("Dumping FAT\n");
    // Iterate over each entry in the FAT table
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Print the details of each FAT entry
        __cio_printf("FAT Entry %d: Next Cluster = %d, Status = %d\n", i, fs.fat->entries[i].next_cluster, fs.fat->entries[i].status);
		__delay(5);
    }
}
/*
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
*/
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

void strip_path(const char *path, char *final_element){
	char file_name[MAX_PATH_LENGTH];
	file_name[0] = '\0';

    // Check if path is empty or null
    if (path == NULL || *path == '\0') {
		__cio_printf("Path %s is null\n", path);
        return;
    }

	int path_i = 0;
	int file_name_i = 0;
	while(path[path_i] != '\0'){
		if (path[path_i] == '/'){
			// __cio_printf("THIS NEW PATH ITEM: %s\n", file_name);
			file_name[0] = '\0';
			file_name_i = 0;
			// __delay(20);
		}
		else{
			file_name[file_name_i] = path[path_i];
			file_name_i++;
		}
		path_i++;
	}
	file_name[file_name_i] = '\0';
	__strcpy(final_element, file_name);
}

void old_parse_path(const char *path, char **dir_names, char *file_name, int *num_dirs) {
	// Initialize file name
    file_name[0] = '\0';
	*num_dirs = 0;

    // Check if path is empty or null
    if (path == NULL || *path == '\0') {
        return;
    }
	char scratch_path[MAX_FILENAME_LENGTH];
	// __strcpy(scratch_path, path);

	int path_i = 0;
	int dir_names_i = 0;
	int file_name_i = 0;
	while(path[path_i] != '\0'){
		if (path[path_i] == '/' && path_i != 0){
			scratch_path[path_i] = '\0';
			__strcpy(dir_names[dir_names_i], scratch_path);
			*num_dirs += 1;
			// __cio_printf("THIS NEW DIR_NAME: %s\n", dir_names[dir_names_i]);
			dir_names_i++;
			file_name[0] = '\0';
			file_name_i = 0;
			// __delay(50);
		}
		else{
			file_name[file_name_i] = path[path_i];
			file_name_i++;
		}
		scratch_path[path_i] = path[path_i];
		path_i++;
	}
	scratch_path[path_i] = '\0';
}

void parse_path(const char *path, DeconstructedPath *dp) {
	#ifdef DEBUG
	__cio_printf("Deconstructing path...\n");
	#endif
    // Initialize file name
    dp->file_name[0] = '\0';
    dp->num_dirs = 0;
	__strcpy(dp->path,path);

    // Check if path is empty or null
    if (path == NULL || *path == '\0') {
        return;
    }

    // Other local variables
    char scratch_path[MAX_PATH_LENGTH];
    char file_name[MAX_FILENAME_LENGTH];
    int path_i = 0;
    int dir_names_i = 0;
    int file_name_i = 0;

    // Loop through the path string
    while (path[path_i] != '\0') {
		scratch_path[path_i] = path[path_i];
        if (path[path_i] == '/') {
            // Null-terminate the directory name in scratch_path
			if (path_i == 0){
            	file_name[0] = '/';
            	file_name[1] = '\0';
			} else{
            	file_name[file_name_i] = '\0';
            	scratch_path[path_i] = '\0';
			}
            // Allocate memory for the directory name and copy it into the paths array
            dp->dirs[dir_names_i] = _km_page_alloc(1);
            __strcpy(dp->dirs[dir_names_i], file_name);
            dp->paths[dir_names_i] = _km_page_alloc(1);
            __strcpy(dp->paths[dir_names_i], scratch_path);
			scratch_path[path_i] = path[path_i];
            dp->num_dirs++;
            dir_names_i++;
            // Reset the index for the file name
            file_name_i = 0;
        } else {
            // Append character to scratch_path for directory or file name
            file_name[file_name_i++] = path[path_i];
			// __cio_printf("idk, here is the scratch path: %s\n", scratch_path);
        }
        path_i++;
    }

    // Null-terminate the file name in scratch_path
    scratch_path[file_name_i] = '\0';
    // Copy the file name into dp->file_name
    __strcpy(dp->file_name, file_name);
}

DirectoryEntry *test_parse_path(const char *path) {
	__cio_printf("Testing parse_path()\n");
	DeconstructedPath dp;  // Declare a DeconstructedPath variable
    // Now you can pass the address of dp to parse_path() for initialization
    parse_path(path, &dp);
	__cio_printf("Path: %s\n", dp.path);
	__cio_printf("Number of directories: %d\n", dp.num_dirs);
    __cio_printf("File name: %s\n", dp.file_name);
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("Directory %d: %s\n", i + 1, dp.dirs[i]);
    }
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("PATH %d: %s\n", i + 1, dp.paths[i]);
    }
	return NULL;
}

int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert){
	#ifdef DEBUG
	__cio_printf("Adding sub entry...\n");
	#endif
	if(dest == NULL || insert == NULL){
		__cio_printf("ERROR: Invalid parameters to add_sub_entry()\n");
		return -1;
	}
	if(dest->type != 2){
		// __cio_printf("%s\n", dest->subdirectory->files[0].filename);
		__cio_printf("ERROR: Adding DirectoryEntry %s to a file, %s, aborting...\n", dest->filename, insert->filename);
		return -1;
	}
	dest->subdirectory->files[0] = *insert;
	dest->subdirectory->num_files += 1;
	#ifdef DEBUG
	__cio_printf("Subdirectory created, %s now contains %s\n", dest->filename, dest->subdirectory->files[0].filename);
	#endif
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

int get_subdirectory_count(DirectoryEntry *parent){
	return 0;
}

int dir_contains(DirectoryEntry *parent, const char *target){
	int subdir_count = get_subdirectory_count(parent);
	for(int i = 0; i < subdir_count; i++){
		if(parent->subdirectory->files[i].filename == target){
			return 0;
		}
	}
	return -1;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
////////////////////DIRECTORY FUNCTIONS////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

Directory *open_dir(const char *path) {
    // Implement logic to open a directory
    return NULL;
}

int close_dir(Directory *dir) {
    // Implement logic to close a directory
    return 0;
}

int list_dir_contents(const char *path) {
	#ifdef DEBUG
	__cio_printf("Listing...\n");
	#endif
	DirectoryEntry *entry = _fs_find_entry(path);
	Directory *dir = (Directory *)entry->subdirectory;
	phl(NULL);
	__cio_printf(" Contents of %s \"%s\":\n", entry->type == 1 ? "file": "directory", entry->filename);
	if (entry->type == 1){
		__cio_printf("ERROR: File %s found while listing directory contents", entry->filename);
		return -1;
	}
	// Print directory contents
	for (uint32_t i = 0; i < dir->num_files; i++) {
		__cio_printf("  -> %s: %s\n", dir->files[i].filename, dir->files[i].type == 1 ? "file": "directory"); //TODO SEAN expand the info here
	}
	__cio_putchar('\n');
	return 0;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//////////////////////FILE SYSTEM//////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

/* FILESYSTEM OPERATIONS */
int _fs_init( void ) {
	__cio_printf("\nFilesystem Implementation:\n");
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
        __cio_printf("ERROR: failed to initialize storage\n");
        return -1;
    }
	fs.disk = disk;

	// Request free space on the RAM disk for the FAT
    fs.fat = (FAT *)disk.request_space(fs.fat_size_sectors * fs.bytes_per_sector);
    if (fs.fat < 0) {
        __cio_printf("ERROR: failed to request free space for FAT\n");
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
	#ifdef DEBUG
	__cio_printf("fs.fat: %d to %d\n", (int)fs.fat, (int)((char*)fs.fat + (fs.fat_size_sectors * fs.bytes_per_sector)));
	#endif
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
	#ifdef DEBUG
	__cio_printf("Filesystem has been initialized\n");
	#endif

	// Return 0 on success
	return 0;
}

int _fs_mount( void ) {
	// Implement FAT32 mounting logic here
	// Mount the filesystem using the provided filesystem structure
	return 0; // Return 0 on success
}

DirectoryEntry *get_dir_entry_from_path(const char *path){
	//path = /dir/subdir/file.txt
	// start at root
	// DirectoryEntry *entry = &fs.root_directory[0];
	// go to dir
	// go to subdir
	// go to file.txt
	// return file.txt
	return NULL;
}

DirectoryEntry *_fs_find_entry_from_path(const char *path) {
	// path = "/dir/subdir"
	// __cio_printf("FINDING BY PATH, DUMPING ROOT BEFORE FIND\n");
	// dump_root();
	
	DeconstructedPath dp;
    // Pass the address of dp to parse_path() for initialization
    parse_path(path, &dp);
	// test_parse_path(path);
	
	// __cio_printf("dp.dirs[0]: %s\n", dp.dirs[0]);
	// __cio_printf("dp.num_dirs: %d\n", dp.num_dirs);
	DirectoryEntry *curr_parent = _fs_find_entry(dp.dirs[0]); // TODO SEAN - this will become from_path(dp.paths[0])
	if(__strcmp(curr_parent->filename, "/") == 0 && dp.num_dirs == 1){ // we are looking in root
		// __cio_printf("FINDING ENTRY %s IN ROOT\n", dp.file_name);
		return _fs_find_entry(dp.file_name);
	}
	DirectoryEntry *curr_entry;
	for(int i = 1; i < dp.num_dirs; i++){
		curr_entry = _fs_find_entry(dp.dirs[i]);  // TODO SEAN - this will become from_path(dp.paths[0])
		if (curr_parent->type == FILE_ATTRIBUTE){
			__cio_printf("ERROR: Type mismatch, found parent \"file\" %s while looking for directory\n", curr_parent->filename);
		}
		if (curr_entry->type == FILE_ATTRIBUTE){
			break;
		}
		else{
			// add_sub_entry(curr_parent, curr_entry);
			// list_dir_contents(curr_parent);
			return NULL;
		}
		#ifdef DEBUG
		__cio_printf("num_files in directory %s: %d\n", curr_entry->filename, curr_parent->subdirectory->num_files);
		__cio_printf("curr entry filename: %s\n", curr_entry->filename);
		#endif
	}
	return curr_entry;
/*
	__delay(200);
	char *dirs[MAX_PATH_LENGTH]; // Double pointer to store directory names
	for (int i = 0; i < MAX_PATH_LENGTH; i++) {
		dirs[i] = (char *)_km_page_alloc(1); // Assuming _km_alloc allocates memory
	}
	char filename[MAX_FILENAME_LENGTH + 1]; // Buffer to store file name
	int num_dirs = 0;

	// parse_path(path, dirs, filename, &num_dirs);
	__cio_printf("Path: %s\n", path);
	for(int i = 0; i < num_dirs; i++){
		__cio_printf("dirs[%d]: %s\n", i, dirs[i]);
	}
	__cio_printf("filename: %s\n", filename);
	__cio_printf("num_dirs: %d\n", num_dirs);

	clear_fs_buffer();
	strip_path(path, fs.buffer);
	if(num_dirs < 1){ // no parent dirs to create
		__cio_printf("TIME TO RETURN EARLY! FINDING %s\n", fs.buffer);
		__delay(200);
		return _fs_find_entry(fs.buffer);
	}

	// some parent dirs necessary
	DirectoryEntry *curr_parent_dir_entry = _fs_find_entry(dirs[0]);
	for(int i = 1; i <= num_dirs; i++){
		DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);
	}

	strip_path(dirs[0], fs.buffer);
	__cio_printf("TRYING TO FIND DIRS[0], FROM: %s\n", dirs[0]);
	__cio_printf("TRYING TO FIND DIRS[0], AKA: %s\n", fs.buffer);
	__delay(20);
	// __cio_printf("CURRENT STRIPPED ITEM: %s\n", fs.buffer);
	DirectoryEntry *curr_dir_entry = _fs_find_entry(fs.buffer);
	if (curr_dir_entry == NULL){
		__cio_printf("NEEDING TO CREATE BUT SHOULDNTTTTTTTTTT\n");
		__delay(10);
		_fs_create_entry(fs.buffer, DIRECTORY_ENTRY);
		curr_dir_entry = _fs_find_entry(fs.buffer);
	}



	__cio_printf("======PATH, CURR, PARENT IN FIND======\n");
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	__delay(200);

	// __cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	// __cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry, curr_parent_dir_entry->filename);
	// __delay(50);
	for(int i = 1; i < num_dirs; i++){

		// Check if the directory already exists
		DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);

		__cio_printf("curr_dir_entry: %s curr_parent_dir_entry %s", curr_dir_entry->filename, curr_parent_dir_entry->filename);
		__delay(50);
		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (curr_dir_entry == NULL) {
			__cio_printf("Failed to find entry for directory %s", dirs[i]);
			__delay(100000);
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
	__delay(100000);
	return entry;
*/
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
	// __cio_printf("Entry for file %s found, size = %d, block = %d\n", entry->filename, entry->size, entry->cluster);

	return entry;
}

int _fs_create_entry_from_path(const char *path, EntryAttribute type){
	#ifdef DEBUG
	__cio_printf("Creating (from path %s)...\n", path);
	#endif

	char *dirs[MAX_PATH_LENGTH]; // Double pointer to store directory names
	for (int i = 0; i < MAX_PATH_LENGTH; i++) {
		dirs[i] = (char *)_km_page_alloc(1); // Assuming _km_alloc allocates memory
	}
	char filename[MAX_FILENAME_LENGTH + 1]; // Buffer to store file name
	int num_dirs = 0;

	// PARSE PATH
	// parse_path(path, dirs, filename, &num_dirs);
	__cio_printf("Path: %s\n", path);
	for(int i = 0; i < num_dirs; i++){
		__cio_printf("dirs[%d]: %s\n", i, dirs[i]);
	}
	__cio_printf("filename: %s\n", filename);
	__cio_printf("num_dirs: %d\n", num_dirs);
	__delay(50);

	
	// for(int i = 0; i < num_dirs; i++){
	// 	strip_path(dirs[i], fs.buffer);
	// 	__cio_printf("Path: %s\n", dirs[i]);
	// 	__cio_printf("Stripped path: %s\n", fs.buffer);
	// }
	// __delay(100);

	clear_fs_buffer();
	// __cio_printf("Dumping root in create_entry_from_path...\n");
	// dump_root();

	DirectoryEntry *curr_parent_dir_entry = &fs.root_directory[0];
	strip_path(dirs[0], fs.buffer);
	__cio_printf("CURRENT STRIPPED ITEM: %s\n", fs.buffer);
	DirectoryEntry *curr_dir_entry = _fs_find_entry(fs.buffer);
	if (curr_dir_entry == NULL){
		_fs_create_entry(fs.buffer, DIRECTORY_ATTRIBUTE);
		curr_dir_entry = _fs_find_entry(fs.buffer);
	}

	// Check if the directory already exists
	// DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[1]);
	// DirectoryEntry *curr_dir_entry = _fs_find_entry("dir");

	__cio_printf("======PRE======\n");
	__cio_printf("  Path: %s\n", path);
	__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
	__delay(20);

	for(int i = 0; i <= num_dirs; i++){ // num_dirs[0] is in root, already handled

		// strip_path(dirs[i], fs.buffer);
		// Check if the directory already exists
		curr_dir_entry = _fs_find_entry_from_path(dirs[i]);

		/*

		__cio_printf("======CURRENT======\n");
		__cio_printf("  Path: %s\n", path);
		__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
		__delay(20);
		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (curr_dir_entry == NULL) {
			if(_fs_create_entry(dirs[i], DIRECTORY_ENTRY) != 0){
				__cio_printf("Failed to create entry for directory %s\n", dirs[i]);
				return -1;
			} else{
				__cio_printf("CREATED DirectoryEntry %s, type %d\n", dirs[i]);
			}
			DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);
			
		}
		*/
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
		__cio_printf("ERROR: Root directory full\n");
		return -1;
	}

	// Populate the empty slot with the new entry details
	DirectoryEntry *new_entry = &fs.root_directory[empty_slot_index];
	__strcpy(new_entry->filename, filename);
	new_entry->size = 0;
	new_entry->type = type;
	new_entry->cluster = empty_slot_index; // Allocate a block for the new entry's data


	// Populate the empty slot with the new entry details
	// __strcpy(fs.root_directory[empty_slot_index].filename, filename);
	// fs.root_directory[empty_slot_index].size = 0; // Set size to 0 initially
	// fs.root_directory[empty_slot_index].cluster = empty_slot_index;
	__cio_printf("Successfully created DirectoryEntry %s, a %s created at block %d\n", fs.root_directory[empty_slot_index].filename, fs.root_directory[empty_slot_index].type == 1 ? "file" : "dir", fs.root_directory[empty_slot_index].cluster);


	__cio_printf("Dumping root in END OF create_entry_from_path...\n");
	dump_root();
	return 0;
}

int _fs_create_entry(const char *filename, EntryAttribute type) {
	#ifdef DEBUG
	__cio_printf("Creating...\n");
	#endif
    // Check if the filename is valid
    if (filename == NULL || *filename == '\0') {
		__cio_printf("ERROR: Invalid filename\n");
        return -1; // Invalid filename
    }

    // Check if the filename already exists
	// No duplicate filenames
	if(_fs_find_entry(filename) != NULL){
		__cio_printf("ERROR: File %s already exists\n", filename);
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
		__cio_printf("ERROR: Root directory is full\n");
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
	// __cio_printf("New entry filename: %s\n", new_entry->filename);
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
	#ifdef DEBUG
	__cio_printf("Renaming...\n");
	#endif
	DirectoryEntry *src = _fs_find_entry(old_filename);
	DirectoryEntry *dest = _fs_find_entry(new_filename);
	if(dest != NULL){
		__cio_printf("ERROR: File %s already exists\n", new_filename);
		return -1;
	}
	__memcpy(src->filename, new_filename, MAX_FILENAME_LENGTH);
	return 0;
}

int _fs_delete_entry(const char *filename){
	#ifdef DEBUG
	__cio_printf("Deleting...\n");
	#endif
	DirectoryEntry *entry = _fs_find_entry(filename);
	if(entry == NULL){
		__cio_printf("ERROR: File %s does not exist", filename);
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
	#ifdef DEBUG
	__cio_printf("Reading...\n");
	#endif
	// Read data from the specified file into fs.buffer

	// __cio_puts("READING... \n");

	// Search for the file in the root directory
	DirectoryEntry *entry = _fs_find_entry(filename);
	if (entry == NULL) {
		__cio_printf("ERROR: filename %s NOT found!\n", filename);
		return -1;
	}

	// Traverse the FAT to find the file's data blocks
	int start_block = entry->cluster;
	int current_block = start_block;
	while (current_block != FAT_EOC) {
		// Calculate the block number corresponding to the current cluster

		// Read the block from the disk image into the buffer
		#ifdef DEBUG
		__cio_printf("reading block %d\n", current_block);
		__delay(20);
		#endif
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
	#ifdef DEBUG
	__cio_printf("Writing...\n");
	#endif

	// Find the directory entry for the specified filename
	DirectoryEntry *entry = _fs_find_entry(filename);

	// If the file doesn't exist, return an error
	if (entry == NULL) {
		// __cio_printf("Filename %s NOT found! Creating...\n", entry);
		// __delay( 200 );
		// _fs_create_entry(entry->filename, FILE_ENTRY);
		__cio_printf("ERROR: Filename %s NOT found!\n", entry);
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
		#ifdef DEBUG
		__cio_printf("Writing to block number %d\n", block_number);
		#endif
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
	#ifdef DEBUG
	__cio_printf("Printing...\n");
	#endif
	DirectoryEntry *entry = _fs_find_entry(filename);
	if(entry == NULL){
		__cio_printf("ERROR: Entry \"%s\" not found\n", filename);
		return -1;
	}
	__cio_printf("This directory entry (type %d) has filename %s, size %d, at block %d.\n", entry->type, entry->filename, entry->size, entry->cluster);
	dump_fs_buffer();
	read_block(entry->cluster);
	return 0;
}

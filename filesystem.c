#include "filesystem.h" // Implement filesystem.h
#include "fshelper.h" // DEV: Include helper functions
#include "common.h" // Include custom data types
#include "support.h" // Include delay function
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output
#include "kmem.h" // Include memory management
#include "ramdisk.h" // Include memory management


// Global variables
static FileSystem fs;
static char *cwd = "/";
static char working_path[MAX_PATH_LENGTH];

Directory root_directory = {
    .name = "/",  // Root directory name
	.num_files = 0,
};

// Define the root directory entry
DirectoryEntry root_directory_entry = {
    .filename = "/",  // / string represents the root directory
    .size = 0,       // Root directory typically has size 0
	.type = 2,
    .cluster = 0,
	.next = NULL,
	.subdirectory = &root_directory,
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////////////HELPER FUNCTIONS/////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

// Print a header for a module with no delay
void phn(const char *text, int ticks) {
	__cio_printf(" - clearing - \n");
	__delay(ticks);
	__cio_clearscroll();
	__cio_moveto(0, 0);
	__cio_printf("<========= %s =========>\n", text);
}

// Print a header for a module
void ph(const char *text) {
	__cio_printf(" - clearing - \n");
	__delay(STEP);
	__cio_clearscroll();
	__cio_moveto(0, 0);
	__cio_printf("<========= %s =========>\n", text);
}

// Print an 80-character long line with '-'
void pl() {
	__cio_putchar('X');
	for(int i = 0; i < 78; i++){
		__cio_putchar('-');
	}
	__cio_putchar('X');
	__delay(INF_PAUSE);
}

// Print an 80-character long line with '-', no delay
void pln() {
	for(int i = 0; i < 80; i++){
		__cio_putchar('-');
	}
	// __delay(MOMENT);
}

// Print an 80-character long line with char line_char
void plw(char line_char) {
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
	plw(line_char);
}

// Print header (or don't if header == NULL), then print 'half' line
void phl( const char * header ){
	if(header != NULL){
		__cio_puts(header);
		if (header[__strlen(header)-1] != '\n'){
			__cio_putchar('\n');
		}
	}
	__cio_puts("+--------------------------------------+\n");
}

// Print header, then print variable length line
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
	__delay(STEP);
	__cio_printf("%s\n", (char *)fs.buffer);
	__delay(STEP);
	return 0;
}

int dump_root( void ){
	__cio_putchar('\n');
	pvl("Dumping Root:\n", '-');
	for(int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++){
		if( __strcmp(root_directory.files[i]->filename, NULL) != 0 && __strcmp(root_directory.files[i]->filename, "") != 0 ){
			// _fs_print_entry(fs.root_directory[i].filename);
			__cio_printf("file %d: filename: %s\n", i, root_directory.files[i]->filename);
			__delay(STEP);
		}
	}
	__cio_putchar('\n');
	return 0;
}

void dump_fat( void ) {
	phl("Dumping FAT\n");
    // Iterate over each entry in the FAT table
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Print the details of each FAT entry
        __cio_printf("FAT Entry %d: Next Cluster = %d, Status = %d\n", i, fs.fat->entries[i].next_cluster, fs.fat->entries[i].status);
		__delay(STEP);
    }
}

int read_block(int block_number){ // TODO SEAN chunks
	if (block_number < 0 || block_number >= DISK_SIZE) {
		__cio_printf("ERROR: Block number %d not within bounds %d - %d\n", block_number, 0, DISK_SIZE);
		return -1; // Invalid block number
	}
	// Copy data from disk image to file system buffer

	return 0;
}

int write_block(int block_number, const uint8_t *data_ptr ) { // TODO SEAN chunks
	if (block_number < 0 || block_number >= DISK_SIZE) {
		__cio_printf("ERROR: Block number %d not within bounds %d - %d\n", block_number, 0, DISK_SIZE);
		return -1;
	}

	if (data_ptr == NULL) {
		__cio_printf("ERROR: Data pointer is NULL\n");
		return -1;
	}

	// Calculate the offset in bytes for the block
	// uint32_t byte_offset = block_number * BLOCK_SIZE + offset;

	// Write data to the disk image
	// int start_index = block_number * BLOCK_SIZE;

	return 0;
}

int read_sector( int sector_number, void *buffer ){
	__memset(buffer, SECTOR_SIZE, 48);
	return 0;
}

void parse_path(const char *path, DeconstructedPath *dp) {
	#ifdef DEBUG
	__cio_printf("Deconstructing path...\n");
	__delay(STEP);
	#endif

    dp->filename[0] = '\0';
    dp->num_dirs = 0;
	__strcpy(dp->path,path);

    if (path == NULL || *path == '\0') {
		__cio_printf("ERROR: Path is NULL\n");
        return;
    }

    // Other local variables
    char scratch_path[MAX_PATH_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
    int path_i = 0;
	int scratch_i = 0;
    int filename_i = 0;
    int dir_names_i = 0;
	// __cio_printf("SCRATCH PATH: %s\n", scratch_path);

	while(path_i < __strlen(path)){ // 11
		filename[filename_i] = path[path_i];
		scratch_path[scratch_i] = path[path_i];

		if(path[path_i] == '/'){
			if(path_i == 0){
				filename[filename_i] = '/';
				filename[filename_i + 1] = '\0';
				scratch_path[scratch_i] = '/';
			} else{
				filename[filename_i] = '\0';
				scratch_path[scratch_i] = '\0';
			}

			// __cio_printf("So filename: %s  scrach_path: %s\n", filename, scratch_path);

			// export filename
			dp->dirs[dir_names_i] = _km_page_alloc(1);
            __strcpy(dp->dirs[dir_names_i], filename);
            dp->paths[dir_names_i] = _km_page_alloc(1);
			// __cio_printf("SCRATCH_PATH: %s AT INDEX %d\n", scratch_path, dir_names_i);
            __strcpy(dp->paths[dir_names_i], scratch_path);
			
            dir_names_i++;

			scratch_path[scratch_i] = path[path_i];
			filename_i = -1;
		}

		scratch_i++;
		filename_i++;
		path_i++;
	}
	// __cio_printf("This step, PRE NULL filename=%s and scratch_path=%s\n", filename, scratch_path);
	if(path[path_i] == '\0'){
		filename[filename_i] = '\0';
		scratch_path[scratch_i] = '\0';

		dp->dirs[dir_names_i] = _km_page_alloc(1);
		__strcpy(dp->dirs[dir_names_i], filename);
		dp->paths[dir_names_i] = _km_page_alloc(1);
		// __cio_printf("SCRATCH_PATH: %s AT INDEX %d\n", scratch_path, dir_names_i);
		__strcpy(dp->paths[dir_names_i], scratch_path);
		dp->num_dirs = dir_names_i + 1;
		// __cio_printf("So filename: %s  scrach_path: %s\n", filename, scratch_path);
		__strcpy(dp->filename, filename);
	}
	// __cio_printf("This step, POST NULL filename=%s and scratch_path=%s\n", filename, scratch_path);
	// __delay(STEP);

	// print_parsed_path(*dp);
	// __delay(STEP);

	#ifdef DEBUG
	__cio_printf("Deconstructing path!!!\n");
	__delay(STEP);
	#endif
}

void test_parse_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("Testing parse_path()...\n");
	__delay(STEP);
	#endif
	
	DeconstructedPath dp;
    parse_path(path, &dp);

	__cio_printf("Path: %s\n", dp.path);
	__cio_printf("Number of directories: %d\n", dp.num_dirs);
    __cio_printf("File name: %s\n", dp.filename);
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("Directory %d: %s\n", i + 1, dp.dirs[i]);
    }
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("PATH %d: %s\n", i + 1, dp.paths[i]);
    }
	#ifdef DEBUG
	__cio_printf("Testing parse_path()!!!\n");
	__delay(STEP);
	#endif
}

void print_parsed_path(DeconstructedPath dp) {
	#ifdef DEBUG
	__cio_printf("PRINTING parsed path...\n");
	__delay(STEP);
	#endif
	__cio_printf("Path: %s\n", dp.path);
	__delay(STEP);
	__cio_printf("Number of directories: %d\n", dp.num_dirs);
	__delay(STEP);
    __cio_printf("File name: %s\n", dp.filename);
	__delay(STEP);
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("Directory %d: %s\n", i + 1, dp.dirs[i]);
		__delay(STEP);
    }
    for (int i = 0; i < dp.num_dirs; i++) {
        __cio_printf("PATH %d: %s\n", i + 1, dp.paths[i]);
		__delay(STEP);
    }
	#ifdef DEBUG
	__cio_printf("PRINTING parsed path!!!\n");
	__delay(STEP);
	#endif
}

int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert){
	#ifdef DEBUG
	__cio_printf("Adding sub entry to %s...\n", dest->filename);
	__delay(STEP);
	#endif

	if(dest == NULL){
		__cio_printf("ERROR: add_sub_entry(): dest %s is NULL\n", dest->filename);
		return -1;
	}
	if(insert == NULL){
		__cio_printf("ERROR: add_sub_entry(): insert %s is NULL\n", insert->filename);
		return -1;
	}
	if(dest->type != 2){
		__cio_printf("ERROR: Adding DirectoryEntry %s to a file, %s, aborting\n", dest->filename, insert->filename);
		return -1;
	}
	if(dest->subdirectory == NULL){
		__cio_printf("ERROR: Destination directory %s has a NULL subdirectory\n", dest->filename);
		return -1;
	}
	// dest->subdirectory->files[dest->subdirectory->num_files++] = insert;
	dest->subdirectory->files[dest->subdirectory->num_files] = insert;
	dest->subdirectory->num_files++;
	
	#ifdef DEBUG
	__cio_printf("Adding sub entry to %s!!!\n", dest->filename);
	__delay(STEP);
	#endif
	return 0;
}

int create_sub_entry(DirectoryEntry *parent, const char *filename, EntryAttribute type) {
	#ifdef DEBUG
	__cio_printf("Creating sub_entry of %s...\n", parent->filename);
	__delay(STEP);
	#endif

    // Check if the filename is valid
    if (parent == NULL) {
		__cio_printf("ERROR: Parent %s undefined\n", parent->filename);
        return -1;
    }

	DirectoryEntry *child = _km_page_alloc(1);

	_fs_initialize_directory_entry(child, filename, 0, type, 0, NULL);

	
	int result = add_sub_entry(parent, child);
	if(result != 0){
		__cio_printf("Failed to create %s as child of %s\n", child->filename, parent->filename);
		return -1;
	}
	

	// TODO SEAN: need to add FAT entry

	#ifdef DEBUG
	if(result == 0){
		__cio_printf("Creating sub_entry of %s!!!\n", parent->filename);
	}
	else{
		__cio_printf("Creating sub_entry of %s---\n", parent->filename);
	}
	__delay(STEP);
	#endif
    return 0;

}

int add_fat_entry(uint32_t next_cluster) {
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        if (fs.fat->entries[i].status == FAT_FREE) {
            // Set the next_cluster field to the provided value
            fs.fat->entries[i].next_cluster = next_cluster;
            
            // Update the status field to indicate that the entry is in use
            fs.fat->entries[i].status = FAT_IN_USE;
            
            // Return the index of the added entry
            return i;
        }
    }
    
	__cio_printf("ERROR: No open entry found in FAT");
    return -1;
}

int get_subdirectory_count(DirectoryEntry *parent){
	return 0;
}

int dir_contains(DirectoryEntry *parent, const char *target){
	int subdir_count = get_subdirectory_count(parent);
	for(int i = 0; i < subdir_count; i++){
		if(parent->subdirectory->files[i]->filename == target){
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

int cd(const char *relpath){
	return 0;
}

Directory *open_dir(const char *path) {
	#ifdef DEBUG
	__cio_printf("Opening Directory...\n");
	__delay(STEP);
	#endif

    // Implement logic to open a directory
    return NULL;
}

int close_dir(Directory *dir) {
	#ifdef DEBUG
	__cio_printf("Closing Directory...\n");
	__delay(STEP);
	#endif

    // Implement logic to close a directory
    return 0;
}

int list_dir_contents(const char *path) {
	#ifdef DEBUG
	__cio_printf("Listing...\n");
	__delay(STEP);
	#endif

	DirectoryEntry *entry = _fs_find_entry(path);
	Directory *dir = (Directory *)entry->subdirectory;
	phl(NULL);
	__cio_printf(" Contents of %s \"%s\":\n", entry->type == 1 ? "file": "directory", entry->filename);
	if (entry->type == 1){
		__cio_printf("ERROR: %s is a *file*; cannot list directory contents", entry->filename);
		return -1;
	}
	// Print directory contents
	for (uint32_t i = 0; i < dir->num_files; i++) {
		__cio_printf("  -> %s: %s\n", dir->files[i]->filename, dir->files[i]->type == 1 ? "file": "directory"); //TODO SEAN expand the info here
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
	#ifdef DEBUG
	__cio_printf("Initializing Filesystem\n");
	__delay(STEP);
	#endif

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

	// Initialize StorageInterface disk
    StorageInterface disk;
    int result = storage_init(&disk);
    if (result < 0) {
        __cio_printf("ERROR: Failed to initialize storage\n");
        return -1;
    }

	// Initialize RAM disk
	result = disk.init(DISK_SIZE);
    if (result != 0) {
		__cio_printf("ERROR: Failed to initialize ramdisk\n");
        return result;
    }
	#ifdef DEBUG
	__cio_printf("Ramdisk memory pool initialized with %d bytes\n", DISK_SIZE * SZ_PAGE);
	__delay(STEP);
	#endif
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

	#ifdef DEBUG
	__cio_printf("Filesystem FAT memory ranges from %d to %d\n", (int)fs.fat, (int)((char*)fs.fat + (fs.fat_size_sectors * fs.bytes_per_sector)));
	__delay(STEP);
	#endif

	fs.fat_entry_size = 4;

	// Root Directory Information
	// fs.root_directory_entries = 32;
	// fs.root_directory = (DirectoryEntry *) disk.request_space(sizeof(DirectoryEntry) * ROOT_DIRECTORY_ENTRIES); // TODO SEAN: do not allocate pages, but bytes

	// if( root_directory == NULL ){
		//TODO SEAN: free fs.fat
		// __cio_printf("ERROR: Failed to init the root directory\n");
		// return -1;
	// }

	// add_fat_entry(0);

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
		__cio_printf("ERROR: Failed to init FS buffer\n");
		return -1;
	}
	clear_fs_buffer();

	// Mount status
	fs.mounted = false;

	// Error Handling
	fs.last_error = 0;

	#ifdef DEBUG
	__cio_printf("Filesystem has been initialized\n");
	__delay(STEP);
	#endif

	// Return 0 on success
	return 0;
}

int _fs_mount( void ) {
	#ifdef DEBUG
	__cio_printf("Mounting...\n");
	__delay(STEP);
	#endif

	// Implement FAT32 mounting logic here
	return 0;
}

DirectoryEntry *_fs_find_entry_from_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("  Finding (from path %s)...\n", path);
	__delay(STEP);
	#endif

	DeconstructedPath dp;
    parse_path(path, &dp);

	// print_parsed_path(dp);



	DirectoryEntry *parent = NULL;
	DirectoryEntry *child = NULL;
	// int result = -1;
	int curr_depth = 0;

	if(dp.num_dirs == 1){
		__cio_printf("Find Iterating, 1\n");
		parent = _fs_find_entry(dp.dirs[0]);
		if (parent == NULL){
			__cio_printf("ERROR: Parent %s not found\n", dp.dirs[0]);
			return NULL;
		}
		__cio_printf("Depth of %d:\n  Parent: %s\n", curr_depth, parent->filename, child->filename);
		curr_depth = 1;
	}

	else if(dp.num_dirs == 2){
		__cio_printf("Find Iterating, 2\n");
		parent = _fs_find_entry(dp.dirs[0]);
		child = _fs_find_entry(dp.dirs[1]);
		__cio_printf("dp.dirs[1] = %s\n", dp.dirs[1]);
		if (parent == NULL){
			__cio_printf("ERROR: Parent %s not found\n", dp.dirs[0]);
			return NULL;
		}
		if (child == NULL){
			__cio_printf("FAILED: Child of %s, \"%s\", not found\n", dp.dirs[0], dp.dirs[1]);
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
		__cio_printf("Depth of %d:  Parent: %s,  Child: %s\n", curr_depth, parent->filename, child->filename);
		curr_depth = 2;
	}
	
	else if (dp.num_dirs == 3){
		__cio_printf("Find Iterating, 3\n");
		parent = _fs_find_entry(dp.dirs[1]);
		if (parent == NULL){
			__cio_printf("ERROR: Parent %s not found\n", dp.dirs[1]);
			return NULL;
		}
		__cio_printf("Looking for child at %s\n", dp.paths[2]);
		child = NULL;
		for(int i = 0; i < parent->subdirectory->num_files; i++){
			if (__strcmp(parent->subdirectory->files[i]->filename, dp.dirs[2]) == 0 ){ // found child!
				child = parent->subdirectory->files[i];
			}
		}
		if(child == NULL){
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
		curr_depth = 3;
	}
	// dump_root();
	__cio_printf("Parent: %s\n", parent->filename);
	__cio_printf("Child: %s\n", child->filename);

	#ifdef DEBUG
	__cio_printf("  Finding (from path %s)!!!\n", path);
	__delay(STEP);
	#endif
	return child;



	// // DirectoryEntry *curr_parent = _fs_find_entry_from_path(dp.paths[0]); // TODO SEAN - this will become from_path(dp.paths[0])
	// __cio_printf("Hey we found it!\n");
	
	// // __cio_printf("dp.dirs[0]: %s\n", dp.dirs[0]);
	// // __cio_printf("dp.num_dirs: %d\n", dp.num_dirs);
	// __cio_printf("Is %s comparable to %s?\n", dp.dirs[0], curr_parent->filename);
	// if(__strcmp(curr_parent->filename, "/") == 0 && dp.num_dirs == 0){ // we are looking in root
	// 	// __cio_printf("FINDING ENTRY %s IN ROOT\n", dp.filename);
	// 	#ifdef DEBUG
	// 	__cio_printf("Finding (from path %s)!!!\n", path);
	// 	__delay(STEP);
	// 	#endif
	// 	return _fs_find_entry(dp.filename);
	// }
	// DirectoryEntry *curr_entry;
	// for(int i = 1; i < dp.num_dirs; i++){
	// 	phl(dp.dirs[i]);
	// 	if (curr_parent->type == FILE_ATTRIBUTE){
	// 		__cio_printf("ERROR: Type mismatch, found parent \"file\" %s while looking for directory\n", curr_parent->filename);
	// 	}
	// 	if (__strcmp(curr_entry->filename, dp.filename) == 0){ //TODO SEAN: neds to be smarter than just filename
	// 		__cio_printf("Found matching filenames: %s %s\n", curr_entry->filename, dp.filename);
	// 		#ifdef DEBUG
	// 		__cio_printf("Finding (from path %s)!!!\n", path);
	// 		__delay(STEP);
	// 		#endif
	// 		break;
	// 	}
	// 	// __cio_printf("WE will now go to path %s", dp.paths[i]);
	// 	curr_entry = _fs_find_entry_from_path(dp.paths[i]);
	// }
	// __cio_printf("RETURNING\n");
	// __delay(1000);
	// return curr_entry;
}

DirectoryEntry *_fs_find_entry(const char *filename) {
	if (__strcmp("/", filename) == 0){
		return &root_directory_entry;
	}
	if (filename[0] == '/'){
		__cio_printf("Path given, filename expected. Finding entry from path %s\n", filename);
		return _fs_find_entry_from_path(filename);
	}

	#ifdef DEBUG
	__cio_printf("Finding %s in root...\n", filename);
	__delay(STEP);
	#endif

	// Search for the file in the root directory
	DirectoryEntry *entry = NULL;
	for (int i = 0; i < ROOT_DIRECTORY_ENTRIES; i++) {
		if (__strcmp(root_directory.files[i]->filename, "") != 0){
			// __cio_printf("COMPING %s WITH %s\n", root_directory.files[i]->filename, filename);
			// __cio_printf("Lets see... %s\n", root_directory.files[i]);
			if (__strcmp(root_directory.files[i]->filename, filename) == 0) {
				// __cio_printf("%d\n", i);
				entry = root_directory.files[i];
				break;
			}
		} 
	}


	#ifdef DEBUG
	if(entry == NULL){
		__cio_printf("Finding %s in root---\n", filename);
	} else{
		__cio_printf("Finding %s in root!!!\n", filename);
	}
	__delay(STEP);
	#endif

	return entry;
}

int _fs_create_entry_from_path(const char *path, EntryAttribute type){
	#ifdef DEBUG
	__cio_printf("  Creating (from path %s)...\n", path);
	__delay(STEP);
	#endif
	int result = -1;

	DeconstructedPath dp;
    parse_path(path, &dp);


	__cio_printf("...\n");
	DirectoryEntry *entry = _fs_find_entry_from_path(path);
	__cio_printf("!!!\n");
	__delay(MOMENT);
	if(entry != NULL){
		__cio_printf("ERROR: File %s already exists at %s", entry->filename, path);
		return -1;
	}
	__cio_printf("ENTRY: %s %d %d\n", entry->filename, entry->type, entry->size);
	
	// result = _fs_create_root_entry(dp.filename, type);
	// if (result != 0){
	// 	__cio_printf("Failed to create essential root entry %s\n", dp.filename);
	// 	return NULL;
	// }
	__cio_printf("poopybutt");
	entry = _fs_find_entry_from_path(path);
	
	// dump_root();
	// _fs_print_entry(_fs_find_entry("/"), true);
	// if(entry != NULL){
	// 	__cio_printf("ERROR: File with path %s already exists\n", path);
	// 	return NULL;
	// }
	DirectoryEntry *parent = NULL;
	DirectoryEntry *child = NULL;
	int curr_depth = 0;

	
	// dump_root();
	__cio_printf("We should be iterating %d times\n", dp.num_dirs);
	if(dp.num_dirs == 1){
		__cio_printf("Create Iterating, 1\n");
		parent = _fs_find_entry(dp.dirs[0]);
		// __cio_printf("Parent: %s\n", parent);
		if (parent == NULL){
			__cio_printf("ERROR: Unable to find root");
			return -1;
		}
		__cio_printf("Depth of %d:\n  Parent: %s\n", curr_depth, parent->filename, child->filename);
		curr_depth = 1;
	}
	// dump_root();
	else if(dp.num_dirs == 2){
		__cio_printf("Create Iterating, 2\n");
		parent = _fs_find_entry(dp.dirs[0]);
		// __cio_printf("Parent: %s\n", parent);
		if (parent == NULL){
			__cio_printf("ERROR: Unable to find root");
			return -1;
		}

		child = _fs_find_entry(dp.dirs[1]);

		// __cio_printf("Child: %s\n", child->filename);
		// __delay(STEP);
		if (child == NULL){
			result = _fs_create_root_entry(dp.dirs[1], DIRECTORY_ATTRIBUTE);
			if(result != 0){
				__cio_printf("ERROR: Failed to find then create child \"%s\"\n", dp.dirs[1]);
				return -1;
			}
			child = _fs_find_entry(dp.dirs[1]);
			if(child == NULL){
				__cio_printf("ERROR: Failed to find created child \"%s\"\n", dp.dirs[1]);
				return -1;
			}
		}
		__cio_printf("Depth of %d:\n  Parent: %s,  Child: %s\n", curr_depth, parent->filename, child->filename);
		curr_depth = 2;
	}
	
	
	// dump_root();
	if (dp.num_dirs == 3){
		__cio_printf("Create Iterating, 3\n");
		parent = _fs_find_entry(dp.dirs[1]);
		if(parent == NULL){
			__cio_printf("Parent %s is NULL\n", dp.dirs[1]);
			return -1;
		}
		__cio_printf("Parent: %s\n", parent->filename);
		child = _fs_find_entry_from_path(dp.paths[2]);
		if(child == NULL){
			__delay(MOMENT);
			__cio_printf("Child %s is NULL\n", dp.dirs[2]);
			// _fs_print_entry(child, true);
			
			__delay(LONG_PAUSE);

			_fs_initialize_directory_entry(child, dp.dirs[2], 0, DIRECTORY_ATTRIBUTE, 0, NULL);

			__delay(LONG_PAUSE);

			__cio_printf("YEHA THIS SUCKS\n");
			__delay(LONG_PAUSE);
			add_sub_entry(parent, child);

			_fs_print_entry(parent, true);

			return -1;
		}
		__cio_printf("Child: %s\n", child->filename);

		pln();
		// __delay(LONG_PAUSE);
		dump_root();
		__delay(MOMENT);
		print_parsed_path(dp);

		__cio_printf("OK WE ARE HERE NOW: %s , created %s\n", dp.dirs[1], dp.paths[2]);
		__cio_printf("parent: %s , child: %s\n", parent->filename, child->filename);
		// pl();
		if(child == NULL){
			__delay(DEBUG_DELAY);
			// create_sub_entry(parent, dp.dirs[2], DIRECTORY_ATTRIBUTE);
			child = _fs_find_entry_from_path(dp.paths[2]);
			_fs_print_entry(parent, true);
			__cio_printf("DID WE GET THE CHILD? %s\n", child->filename);
			// _fs_create_entry_from_path(path, DIRECTORY_ATTRIBUTE)
		}

		__cio_printf("Depth of %d:\n  Parent: %s,  Child: %s\n", curr_depth, parent->filename, child->filename);
		
		curr_depth = 3;
	}
	// dump_root();
	__cio_printf("Parent: %s\n", parent->filename);
	__cio_printf("Child: %s\n", child->filename);

	#ifdef DEBUG
	__cio_printf("  Creating (from path %s)!!!\n", path);
	__delay(STEP);
	#endif
	return 0;

/*
	while(curr_depth < MAX_DEPTH){
		break;
	}
	if(parent->filename[0] = '/'){
		_fs_find_entry(entry);
	}
	// __cio_printf("Parent: %s\n", dp.dirs[0]);
	if(dp.num_dirs > 0){
		// TODO SEAN - check 0 case, see if mounted?	
	}
*/
	// __delay(STEP);

	// clear_fs_buffer();
	// int result = -1;
	// if(dp.num_dirs <= 1){ // create entry in root
	// 	result = _fs_create_root_entry(dp.filename, type);
	// 	return result;
	// }

	// num_dirs >= 1
	// DirectoryEntry *parent = _fs_find_entry(dp.dirs[0]);
	// __delay(STEP);
	// 
	
	// print_parsed_path(dp);
	// if(child->)
	int path_i = 1;
	while(path_i <= dp.num_dirs){

		path_i++;
	}
	return 0;

	DirectoryEntry *file = _fs_find_entry(dp.dirs[0]);
	__cio_printf("FILENAME: %s\n",file->filename);
	int i = 1;
	while (file != NULL && i <= dp.num_dirs){
		__cio_printf("finding %s at i=%d\n", dp.paths[i], i);
		file = _fs_find_entry_from_path(dp.paths[i]);
		__cio_printf("found: %s\n", file->filename);
		__delay(STEP);
		
		i++;
	}



	/*
	DirectoryEntry *curr_parent_dir_entry = &fs.root_directory[0];
	strip_path(dirs[0], fs.buffer);
	__cio_printf("CURRENT STRIPPED ITEM: %s\n", fs.buffer);
	DirectoryEntry *curr_dir_entry = _fs_find_entry(fs.buffer);
	if (curr_dir_entry == NULL){
		_fs_create_root_entry(fs.buffer, DIRECTORY_ATTRIBUTE);
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

		

		__cio_printf("======CURRENT======\n");
		__cio_printf("  Path: %s\n", path);
		__cio_printf("  curr_dir_entry: %s curr_parent_dir_entry %s\n", curr_dir_entry->filename, curr_parent_dir_entry->filename);
		__delay(20);
		// If the directory doesn't exist, create it as a subdirectory under the parent directory entry
		if (curr_dir_entry == NULL) {
			if(_fs_create_root_entry(dirs[i], DIRECTORY_ENTRY) != 0){
				__cio_printf("Failed to create entry for directory %s\n", dirs[i]);
				return -1;
			} else{
				__cio_printf("CREATED DirectoryEntry %s, type %d\n", dirs[i]);
			}
			DirectoryEntry *curr_dir_entry = _fs_find_entry(dirs[i]);
			
		}

		// Update the parent directory entry to the current directory entry
		// if(_fs_find_entry(curr_parent_dir_entry) == NULL){
		// 	// __cio_printf("PROBLEM WITH %s", dirs[i]->fi);
		// 	_fs_create_root_entry(dirs[i], DIRECTORY_ENTRY);
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


	*/
	#ifdef DEBUG
	__cio_printf("Creating (from path %s)!!!\n", path);
	__delay(STEP);
	#endif
	return 0;
}

int _fs_create_root_entry(const char *filename, EntryAttribute type) {
	#ifdef DEBUG
	__cio_printf(" Creating a root entry %s...\n", filename);
	__delay(STEP);
	#endif

	// Check if the filename is valid
    if (filename == NULL || *filename == '\0') {
		__cio_printf("ERROR: Filename %s undefined\n", filename);
        return -1;
    }

	if (filename[0] == '/'){
		__cio_printf("ERROR: Invalid filename, do not use path\n");
		return -1;
	}

	// Make sure new entry attempt is not root
	if(__strcmp(filename, "/") == 0){
		__cio_printf("ERROR: Cannot create new root!\n");
		return -1;
	}

	// Check if the filename already exists IN ROOT
	if(_fs_find_entry(filename) != NULL){
		__cio_printf("ERROR: File %s already exists in root\n", filename);
		return -1;
	}

    // Check if the root directory is full
    if (root_directory.num_files >= ROOT_DIRECTORY_ENTRIES) {
		__cio_printf("ERROR: Root directory is full!\n");
        return -1;
    }

    // Create a new directory entry
    DirectoryEntry *new_entry = _km_page_alloc(1);
    _fs_initialize_directory_entry(new_entry, filename, 0, type, 0, NULL);
	
	int result = add_sub_entry(_fs_find_entry("/"), new_entry);
	if(result != 0){
		__cio_printf("Failed to create %s as child of %s\n", new_entry->filename, "/");
		return -1;
	}

	#ifdef DEBUG
	__cio_printf(" Creating a root entry %s!!!\n", filename);
	__delay(STEP);
	#endif
    return 0; // Success
}

void _fs_initialize_directory_entry(DirectoryEntry *entry, const char *filename, uint32_t size, EntryAttribute type, uint32_t cluster, DirectoryEntry *next) {
    #ifdef DEBUG
    __cio_printf("Initializing entry %s...\n", filename);
    __delay(STEP);
    #endif
    if (entry == NULL) {
        __cio_printf("ERROR: 'entry' is NULL\n");
        return;
    }

    // Copy filename
    if (filename != NULL) {
        int i;
        for (i = 0; filename[i] != '\0' && i < MAX_FILENAME_LENGTH; i++) {
            entry->filename[i] = filename[i];
        }
        entry->filename[i] = '\0'; // Null-terminate string
    } else {
        entry->filename[0] = '\0'; // Empty filename
    }

    entry->size = size;
    entry->type = type;
    entry->cluster = cluster;
    entry->next = next;

    // Allocate memory for the subdirectory
    entry->subdirectory = _km_page_alloc(1);
    if (entry->subdirectory == NULL) {
        // Handle allocation failure
		__cio_printf("Failed to allocate for subdirectory for entry %s\n", entry->filename);
        return;
    }

    // Initialize the subdirectory
    entry->subdirectory->name[0] = '\0'; // Empty directory name
    entry->subdirectory->num_files = 0; // No files in the directory

    #ifdef DEBUG
    __cio_printf("Initializing entry %s!!!\n", filename);
    __delay(STEP);
    #endif
}

int _fs_rename_entry(const char *old_filename, const char *new_filename){
	#ifdef DEBUG
	__cio_printf("Renaming...\n");
	__delay(STEP);
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
	__delay(STEP);
	#endif

	DirectoryEntry *entry = _fs_find_entry(filename);
	if(entry == NULL){
		__cio_printf("ERROR: File %s does not exist", filename);
		return -1;
	}

	__memclr(entry, sizeof(DirectoryEntry));

	return 0;
}

int _fs_open_file(const char *filename, const char *mode){
	#ifdef DEBUG
	__cio_printf("Opening...\n");
	__delay(STEP);
	#endif

	if (mode == NULL || (mode[0] != 'r' && mode[0] != 'w')) {
		__cio_printf("ERROR: Invalid mode (%s) when opening file\n", mode);
		return -1;
	}

	DirectoryEntry *entry = _fs_find_entry(filename);
	if (entry == NULL) {
		__cio_printf("ERROR: Filename %s not found\n", filename);
		return -1;
	}
	// uint32_t size = entry->size;

	return 0;
}

int _fs_read_file(const char *filename) {
	#ifdef DEBUG
	__cio_printf("Reading...\n");
	__delay(STEP);
	#endif

	// Read data from the specified file into fs.buffer

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
		__delay(STEP);
		#endif

		read_block(current_block);
		dump_fs_buffer();
		clear_fs_buffer();

		// Move to the next cluster in the FAT
		current_block = fs.fat->entries[current_block].next_cluster;
	}
	// dump_fs_buffer();
	return 0;
}

int _fs_write_file(const char *path, const void *data) {
	#ifdef DEBUG
	__cio_printf("Writing...\n");
	__delay(STEP);
	#endif

	DirectoryEntry *entry = _fs_find_entry_from_path(path);

	// If the file doesn't exist, return an error
	if (entry == NULL) {
		__cio_printf("ERROR: Filename %s not found\n", entry);
		return -1;
	}

	// Retrieve the starting cluster of the file
	uint32_t start_block = entry->cluster;
	uint32_t current_block = start_block; 

	// Write data blocks into the file
	const uint8_t *data_ptr = (const uint8_t *)data;
	while (current_block != FAT_EOC) {
		// Calculate the block number corresponding to the current cluster
		#ifdef DEBUG
		__cio_printf("Writing to block number %d\n", current_block);
		__delay(STEP);
		#endif

		// __delay( 200);

		// Write data from the buffer to the disk image
		// write_block(block_number, data_ptr); 
		write_block(current_block, data_ptr);
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
	#ifdef DEBUG
	__cio_printf("Closing...\n");
	__delay(STEP);
	#endif

	return 0;
}

int _fs_print_children(DirectoryEntry *entry){
	int num_files = entry->subdirectory->num_files;
	if (num_files == 0){
		return 0;
	}
	#ifdef DEBUG
	__cio_printf("Printing %d children...\n", num_files);
	__delay(STEP);
	#endif
	int i;
	for(i = 0; i < num_files; i++){
		DirectoryEntry *child = entry->subdirectory->files[i];
		__cio_printf(" -> Child %d: %s\n", i+1, child->filename);
		// _fs_print_children(child);
	}
	#ifdef DEBUG
	__cio_printf("Printing %d children!!!\n", entry->subdirectory->num_files);
	__delay(STEP);
	#endif
	return num_files;
}

int _fs_print_entry(DirectoryEntry *entry, bool_t print_children){
	#ifdef DEBUG
	__cio_printf("  Printing \"%s\"...\n", entry->filename);
	__delay(STEP);
	#endif

	if(entry == NULL){
		__cio_printf("ERROR: Entry %s not found\n", entry->filename);
		return -1;
	}
	__cio_printf("This %s has filename %s, size %d, at block %d. It has %d children\n", entry->type == 1 ? "file" : "directory", entry->filename, entry->size, entry->cluster, entry->subdirectory->num_files);
	if(print_children){
		_fs_print_children(entry);
	}

	#ifdef DEBUG
	__cio_printf("  Printing \"%s\"!!!\n", entry->filename);
	__delay(STEP);
	#endif
	return 0;
}

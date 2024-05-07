/*
** File: filesystem.c
** Description: This file includes the full implementation of a filesystem.
**
** @author Sean O'Beirne
*/

#if 1  // BEG SETUP
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

Directory root_directory = {
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
	.depth = 0,
};

DeconstructedPath cwd = {
	.path = "/",
	.filename = "/",
	.num_dirs = 0,
	.path_type = ABSOLUTE,
};

DeconstructedPath nwd = { // "New" working directory
	.filename = "/",
	.path = "/",
	.num_dirs = 0,
	.path_type = ABSOLUTE,
};

#endif // END SETUP

#if 1  // BEG Handling input
void _fs_shell( int code ) {
	// __cio_printf("Shell received code %d, char %c\n", code, (char)code);
	// const char *shell_buffer = fs.disk.request_space(MAX_PATH_LENGTH); // TODO need to change
	// __memclr(shell_buffer, MAX_PATH_LENGTH);
	switch( code ) {

		case 'f':
			__cio_printf("Make file path: ");
			clear_fs_buffer();
			__cio_gets(fs.buffer, MAX_FILENAME_LENGTH + 1);
			__cio_printf("Making file at %s\n", fs.buffer);
			_fs_create_file(fs.buffer);
			break;

		case 'd':
			__cio_printf("Make directory path: ");
			clear_fs_buffer();
			__cio_gets(fs.buffer, MAX_FILENAME_LENGTH + 1);
			__cio_printf("Making directory at %s\n", fs.buffer);
			_fs_create_dir(fs.buffer);
			break;

		case 'x':
			__cio_printf("Delete entry: ");
			clear_fs_buffer();
			__cio_gets(fs.buffer, MAX_FILENAME_LENGTH + 1);
			__cio_printf("Deleting entry at %s\n", fs.buffer);
			_fs_delete_entry(fs.buffer);
			break;

		case 'l':
			nl();
			list_dir_contents(fs.cwd, false);
			break;

		case '/': // fall through
		case 'c':
			__cio_printf("New (absolute) path: ");
			clear_fs_buffer();
			__cio_gets(fs.buffer, MAX_FILENAME_LENGTH + 1);
			change_dir(fs.buffer);
			// __cio_printf("CD'ing into %s\n", fs.buffer);
			break;


		case 'r':
			nl();
			__cio_printf("Listing root:\n");
			list_dir_contents("/", false);
			break;
		
		case 't':
			dump_fat();
			break;

		// case '':
		// 	break;

		case '\r': // FALL THROUGH
		case '\n':
			break;
		
		case 'a':
			__cio_printf("Printing nwd:\n");
			print_parsed_path();
			break;
	
		default:
			__cio_printf( "shell: unknown request '0x%02x'\n", code );
			// FALL THROUGH

		case 'h':  // help message
			__cio_puts( "\nCommands:\n"
						"  	 f  -- make new file\n"
						"  	 d  -- make new directory\n"
						"  	 x  -- delete entry\n"
						"  	 l  -- list cwd contents\n"
						"  	 c  -- change directory\n"
						"  	 p  -- print entry\n"
						"  	 r  -- dump root\n"
						"  	 a  -- print last parsed path\n"
						"  	 t  -- dump FAT\n"
						// "	 f  -- \n"
						"  	 h  -- print this message\n"
						);
			break;
	}
}

#endif // END Handle input

#if 1  // BEG Print functions
// Print a header for a module with delay of 'ticks'
void phn(const char *text, int ticks) {
	__cio_printf(" - clearing - \n");
	__delay(ticks);
	__cio_clearscroll();
	__cio_moveto(0, 0);
	__cio_printf("<========= %s =========>\n", text);
	plnn();
}

// Print a header for a module
void ph(const char *text) {
	phn(text, STEP);
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

// Print an 80-character long line with '-', no delay, only if DEBUG is on
void pl_debug() {
	#ifdef DEBUG
	for(int i = 0; i < 80; i++){
		__cio_putchar('-');
	}
	#endif
}

// Force-print an 80-character long line with '-', no delay
void pln() {
	for(int i = 0; i < 80; i++){
		__cio_putchar('-');
	}
}

void plnn() {
	for(int i = 0; i < 80; i++){
		__cio_putchar('-');
	}
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
void pvl( const char *header, char line_char, int indent ){
	if(header == NULL){
		__cio_printf("ERROR: Cannot print header, header is NULL\n");
		return;
	}
	if(line_char == NULL){
		line_char = '-';
	}
	// Print header
	for(int i = 0; i < indent; i++){
		__cio_putchar(' ');
	}
	__cio_puts(header);
	if (header[__strlen(header)-1] != '\n'){
		__cio_putchar('\n');
	}
	// Print dashes
	int num_dashes = __strlen(header);
	bool_t maxxed = num_dashes >= 80 ? true : false;
	for(int i = 0; i < indent; i++){
		__cio_putchar(' ');
	}
	while(num_dashes > 0){
		__cio_putchar(line_char);
		num_dashes--;
	}
	if(!maxxed)
		__cio_putchar('\n');
}

void nl(){
	__cio_putchar('\n');
}

void print_chars(){
	for(uint8_t i = 0; i <= MAX_ASCII_CHAR; i++){
		if (10 < i && i < 14){
			continue;
		}
		if(i == 10){
			__cio_printf("%d:%s  ", i, "\\n");
		}
		else{
			__cio_printf("%d:%c  ", i, i);
		}
		if(i % 10 == 0 || i == MAX_ASCII_CHAR){
			__cio_putchar('\n');
		}
	}
}

void d(){
	__delay(100);
}

// Pass just a string, creates a box for it
void box_h(int max_item_size){
	__cio_putchar('+');
	for(int i = 1; i <= max_item_size; i++){
		__cio_putchar('-');
	}
	__cio_putchar('+');
	__cio_putchar('\n');
}

void box_v(char *to_print){
	__cio_printf("| ");
}

void box_pad_right(int longest_line){
	int i = __strlen(fs.buffer);
	int num = longest_line - __strlen(fs.buffer);
	__cio_printf("%s", fs.buffer);
	for (i = 0; i <= num; i++){
		__cio_putchar(' ');
	}
	__cio_putchar('|');
	__cio_putchar('\n');
}

#endif // END Print functions

#if 1  // BEG Dumping functions
int dump_fs_buffer( void ){
	pvl("Dumping Buffer:\n", '-', 2);
	__delay(STEP);
	__cio_printf("    %s\n", (char *)fs.buffer);
	__delay(STEP);
	return 0;
}

// print buffer
void pb( void ){
	__cio_printf("buffer: %s\n", fs.buffer);
}

// dump root
void dr(){
	int success = _fs_print_entry(&root_directory_entry);
	if(success == -1){
		__cio_printf("FAILURE, root is NULL!!!\n");
	}
}
#if 1
void dump_fat() {
    __cio_printf("FAT Dump:\n");

    // Iterate through each entry in the FAT
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        // Print the index and status of the FAT entry
        __cio_printf("Entry %d: ", i);
        switch(fs.fat->entries[i].status) {
            case FAT_FREE:
                __cio_printf("FREE");
				__delay(CSTEP);
                break;
            case FAT_IN_USE:
                __cio_printf("IN USE");
				__delay(MOMENT);
                break;
            default:
                __cio_printf("UNKNOWN");
        }

        // Print the next cluster value
        __cio_printf(", Next Cluster: ");
        if (fs.fat->entries[i].next_cluster == FAT_EOC) {
            __cio_printf("END OF CHAIN\n");
        } else {
            __cio_printf("%d\n", fs.fat->entries[i].next_cluster);
        }
		__delay(CSTEP);
    }
}
#endif

#endif // END Dumping functions

#if 1   // BEG Read / write functions
int read_block(int block_number){ // TODO SEAN chunks
	if (block_number < 0 || block_number >= DISK_SIZE) {
		__cio_printf("ERROR: Block number %d not within bounds %d - %d\n", block_number, 0, DISK_SIZE);
		return -1; // Invalid block number
	}
	// Copy data from disk image to file system buffer

	return 0;
}

int calculate_block_number(int cluster) {
    // Check if the cluster number is valid
    if (cluster < 2 || cluster >= MAX_FAT_ENTRIES) {
        __cio_printf("ERROR: Invalid cluster number\n");
        return -1;
    }

    // Calculate the block number corresponding to the cluster
	int block_number = (cluster - 2) * fs.bytes_per_cluster + fs.reserved_cluster_count * fs.bytes_per_cluster;
    return block_number;
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

#endif  // END Read / write functions

#if 1  // BEG Helper functions

void show_header_info(bool_t horrizontal){
	int len;

	len = __strlen(fs.cwd);
	__cio_puts_at(50, 0, "                             ");
	__cio_puts_at(80-len, 0, fs.cwd);

	// TODO fix these, they don't show up
	if(horrizontal == true){
		len = __strlen("9 = Directory  7 = File");
		__cio_printf_at(80-len, 1, "%c = Directory  %c = File", 9, 7);
	}
	else{
		len = __strlen("9 = Directory");
		__cio_printf_at(80-len, 1, "%c = Directory", 9);
		len = __strlen("7 = File");
		__cio_printf_at(80-len, 2, "%c = File", 7);
	}
}

void init_fs_buffer( void ) {
	fs.buffer = (char *)_km_page_alloc(1);
}

void clear_fs_buffer( void ) {
	// Fill the fs.buffer with zeros
	__memclr(fs.buffer, SZ_PAGE);
}

void clean_nwd(){
	// SAVE
	cwd.num_dirs = nwd.num_dirs;
    cwd.path_type = nwd.path_type;
    cwd.entry_type = nwd.entry_type;
    cwd.op_type = nwd.op_type;

    __memcpy(cwd.filename, nwd.filename, MAX_FILENAME_LENGTH);
    __memcpy(cwd.path, nwd.filename, MAX_PATH_LENGTH);

    for (int i = 0; i < MAX_FILENAME_LENGTH; i++) {
        if (cwd.dirs[i] != NULL) {
            __memcpy(cwd.dirs[i], nwd.dirs[i], __strlen(nwd.dirs[i]));
        }
        if (cwd.paths[i] != NULL) {
            __memcpy(cwd.paths[i], nwd.paths[i], __strlen(nwd.paths[i]));
        }
    }

	// CLEAN

	// Reset all fields of nwd
	nwd.num_dirs = 0;
    nwd.path_type = ABSOLUTE;
    nwd.entry_type = DIRECTORY;
    nwd.op_type = FIND;

    // Clear the filename and path arrays
    __memclr(nwd.filename, MAX_FILENAME_LENGTH);
    __memclr(nwd.path, MAX_PATH_LENGTH);

    // Clear the dirs and paths arrays
    for (int i = 0; i < MAX_FILENAME_LENGTH; i++) {
        if (nwd.dirs[i] != NULL) {
            __memclr(nwd.dirs[i], __strlen(nwd.dirs[i]));
            nwd.dirs[i] = NULL;
        }
        if (nwd.paths[i] != NULL) {
            __memclr(nwd.paths[i], __strlen(nwd.paths[i]));
            nwd.paths[i] = NULL;
        }
    }
}

void parse_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("Deconstructing path %s...\n", path);
	__delay(STEP);
	#endif

    char scratch_buffer[MAX_PATH_LENGTH];
	
	if (path == NULL || path[0] == '\0') {
		__cio_printf("ERROR: Path is NULL\n");
		return;
	}

	if(path[0] != '/'){
		__cio_printf("**WARNING**: Relative paths not fully supported!\n");
	}

	// root case
	if(__strcmp(path, "/") == 0){
		nwd.filename[0] = '/';
		nwd.path[0] = '/';
		nwd.num_dirs = 1;
		nwd.dirs[0] = "/";
		nwd.paths[0] = "/";
		nwd.path_type = ABSOLUTE;
		nwd.entry_type = DIRECTORY;
		#ifdef DEBUG
		__cio_printf("Deconstructing path %s!!!\n", path);
		__delay(STEP);
		#endif
		return;
	}
	else{
        nwd.filename[0] = '\0';
        nwd.num_dirs = 0;
        int pt = (path[0] == '/' ? ABSOLUTE : RELATIVE);
        // Don't clear scratch_buffer, assume it's initialized properly
        if(pt == RELATIVE){
            __strcpy(scratch_buffer, cwd.path); // Use scratch_buffer directly
            __strcat(scratch_buffer, path);
        }
        else{
            __strcpy(scratch_buffer, path);
            nwd.num_dirs = 1; // count root
        }
        nwd.path_type = pt;
        __strcpy(nwd.path, scratch_buffer); // Use scratch_buffer directly
    }


    // Other local variables
    char scratch_path[MAX_PATH_LENGTH];
    char filename[MAX_FILENAME_LENGTH];
	for(int i = 0; i < MAX_PATH_LENGTH; i++){
		if (i < MAX_FILENAME_LENGTH){
			filename[i] = '\0';
		}
		scratch_path[i] = '\0';
	}
    int path_i = 0;
	int scratch_i = 0;
    int filename_i = 0;
    int dir_names_i = 0;
    // int dir_name_i = 0;

	while(path_i < __strlen(path)){
		filename[filename_i] = path[path_i];
		scratch_path[scratch_i] = path[path_i];

		if(path[path_i] == '/'){
			// __cio_printf("printing paths: %s, sphere: %c\n", scratch_path, scratch_path[scratch_i]);
			if(path_i == 0){ // be cautious about '../'
				filename[filename_i] = '/';
				filename[filename_i + 1] = '\0';
				scratch_path[scratch_i] = '/';
			} else{
				filename[filename_i] = '\0';
				scratch_path[scratch_i] = '\0';
			}

			// export filename and path
			// nwd->dirs[dir_names_i] = fs.disk.request_space(sizeof(nwd->dirs));
			nwd.dirs[dir_names_i] = fs.disk.request_space(sizeof(nwd.dirs));
            __strcpy(nwd.dirs[dir_names_i], filename);
            // nwd->paths[dir_names_i] = fs.disk.request_space(sizeof(nwd->paths));
            nwd.paths[dir_names_i] = fs.disk.request_space(sizeof(nwd.paths));
            __strcpy(nwd.paths[dir_names_i], scratch_path);
			
            dir_names_i++;

			scratch_path[scratch_i] = path[path_i];
			// __cio_printf("printing paths: %s, sphere: %c\n", scratch_path, scratch_path[scratch_i]);
			__memclr(filename, filename_i);
			filename_i = -1;
		}

		scratch_i++;
		filename_i++;
		path_i++;
		// __cio_printf("iterating...fn:%s  sp:%s\n        ps:%s, ds:%s\n", filename, scratch_path, path, nwd->dirs[dir_names_i]);
		// __delay(25);
	}
	// __cio_printf("This step, PRE NULL filename=%s and scratch_path=%s\n", filename, scratch_path);
	if(path[path_i] == '\0'){

		filename[filename_i] = '\0';
		scratch_path[scratch_i] = '\0';

		// __cio_printf("sizeof nwd->dirs: %d, %d\n", sizeof(nwd->dirs), nwd->dirs);
		// __cio_printf("sizeof nwd->dirs[0]: %d\n", sizeof(nwd->dirs[0]));
		// plnn();
		// nwd->dirs[dir_names_i] = fs.disk.request_space();
		// nwd->paths[dir_names_i] = fs.disk.request_space();
		nwd.paths[dir_names_i] = _km_page_alloc(1);
		nwd.dirs[dir_names_i] = _km_page_alloc(1);
		__strcpy(nwd.filename, filename);
		__strcpy(nwd.dirs[dir_names_i], filename);
		__strcpy(nwd.paths[dir_names_i], scratch_path);
		nwd.num_dirs = dir_names_i + 1;
		__memclr(filename, filename_i);
	}

	#ifdef DEBUG
	__cio_printf("Deconstructing path %s!!!\n", path);
	__delay(STEP);
	#endif
}

void test_parse_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("Testing parse_path()...\n");
	__delay(STEP);
	#endif
	
    parse_path(path);

	__cio_printf("Path: %s\n", nwd.path);
	__cio_printf("Number of directories: %d\n", nwd.num_dirs);
    __cio_printf("File name: %s\n", nwd.filename);
    for (int i = 0; i < nwd.num_dirs; i++) {
        __cio_printf("Directory %d: %s\n", i + 1, nwd.dirs[i]);
    }
    for (int i = 0; i < nwd.num_dirs; i++) {
        __cio_printf("PATH %d: %s\n", i + 1, nwd.paths[i]);
    }
	#ifdef DEBUG
	__cio_printf("Testing parse_path()!!!\n");
	__delay(STEP);
	#endif
}

void print_parsed_path() {
	#ifdef DEBUG
	__cio_printf("Printing parsed path...\n");
	__delay(STEP);
	#endif
	if(nwd.num_dirs == 0){ // 'nwd' should ALWAYS contain at least the root
		__cio_printf("ERROR: DeconstructedPath nwd is NULL\n");
		#ifdef DEBUG
		__cio_printf("Printing parsed path---\n");
		__delay(STEP);
		#endif
		return;
	}
	__cio_printf("  Path: %s\n", nwd.path);
	__cio_printf("  Number of directories: %d\n", nwd.num_dirs);
    __cio_printf("  File name: %s\n", nwd.filename);
    for (int i = 0; i < nwd.num_dirs; i++) {
        __cio_printf("  Directory %d: %s\n", i + 1, nwd.dirs[i]);
    }
    for (int i = 0; i < nwd.num_dirs; i++) {
        __cio_printf("  PATH %d: %s\n", i + 1, nwd.paths[i]);
    }
	__cio_printf("  Entry type: %s\n", nwd.entry_type == 1 ? "file" : "directory");
	__cio_printf("  Path type: %s\n", nwd.path_type == 1 ? "absolute" : "relative");
	__cio_printf("  Operation type: %s\n", nwd.op_type == 1 ? "find" : "create");
	#ifdef DEBUG
	__cio_printf("Printing parsed path!!!\n");
	__delay(STEP);
	#endif
}

int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert){
	#ifdef DEBUG
	__cio_printf("Adding sub entry %s to %s...\n", insert->filename, dest->filename);
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
	if(dest->type == FILE){
		__cio_printf("ERROR: Adding DirectoryEntry %s to a file, %s, aborting\n", insert->filename, dest->filename);
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
	__cio_printf("Adding sub entry %s to %s!!!\n", insert->filename, dest->filename);
	__delay(STEP);
	#endif
	return 0;
}
#if 1
DirectoryEntry *create_sub_entry(DirectoryEntry *parent, const char *filename, EntryType type) {
    // Create the entry in the parent directory
    // You already have this part implemented

	DirectoryEntry *child = fs.disk.request_space(sizeof(DirectoryEntry));
    // Allocate a new cluster in the FAT for the entry
    uint16_t new_cluster = allocate_cluster();

	_fs_initialize_directory_entry(child, filename, 0, type, new_cluster, NULL, parent->depth+1, nwd.paths[parent->depth+1]);

	
	int result = add_sub_entry(parent, child);
	if(result != 0){
		__cio_printf("ERROR: Failed to create %s as child of %s\n", child->filename, parent->filename);
		#ifdef DEBUG
		__cio_printf("Creating %s, a sub entry of %s---\n", filename, parent->filename);
		__delay(STEP);
		#endif
		return NULL;
	}



    // Set the cluster number in the DirectoryEntry
    child->cluster = new_cluster;

    // Update the FAT
    update_fat_entry(new_cluster, FAT_EOC);
	__cio_printf("New cluster: %d\n", new_cluster);

    return child;
}
#endif

DirectoryEntry *get_sub_entry(DirectoryEntry *parent, const char *filename){
	#ifdef DEBUG
	__cio_printf("Getting sub_entry of %s, should be %s...\n", parent->filename, filename);
	__delay(STEP);
	#endif

	DirectoryEntry *ret = NULL;
	for(int i = 0; i < parent->subdirectory->num_files; i++){
		if(__strcmp(parent->subdirectory->files[i]->filename, filename) == 0){
			ret = parent->subdirectory->files[i];
			// __cio_printf("I guess %s == %s\n", ret->filename, filename);
			break;
		}
	}

	#ifdef DEBUG
	__cio_printf("Getting sub_entry of %s, should be %s!!!\n", parent->filename, filename);
	__delay(STEP);
	#endif
	
	return ret;
}

#if 1
// Function to add a new entry to the FAT
int add_fat_entry(uint32_t next_cluster) {
    for (int i = 0; i < MAX_FAT_ENTRIES; i++) {
        if (fs.fat->entries[i].status == FAT_FREE) {
            // Check if the next_cluster value is already in use
            for (int j = 0; j < MAX_FAT_ENTRIES; j++) {
                if (fs.fat->entries[j].next_cluster == next_cluster && fs.fat->entries[j].status != FAT_FREE) {
                    // Provided next_cluster value is already in use
                    __cio_printf("ERROR: Cluster %d is already in use\n", next_cluster);
                    return -1;
                }
            }
            
            // Set the next_cluster field to the provided value
            fs.fat->entries[i].next_cluster = next_cluster;
            
            // Update the status field to indicate that the entry is in use
            fs.fat->entries[i].status = FAT_IN_USE;
            
            // Return the index of the added entry
            return i;
        }
    }
    
    // No open entry found in FAT
    __cio_printf("ERROR: No open entry found in FAT\n");
    return -1;
}
#endif


DirectoryEntry *find_or_create_entry_recursive(DirectoryEntry *current_dir, int depth) {
	#ifdef DEBUG
	__cio_printf("Recursing on path %s...\n", nwd.path);
	__delay(STEP);
	#endif
	if (current_dir == NULL){
		__cio_printf("ERROR: Current directory is null\n");
	}
	if(depth < 0 || depth > nwd.num_dirs){
		__cio_printf("ERROR: Invalid depth %d\n", depth);
	}
	// __cio_printf("We are now iterating over %s recursively\n", current_dir->filename);
	// __cio_printf("Clear as day:\n  %s\n  %s\n", current_dir->filename, nwd.dirs[depth]);

	EntryType entry_type = (nwd.entry_type == FILE && depth == nwd.num_dirs - 1) ? FILE : DIRECTORY;


	DirectoryEntry *entry = NULL;
	entry = get_sub_entry(current_dir, nwd.dirs[depth]);
	// _fs_print_entry(entry);

	// If the entry doesn't exist, create it
	if (entry == NULL){
		if(nwd.op_type == CREATE) {
			// Create the entry based on whether it's a directory or a file
			entry = create_sub_entry(current_dir, nwd.dirs[depth], entry_type);
			// _fs_print_entry(entry);

			// If the entry couldn't be created, return NULL
			if (entry == NULL) {
				__cio_printf("ERROR: Failed to create sub entry of %s\n", current_dir->filename);
				return NULL;
			}
		}
		else if(nwd.op_type == FIND){
			return NULL;
		}
    }
	else if(nwd.op_type == FIND && __strcmp(entry->filename, nwd.filename) == 0){
		return entry;
	}
    

    // If there are more components in the path, recursively traverse
    if (nwd.num_dirs > depth+1) {
        return find_or_create_entry_recursive(entry, depth + 1);
    }

	#ifdef DEBUG
	__cio_printf("Recursing on path %s!!!\n", nwd.path);
	__delay(STEP);
	#endif

    return entry;
}

DirectoryEntry *find_or_create_entry() {
	#ifdef DEBUG
	__cio_printf("Finding/Creating path %s...\n", nwd.path);
	__delay(STEP);
	#endif

    // Start the traversal from the root directory
	int offset = nwd.path_type == ABSOLUTE ? 1 : 0; // skip root if absolute path
    DirectoryEntry *entry = find_or_create_entry_recursive(&root_directory_entry, offset);
	
	#ifdef DEBUG
	__cio_printf("Finding/Creating path %s!!!\n", nwd.path);
	__delay(STEP);
	#endif
	return entry;
}

int get_next_cluster(int current_cluster) {
    // Check if the current cluster is valid
    if (current_cluster < fs.reserved_cluster_count || current_cluster >= DISK_SIZE) {
        __cio_printf("ERROR: Invalid current cluster number\n");
        return -1;
    }

    // Get the next cluster number from the FAT table
    int next_cluster = fs.fat->entries[current_cluster].next_cluster;

    // Check if the next cluster number is valid
    if (next_cluster >= fs.reserved_cluster_count && next_cluster < FAT_EOC) {
        return next_cluster;
    } else {
        return -1; // End of file reached
    }
}

// Function to allocate a new cluster in the FAT
uint16_t allocate_cluster() {
    // Iterate through the FAT to find a free cluster
    for (uint16_t cluster = fs.reserved_cluster_count; cluster < MAX_FAT_ENTRIES; cluster++) {
        // Check if the cluster is free (marked as FAT_FREE)
        if (fs.fat->entries[cluster].status == FAT_FREE) {
            // Mark the cluster as in use
            fs.fat->entries[cluster].status = FAT_IN_USE;
            return cluster;
        }
    }
    // If no free cluster is found, return an error value
    return FAT_EOC; // FAT_EOC indicates error here
}

// Function to update the FAT entry for a given cluster
void update_fat_entry(uint16_t cluster, uint16_t value) {
    // Update the next_cluster field of the FAT entry for the specified cluster
    fs.fat->entries[cluster].next_cluster = value;
}

#endif // END Helper functions

#if 1  // BEG Directory manipulation
//////////////////////////
// DIRECTORY FUNCTIONS  //
//////////////////////////
int dir_contains(DirectoryEntry *parent, const char *target){
	for(int i = 0; i < parent->subdirectory->num_files; i++){
		if(__strcmp(parent->subdirectory->files[i]->filename, target) == 0){
			return 0;
		}
	}
	return -1;
}

int cd_parent() {
	int result = -1;
    // Check if the current working directory is already root
    if (__strcmp(fs.cwd, "/") == 0) {
		__cio_printf("ERROR: Already at root, cannot go up\n");
        return result;
    }

    uint32_t len = __strlen(fs.cwd);
	// __cio_printf("This len = %d\n", len);

    // Find the last occurrence of '/' character
    int i;
    for (i = len - 1; i >= 0; i--) {
        if (fs.cwd[i] == '/') {
            break;
        }
    }
    // Terminate and clear the rest of the string's memory
	for(int j = i; j < len; j++){
		if(j == 0) continue; // do not remove root from path
		fs.cwd[j] = '\0';
	}

	return result;
}

int change_dir(const char *path){
	#ifdef DEBUG
	__cio_printf("CD'ing to %s...\n", path);
	__delay(STEP);
	#endif
	
	if(__strcmp(path, "/") == 0){
		__memclr(fs.cwd, MAX_PATH_LENGTH);
		__strcpy(fs.cwd, "/");
		show_header_info(true);
		return 0;
	}

	DirectoryEntry *new_entry = _fs_find_entry(path);
	if(new_entry == NULL){
		__cio_printf("ERROR: Unable to find directory at path %s\n", path);
		return -1;
	}

	if(nwd.path_type == ABSOLUTE){
		__memclr(fs.cwd, MAX_PATH_LENGTH);
		__strcpy(fs.cwd, new_entry->path);
	}
	else if(nwd.path_type == RELATIVE){
		// TODO: Make relative path navigations work
		__strcat(fs.cwd, nwd.path);
		_fs_print_entry(_fs_find_entry(fs.cwd));
	}
	
	show_header_info(true);

	#ifdef DEBUG
	__cio_printf("CD'ing to %s!!!\n", path);
	__delay(STEP);
	#endif
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

int list_dir_contents(const char *path, bool_t box) { //TODO make this more line print_entry()
	#ifdef DEBUG
	__cio_printf("Listing %s...\n", path);
	__delay(STEP);
	#endif

	// if(path == NULL){
	// 	_fs_print_entry()
	// }
	// fs.cwd = (path[0] == '/' ? sprint(fs.cwd, "%s/%s", fs.cwd, path : fs.cwd));
	// if (path[0] != '/'){
	// 	sprint(fs.cwd, "%s/%s", fs.cwd, path);
	// }
	DirectoryEntry *entry = _fs_find_entry(path);
	if (entry == NULL){
		__cio_printf("ERROR: Finding %s returned NULL value\n", path);
		return -1;
	}
	_fs_print_entry(entry);
	return 0;

	Directory *dir = (Directory *)entry->subdirectory;
	if (dir == NULL){
		__cio_printf("ERROR: Failed to find valid Directory at %s\n", path);
		return -1;
	}

	if (entry->type == FILE){
		__cio_printf("ERROR: %s is a *file*; cannot list directory contents", entry->filename);
		return -1;
	}

	if(box){
		__sprint(fs.buffer, "| dir: %s", path);
		int len = __strlen(fs.buffer);
		int longest_line = __strlen(fs.buffer);
		for (int i = 0; i < dir->num_files; i++) {
			len = __strlen(dir->files[i]->filename);
			if (len > longest_line){
				longest_line = len;
			}
			// __cio_printf("|    %c %s |\n", entry->type == 1 ? FILLED_CIRCLE : OPEN_CIRCLE, dir->files[i]->filename); //TODO SEAN expand the info here
		}	
		box_h(longest_line);
		box_pad_right(longest_line);
		box_h(longest_line);
		clear_fs_buffer();
		for (int i = 0; i < dir->num_files; i++) {
			__sprint(fs.buffer, "| %s", dir->files[i]->filename); //TODO SEAN expand the info here
			box_pad_right(longest_line);
			// __cio_printf("|    %c %s |\n", entry->type == 1 ? FILLED_CIRCLE : OPEN_CIRCLE, dir->files[i]->filename); //TODO SEAN expand the info here
			clear_fs_buffer();
		}
		box_h(longest_line);
	} else{
		clear_fs_buffer();
		__sprint(fs.buffer, "Directory: %s\n", entry->filename);
		pb();
		for(int i = 0; i < entry->depth; i++){
			__cio_putchar(' ');
		}
		for(int i = 0; i < entry->subdirectory->num_files; i++){
			clear_fs_buffer();
			__sprint(fs.buffer, "  %c %s\n", entry->type == FILE ? FILLED_CIRCLE : OPEN_CIRCLE, dir->files[i]->filename); //TODO SEAN expand the info here
			pb();
		}
		__cio_putchar('\n');

		// pvl(fs.buffer, '-', 1); // do this or the line below
		// __cio_printf("%s\n", header);
	}
	
}
#endif // END Directory manipulation


#if 1  // BEG Filesystem functions
//////////////////////////
// FILESYSTEM FUNCTIONS //
//////////////////////////
int _fs_init( void ) {
	#ifdef DEBUG
	__cio_printf("Initializing Filesystem\n");
	__delay(STEP);
	#endif

	// Implement FAT32 initialization logic here
	// Initialize the filesystem structure and perform any necessary setup

	// Initialize the file system struct with appropriate values
	fs.bytes_per_cluster = SZ_PAGE;
	fs.reserved_cluster_count = 4;
	fs.total_clusters = MAX_FAT_ENTRIES;

	// Initialize StorageInterface disk
    StorageInterface disk;
    int result = _storage_init(&disk);
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
    fs.fat = (FAT *)disk.request_space(fs.bytes_per_cluster * fs.total_clusters);
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

	// cwd
	fs.cwd[0] = '/';

	// Mount status
	fs.mounted = false;

	// TODO: make this help text persist
	// clear_fs_buffer();
	// sprint(fs.buffer, "%c = Directory  %c = File", 9, 7);
	// int len = __strlen(fs.buffer);
	// __cio_puts_at(80-len, 1, fs.buffer);

	#ifdef DEBUG
	__cio_printf("Filesystem has been initialized\n");
	__delay(STEP);
	#endif

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

bool_t is_root(const char *path){
	return (path[0] == '/' && path[1] == '\0');
}

DirectoryEntry *_fs_find_entry(const char *path){
	#ifdef DEBUG
	__cio_printf("Finding entry at %s...\n", path);
	__delay(STEP);
	#endif
	
	if(is_root(path)){
		return &root_directory_entry;
	}
	// set up 'nwd' helper
	clean_nwd();
	parse_path(path);
	nwd.op_type = FIND;

	DirectoryEntry *entry = find_or_create_entry();
	// _fs_print_entry(entry);
	
	#ifdef DEBUG
	__cio_printf("Finding entry at %s!!!\n", path);
	__delay(STEP);
	#endif
	
	return entry;
}

// TODO handle relative paths
// TODO Parameter error validation

DirectoryEntry *_fs_create_file(const char *path){
	#ifdef DEBUG
	__cio_printf("Creating file at %s...\n", path);
	__delay(STEP);
	#endif

	clean_nwd();
	// set up 'nwd' helper
	parse_path(path);
	nwd.entry_type = FILE;
	nwd.op_type = CREATE;

	DirectoryEntry *entry = find_or_create_entry();
	// _fs_print_entry(entry);

	#ifdef DEBUG
	__cio_printf("Creating file at %s!!!\n", path);
	__delay(STEP);
	#endif

	return entry;
}

DirectoryEntry *_fs_create_dir(const char *path){
	#ifdef DEBUG
	__cio_printf("Creating dir at %s...\n", path);
	__delay(STEP);
	#endif

	// set up 'nwd' helper
	clean_nwd();
	parse_path(path);
	nwd.entry_type = DIRECTORY;
	nwd.op_type = CREATE;

	DirectoryEntry *entry = find_or_create_entry();
	// _fs_print_entry(entry);
	
	#ifdef DEBUG
	__cio_printf("Creating dir at %s!!!\n", path);
	__delay(STEP);
	#endif

	return entry;
}

void _fs_initialize_directory_entry(DirectoryEntry *entry, const char *filename, uint32_t size, EntryType type, uint32_t cluster, DirectoryEntry *next, uint8_t depth, const char *path) {
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
	entry->depth = depth;
	if(path != NULL){
		__strcpy(entry->path, path);
	}
    // Allocate memory for the subdirectory
    // entry->subdirectory = _km_page_alloc(1);
    entry->subdirectory = fs.disk.request_space(sizeof(entry->subdirectory));
    if (entry->subdirectory == NULL) {
        // Handle allocation failure
		__cio_printf("Failed to allocate for subdirectory for entry %s\n", entry->filename);
        return;
    }

    // Initialize the subdirectory
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
		__cio_printf("ERROR: File %s does not exist\n", filename);
		return -1;
	}
	// __cio_printf("Here we are deleting an entry of filename %s, entry = %d", filename, (entry->subdirectory->num_files * sizeof(Directory)));
	for(int i = 0; i < entry->subdirectory->num_files; i++){
		__memclr(entry->subdirectory->files[i], sizeof(DirectoryEntry));
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
#if 1
int _fs_read_file(const char *filename) {
	#ifdef DEBUG
	__cio_printf("Reading...\n");
	__delay(STEP);
	#endif

	// Search for the file in the directory structure
	DirectoryEntry *entry = _fs_find_entry(filename);
	if (entry == NULL) {
		__cio_printf("ERROR: Filename %s not found!\n", filename);
		return -1;
	}

	// Start reading data blocks from the file's clusters
	int current_cluster = entry->cluster;
	while (current_cluster != FAT_EOC) {
		// Calculate the block number corresponding to the current cluster
		uint32_t block_number = calculate_block_number(current_cluster);

		// Read the block from the disk image into the buffer
		#ifdef DEBUG
		__cio_printf("Reading block %d\n", block_number);
		__delay(STEP);
		#endif
		read_block(block_number);

		// Move to the next cluster in the FAT
		current_cluster = get_next_cluster(current_cluster);
	}

	return 0;
}

#endif
#if 0
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
#endif

int _fs_write_file(const char *path, const void *data) {
	#ifdef DEBUG
	__cio_printf("Writing...\n");
	__delay(STEP);
	#endif

	DirectoryEntry *entry = _fs_find_entry(path);

	// If the file doesn't exist, return an error
	if (entry == NULL) {
		__cio_printf("ERROR: File %s not found\n", path);
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
	// #ifdef DEBUG
	// __cio_printf(" Printing %d children...\n", num_files);
	// __delay(STEP);
	// #endif
	// __cio_printf("Children...\n");
	for(int i = 0; i < num_files; i++){
		DirectoryEntry *child = entry->subdirectory->files[i];
		
		for(int i = 0; i < ((entry->depth)) + 2; i++){
			__cio_putchar(' ');
			// __cio_putchar(' ');
		}
		__cio_printf("%c %s", child->type == 2 ? OPEN_CIRCLE : FILLED_CIRCLE, child->filename);
		if(child->type == FILE){
			__cio_printf(" (%d bytes)");
		}
		__cio_putchar('\n');
		_fs_print_children(child);
	}
	// #ifdef DEBUG
	// __cio_printf("Printing %d children!!!\n", entry->subdirectory->num_files);
	// __delay(STEP);
	// #endif
	return num_files;
}

int _fs_print_entry(DirectoryEntry *entry){
	// #ifdef DEBUG
	// __cio_printf("  Printing \"%s\"...\n", entry->filename);
	// __delay(STEP);
	// #endif

	if(entry == NULL){
		__cio_printf("ERROR: Could not print %s, entry is NULL\n", entry->filename);
		return -1;
	}

	if(entry->type == DIRECTORY){
		clear_fs_buffer();
		__strcat(fs.buffer, "Directory: ");
		__strcat(fs.buffer, entry->filename);
		pvl(fs.buffer, '-', 2);
		_fs_print_children(entry);
	}
	else if(entry->type == FILE){
		clear_fs_buffer();
		__strcat(fs.buffer, "Found file: ");
		__strcat(fs.buffer, entry->filename);
		__cio_printf("  %s\n", fs.buffer);
		__cio_printf("    Path: %s\n", entry->path);
		__cio_printf("    Size: %d\n", entry->size);
		__cio_printf("    Depth: %d\n", entry->depth);
	}

	// #ifdef DEBUG
	// __cio_printf("  Printing \"%s\"!!!\n", entry->filename);
	// __delay(STEP);
	// #endif
	return 0;
}

#endif // END Filesystem implementation

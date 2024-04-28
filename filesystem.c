#if 1  // SETUP
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
	.curr = 0,
	.filename = "/",
	.num_dirs = 0,
	.path_type = ABSOLUTE_PATH,
};

DeconstructedPath nwd = { // "New" working directory
	.path = "/",
	.curr = 0,
	.filename = "/",
	.num_dirs = 0,
	.path_type = ABSOLUTE_PATH,
};

#endif // SETUP

#if 1  // BEG Handling input
//////////////////////
// HELPER FUNCTIONS //
//////////////////////
void get_path( void ){
	#ifdef DEBUG
	__cio_printf("Getting path...\n");
	__delay(STEP);
	#endif
	
	int in_len = 2;
	while(in_len > 1){
		int start = __strlen(fs.cwd) + 1;
		__cio_printf("%s: ", fs.cwd);
		clear_fs_buffer();
		in_len = __cio_gets(fs.buffer, 80 - start);
		parse_input(in_len);
	}

	// dump_fs_buffer();
	// dr();

	#ifdef DEBUG
	__cio_printf("Getting path!!!\n");
	__delay(STEP);
	#endif
}

void parse_input(int in_len){
	char command_buffer[MAX_FILENAME_LENGTH];
	int buffer_i = 0;
	char *args[MAX_FILENAME_LENGTH+1];
	for (int i = 0; i < MAX_ARGS; i++) {
		args[i] = fs.disk.request_space(MAX_FILENAME_LENGTH);
	}
	// char **words = fs.disk.request_space(MAX_FILENAME_LENGTH);
	int args_i = 0;

	for (int i = 0; i < in_len; i++) {
        if (fs.buffer[i] == ' ' || fs.buffer[i] == '\n') {
			command_buffer[buffer_i++] = '\0';
			buffer_i = 0;
			__strcpy(args[args_i++], command_buffer);
			__memclr(command_buffer, MAX_INPUT);
        }
		else{
			command_buffer[buffer_i++] = fs.buffer[i];
		}
    }
	command_buffer[buffer_i] = '\0'; // Null-terminate the last word
    __memcpy(args[args_i], command_buffer, MAX_INPUT); // Copy the last word to words array
	run_command(args, args_i);

}

void run_command(char **args, int arg_count){
	if(__strcmp("pwd", args[0]) == 0){
		pwd();
	}
	else if(__strcmp("cd", args[0]) == 0){
		change_dir(args[1]);
	}
	else if(__strcmp("mkdir", args[0]) == 0){
		__cio_printf("Making dir!\n");
	}
	else if(__strcmp("print", args[0]) == 0){
		__sprint(fs.buffer, "/%s%s", fs.cwd, args[1]);
		dump_fs_buffer();
		// __delay(100);
		_fs_print_entry(_fs_find_entry_from_path(args[1]), false);
		// __delay(100);
	}		
}
#endif // END Handle input

#if 1  // BEG Print functions
// Print a header for a module with no delay
void phn(const char *text, int ticks) {
	__cio_printf(" - clearing - \n");
	__delay(ticks);
	__cio_clearscroll();
	__cio_moveto(0, 0);
	__cio_printf("<========= %s =========>\n", text);
	pln();
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
	pvl("Dumping Buffer:\n", '-', 0);
	__delay(STEP);
	__cio_printf(" %s\n", (char *)fs.buffer);
	__delay(STEP);
	return 0;
}

// print buffer
void pb( void ){
	__cio_printf("buffer: %s\n", fs.buffer);
}

// dump root
void dr(){
	int success = _fs_print_entry(&root_directory_entry, true);
	if(success == -1){
		__cio_printf("What the fuck, root is NULL!!!\n");
	// _fs_print_entry(_fs_find_root_entry("/"), true);
	}
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

#endif  // END Read / write functions

#if 1  // BEG Helper functions
void adjust_cwd( DeconstructedPath *cwd, const char *path ){
	__cio_printf("ADJUSTING.............\n");
	parse_path(path);
	print_parsed_path(*cwd);
}

// Print the header for the whole filesystem application.
void show_header_info(bool_t horrizontal){
	int len;

	len = __strlen(fs.cwd);
	__cio_puts_at(80-len, 0, fs.cwd);

	if(horrizontal){
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
	__memclr(fs.buffer, BLOCK_SIZE);
}

void merge_path(char *path) {
	#ifdef DEBUG
	__cio_printf("Merging paths, cwd %s, and path %s...\n", fs.cwd, path);
	__delay(STEP);
	#endif

	// pwd();
	int result;
	DeconstructedPath dp;
	parse_path(path);
	__cio_printf("Tryna get %s%s\n", fs.cwd, dp.path);
	int i = 0;
	while(strcmp(dp.dirs[i++], "..") == 0){
		result = cd_parent();
		if(result != 0){
			__cio_printf("Failed to move up a diretory\n");
		}
	}
	__cio_printf("%s\n", fs.cwd_dp);
}

void parse_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("Deconstructing path %s...\n", path);
	__delay(STEP);
	#endif

	// root case
	if(__strcmp(path, "/") == 0){
		nwd.filename[0] = '/';
		nwd.path[0] = '/';
		nwd.num_dirs = 1;
		nwd.dirs[0] = "/";
		nwd.paths[0] = "/";
		nwd.path_type = ABSOLUTE_PATH;
		nwd.curr = 0;
		#ifdef DEBUG
		__cio_printf("Deconstructing path %s!!!\n", path);
		__delay(STEP);
		#endif
		return;
	}
	else{
		if (path == NULL || *path == '\0') {
			__cio_printf("ERROR: Path is NULL\n");
			return;
		}
		nwd.filename[0] = '\0';
		nwd.num_dirs = 0;
		clear_fs_buffer();
		int pt = path[0] == '/' ? ABSOLUTE_PATH : RELATIVE_PATH;
		if(pt == RELATIVE_PATH){
			__strcpy(fs.buffer, cwd.path);
			__strcat(fs.buffer, path);
		}
		else{
			__strcpy(fs.buffer, path);
			nwd.num_dirs = 1; // count root
		}
		nwd.path_type = pt;
		__strcpy(nwd.path, fs.buffer);
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
    int dir_name_i = 0;

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
			// dp->dirs[dir_names_i] = fs.disk.request_space(sizeof(dp->dirs));
			nwd.dirs[dir_names_i] = fs.disk.request_space(sizeof(nwd.dirs));
            __strcpy(nwd.dirs[dir_names_i], filename);
            // dp->paths[dir_names_i] = fs.disk.request_space(sizeof(dp->paths));
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
		// __cio_printf("iterating...fn:%s  sp:%s\n        ps:%s, ds:%s\n", filename, scratch_path, path, dp->dirs[dir_names_i]);
		// __delay(25);
	}
	// __cio_printf("This step, PRE NULL filename=%s and scratch_path=%s\n", filename, scratch_path);
	if(path[path_i] == '\0'){

		filename[filename_i] = '\0';
		scratch_path[scratch_i] = '\0';

		// __cio_printf("sizeof dp->dirs: %d, %d\n", sizeof(dp->dirs), dp->dirs);
		// __cio_printf("sizeof dp->dirs[0]: %d\n", sizeof(dp->dirs[0]));
		// pln();
		// dp->dirs[dir_names_i] = fs.disk.request_space();
		// dp->paths[dir_names_i] = fs.disk.request_space();
		nwd.paths[dir_names_i] = _km_page_alloc(1);
		nwd.dirs[dir_names_i] = _km_page_alloc(1);
		__strcpy(nwd.filename, filename);
		__strcpy(nwd.dirs[dir_names_i], filename);
		__strcpy(nwd.paths[dir_names_i], scratch_path);
		nwd.num_dirs = dir_names_i + 1;
		nwd.curr = 0;
		__memclr(filename, filename_i);
	}

	#ifdef DEBUG
	__cio_printf("Deconstructing path %s!!!\n", path);
	__delay(STEP);
	#endif
	pln();
	__delay(20);
	print_parsed_path(nwd);
	__delay(200);
}

void test_parse_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("Testing parse_path()...\n");
	__delay(STEP);
	#endif
	
	DeconstructedPath dp;
    parse_path(path);

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
	__cio_printf("Printing parsed path...\n");
	__delay(STEP);
	#endif
	if(dp.num_dirs == 0){ // 'dp' should ALWAYS contain at least the root
		__cio_printf("ERROR: DeconstructedPath dp is NULL\n");
		#ifdef DEBUG
		__cio_printf("Printing parsed path---\n");
		__delay(STEP);
		#endif
		return;
	}
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
	__cio_printf("Printing parsed path!!!\n");
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

int create_sub_entry(DirectoryEntry *parent, const char *filename, EntryType type) {
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

	_fs_initialize_directory_entry(child, filename, 0, type, 0, NULL, parent->depth+1);

	
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
#endif // END Helper functions

#if 1  // BEG Directory manipulation
//////////////////////////
// DIRECTORY FUNCTIONS  //
//////////////////////////
int dir_contains(DirectoryEntry *parent, const char *target){
	int subdir_count = get_subdirectory_count(parent);
	for(int i = 0; i < subdir_count; i++){
		if(parent->subdirectory->files[i]->filename == target){
			return 0;
		}
	}
	return -1;
}

void pwd(){
	__cio_printf("%s\n", fs.cwd);
}

const char *get_current_dir(){
	return fs.cwd;
}

int cd_parent() {
	int result = -1;
    // Check if the current working directory is already root
    if (__strcmp(fs.cwd, "/") == 0) {
		__cio_printf("ERROR: Already at root, cannot go up\n");
        return result;
    }

    uint32_t len = __strlen(fs.cwd);
	__cio_printf("This len = %d\n", len);

    // Find the last occurrence of '/' character
    int i;
    for (i = len - 1; i >= 0; i--) {
        if (fs.cwd[i] == '/') {
            break;
        }
    }
    // Terminate and clear the rest of the string's memory
	for(int j = i + 1; j < len; j++){
		fs.cwd[j] = '\0';
	}

	pln();
	__cio_printf("idk, path1 %s\n", cwd.path);
	adjust_cwd(&cwd, fs.cwd);
	__cio_printf("idk, path2 %s\n", cwd.path);
	
	return result;
}

int change_dir(const char *path){
	#ifdef DEBUG
	__cio_printf("CD'ing to %s...\n", path);
	__delay(STEP);
	#endif

	DirectoryEntry *entry;
	DeconstructedPath dp;
	parse_path(path);

	if(dp.path_type == ABSOLUTE_PATH){
		__strcpy(fs.cwd, dp.path);
	}
	else if(dp.path_type == RELATIVE_PATH){
		clear_fs_buffer();
		__strcat(fs.cwd, dp.path);
		__cio_printf("GOT IT JK %s, \n", fs.cwd);
		if((entry = _fs_find_entry_from_path(fs.cwd)) == 0){
			__cio_printf("GOT IT %s, \n", fs.cwd);
			__cio_printf("GOT IT %s, \n", entry->filename);
			merge_path(dp.path);
			__cio_printf("GOT IT %s, \n", fs.cwd);
		}
		_fs_print_entry(_fs_find_entry_from_path(fs.cwd), true);
	}
	_fs_print_entry(_fs_find_entry_from_path("/"), true);

	// dr();
	
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

	if (path[0] != '/'){
		__cio_printf("ERROR: Please use absolute path to list dir content.\n");
		return -1;
	}

	DirectoryEntry *entry = _fs_find_entry_from_path(path);
	if (entry == NULL){
		__cio_printf("ERROR: Finding %s returned NULL value\n", path);
		return -1;
	}

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
		__cio_putchar('\n');
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
	
#if 0
	// Print directory contents
	for (uint32_t i = 0; i < dir->num_files; i++) {
		len = __strlen(dir->files[i]->filename);
		if (len > longest_line){
			longest_line = len;
		}
		// __cio_printf("|    %c %s |\n", entry->type == 1 ? FILLED_CIRCLE : OPEN_CIRCLE, dir->files[i]->filename); //TODO SEAN expand the info here
		__sprint(fs.buffer, "|    %c %s |\n", entry->type == 1 ? FILLED_CIRCLE : OPEN_CIRCLE, dir->files[i]->filename); //TODO SEAN expand the info here
		box_pad_right(longest_line);
	}
#endif

	#ifdef DEBUG
	__cio_printf("Listing %s!!!\n", path);
	__delay(STEP);
	#endif
	return 0;
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
	fs.bytes_per_sector = SECTOR_SIZE;
	fs.sectors_per_cluster = 8;
	fs.reserved_sector_count = 1;
	fs.number_of_fats = 1;
	fs.total_sectors = 2880;
	fs.fat_size_sectors = 16; // fat size in sectors
	fs.root_directory_cluster = 1; // root is at cluster _
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
	fs.root_directory_entry = (DirectoryEntry *) disk.request_space(sizeof(DirectoryEntry) * ROOT_DIRECTORY_ENTRIES); // TODO SEAN: do not allocate pages, but bytes

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

	// cwd
	fs.cwd[0] = '/';
	fs.cwd_dp = disk.request_space(sizeof(DeconstructedPath)); 
	parse_path("/");

	// Mount status
	fs.mounted = false;

	// Error Handling
	fs.last_error = 0;

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

DirectoryEntry *_fs_find_entry_from_path(const char *path) {
	#ifdef DEBUG
	__cio_printf("  Finding (from path %s)...\n", path);
	__delay(STEP);
	#endif
	
    parse_path(path);


	DirectoryEntry *parent = NULL;
	DirectoryEntry *child = NULL;

	if(nwd.num_dirs == 0){
		__cio_printf("ERROR: Root issue, num_dirs should never be 0, due to root.\n");
		return NULL;
	}

	if(nwd.num_dirs == 1){
		parent = _fs_find_root_entry(nwd.dirs[0]);
		if (parent == NULL){
			// __cio_printf(" _fs_find_entry_from_path(%s) failed:\n   Parent %s not found\n", path, dp.dirs[0]);
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
	}
	else if(nwd.num_dirs == 2){
		parent = _fs_find_root_entry(nwd.dirs[0]);
		child = _fs_find_root_entry(nwd.dirs[1]);
		if (parent == NULL){
			// __cio_printf(" _fs_find_entry_from_path(%s) failed:\n   Parent %s not found\n", path, dp.dirs[0]);
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
		if (child == NULL){
			// __cio_printf(" _fs_find_entry_from_path(%s) failed:\n   Child of %s, \"%s\", not found\n", path, dp.dirs[0], dp.dirs[1]);
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
	}
	
	else if (nwd.num_dirs == 3){
		parent = _fs_find_root_entry(nwd.dirs[1]);
		if (parent == NULL){
			// __cio_printf(" _fs_find_entry_from_path(%s) failed:\n  Parent root entry %s not found\n", path, dp.dirs[1]);
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
		#ifdef DEBUG
		__cio_printf("Looking for child at %s\n", nwd.paths[2]);
		#endif
		child = NULL;
		for(int i = 0; i < parent->subdirectory->num_files; i++){
			if (__strcmp(parent->subdirectory->files[i]->filename, nwd.dirs[2]) == 0 ){ // found child!
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
	}
	else if (nwd.num_dirs >= 4){
		parent = _fs_find_entry_from_path(nwd.paths[nwd.num_dirs - 2]);
		if (parent == NULL){
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
		__cio_printf("Looking for child at %s\n", nwd.paths[nwd.num_dirs - 2]);
		child = _fs_find_entry_from_path(nwd.paths[nwd.num_dirs - 2]);
		if(child == NULL){
			for(int i = 0; i < parent->subdirectory->num_files; i++){
				if (__strcmp(parent->subdirectory->files[i]->filename, nwd.dirs[nwd.num_dirs - 1]) == 0 ){ // found child!
					child = parent->subdirectory->files[i];
				}
			}
		}
		if(child == NULL){
			#ifdef DEBUG
			__cio_printf("  Finding (from path %s)---\n", path);
			__delay(STEP);
			#endif
			return NULL;
		}
	}

	#ifdef DEBUG
	__cio_printf("  Finding (from path %s)!!!\n", path);
	__delay(STEP);
	#endif
	return child;



	// 	if (curr_parent->type == FILE){
	// 		__cio_printf("ERROR: Type mismatch, found parent \"file\" %s while looking for directory\n", curr_parent->filename);
	// 	}
	// 	if (__strcmp(curr_entry->filename, dp.filename) == 0){ //TODO SEAN: neds to be smarter than just filename
	// 		__cio_printf("Found matching filenames: %s %s\n", curr_entry->filename, dp.filename);
	// 		#ifdef DEBUG
	// 		__cio_printf("Finding (from path %s)!!!\n", path);
	// 		#endif
	// 		break;
	// 	}
	// 	// __cio_printf("WE will now go to path %s", dp.paths[i]);
	// 	curr_entry = _fs_find_entry_from_path(dp.paths[i]);
	// }
	// __cio_printf("RETURNING\n");
	// ;
	// return curr_entry;
}

DirectoryEntry *_fs_find_root_entry(const char *filename) {
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

int _fs_create_entry_from_path(const char *entry_path, EntryType type){
	#ifdef DEBUG
	__cio_printf("  Creating (from path %s)...\n", entry_path);
	__delay(STEP);
	#endif
	clear_fs_buffer();
	__strcpy(fs.buffer, fs.cwd);
	__strcat(fs.buffer, entry_path);
	
	int result = -1;
	if(entry_path[0] != '/'){
		#ifdef DEBUG
		__cio_printf("Path is now relative\n");
		#endif
	}
	char *path = _km_page_alloc(1);
	__strcpy(path, entry_path);

	DeconstructedPath dp;
    parse_path(path);


	DirectoryEntry *entry = _fs_find_entry_from_path(path);
	if(entry != NULL){
		__cio_printf("ERROR: File %s already exists at %s", entry->filename, path);
		return -1;
	}
	// __cio_printf("FUTURE ENTRY: %s %d %d\n", entry->filename, entry->type, entry->size);

	DirectoryEntry *parent = NULL;
	DirectoryEntry *child = NULL;
	
	dr();
	pl();
	
	if(dp.num_dirs == 1){
		parent = _fs_find_root_entry(dp.dirs[0]);
		// __cio_printf("Parent: %s\n", parent);
		if (parent == NULL){
			__cio_printf("ERROR: Unable to find root\n");
			return -1;
		}
	}
	else if(dp.num_dirs == 2){
		parent = _fs_find_root_entry(dp.dirs[0]);
		// __cio_printf("Parent: %s\n", parent);
		if (parent == NULL){
			__cio_printf("ERROR: Unable to find root\n");
			return -1;
		}

		child = _fs_find_root_entry(dp.dirs[1]);

		// __cio_printf("Child: %s\n", child->filename);
		if (child == NULL){
			result = _fs_create_root_entry(dp.dirs[1], DIRECTORY);
			if(result != 0){
				__cio_printf("ERROR: Failed to find then create child \"%s\"\n", dp.dirs[1]);
				return -1;
			}
			child = _fs_find_root_entry(dp.dirs[1]);
			if(child == NULL){
				__cio_printf("ERROR: Failed to find created child \"%s\"\n", dp.dirs[1]);
				return -1;
			}
		}
	}
	else if (dp.num_dirs == 3){
		parent = _fs_find_root_entry(dp.dirs[1]);
		if(parent == NULL){
			#ifdef DEBUG
			__cio_printf("Entry %s does not already exist in root, creating...\n", dp.dirs[1]);
			#endif
			result = _fs_create_root_entry(dp.dirs[1], DIRECTORY);
			if (result == -1){
				__cio_printf("ERROR: Unable to create directory %s in root\n", dp.dirs[1]);
			}
			parent = _fs_find_root_entry(dp.dirs[1]);
			if(parent == NULL){
				__cio_printf("ERROR: Could not find newly created dir %s\n", dp.dirs[1]);
			}
		}
		child = _fs_find_entry_from_path(dp.paths[2]);
		if (child != NULL){
			// this should not be hit, as we have  already
			// validated that path "/parent/child" return NULL;
			__cio_printf("ERROR: Child somehow already exists, aborting\n");
			return -1;
		}
		else{ // create child
			#ifdef DEBUG
			__cio_printf("Child %s is NULL, creating...\n", dp.dirs[2]);
			#endif
			child = _km_page_alloc(1);
			_fs_initialize_directory_entry(child, dp.dirs[2], 0, DIRECTORY, 0, NULL, parent->depth+1);
			add_sub_entry(parent, child);
		}
	}
	else if (dp.num_dirs >= 4){
		parent = _fs_find_entry_from_path(dp.paths[dp.num_dirs- 2]); // starts at index \2 for nd=4
		while(parent == NULL){
			#ifdef DEBUG
			__cio_printf("Entry %s does not already exist in root, creating...\n", dp.dirs[dp.num_dirs - 2]);
			#endif
			result = _fs_create_entry_from_path(dp.paths[dp.num_dirs - 2], DIRECTORY);
			if (result == -1){
				__cio_printf("ERROR: Unable to create directory %s\n", dp.dirs[dp.num_dirs - 2]);
			}
			pln();
			parent = _fs_find_entry_from_path(dp.paths[dp.num_dirs - 2]);
			if(parent == NULL){
				__cio_printf("ERROR: Could not find newly created parent %s\n", dp.paths[dp.num_dirs - 2]);
				return -1;
			}
		}
		__cio_printf("Looking for child at %s\n", dp.paths[dp.num_dirs - 1]);
		child = _fs_find_entry_from_path(dp.paths[dp.num_dirs - 1]);
		if(child == NULL){
			for(int i = 0; i <= parent->subdirectory->num_files; i++){
				__cio_printf("Looking for child at %s\n", dp.paths[dp.num_dirs - 1]);
				pln();
				// dr();
				print_parsed_path(dp);
				if (__strcmp(parent->subdirectory->files[i]->filename, dp.dirs[dp.num_dirs - 1]) == 0 ){ // found child!
					child = parent->subdirectory->files[i];
					break;
				}
			}
		}
		if(child == NULL){
			result = _fs_create_entry_from_path(dp.paths[dp.num_dirs - 1], type);
			if (result == -1){
				__cio_printf("ERROR: Unable to create entry %s\n", dp.dirs[dp.num_dirs - 1]);
				return -1;
			}
			child = _fs_find_entry_from_path(dp.paths[dp.num_dirs - 1]);
			__cio_printf("FOUND CHILD \n");
			_fs_print_entry(child, true);
		}
		
		if(child == NULL){
			__cio_printf("ERROR: Failed to create file at %s\n", dp.dirs[dp.num_dirs - 1]);
			return NULL;
		}
	}

	#ifdef DEBUG
	__cio_printf("  Creating (from path %s)!!!\n", path);
	__delay(STEP);
	#endif
	return 0;

	if(dp.num_dirs == 0){
		result = _fs_mount();
		// TODO SEAN - check 0 case, see if mounted?	
	}
}

int _fs_create_root_entry(const char *filename, EntryType type) {
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
	if(_fs_find_root_entry(filename) != NULL){
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
    _fs_initialize_directory_entry(new_entry, filename, 0, type, 0, NULL, 1);
	
	int result = add_sub_entry(_fs_find_root_entry("/"), new_entry);
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

void _fs_initialize_directory_entry(DirectoryEntry *entry, const char *filename, uint32_t size, EntryType type, uint32_t cluster, DirectoryEntry *next, uint8_t depth) {
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

    // Allocate memory for the subdirectory
    entry->subdirectory = _km_page_alloc(1);
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

	DirectoryEntry *src = _fs_find_root_entry(old_filename);
	DirectoryEntry *dest = _fs_find_root_entry(new_filename);
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

	DirectoryEntry *entry = _fs_find_root_entry(filename);
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

	DirectoryEntry *entry = _fs_find_root_entry(filename);
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
	DirectoryEntry *entry = _fs_find_root_entry(filename);
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
		}
		__cio_printf("%c %s\n", child->type == 2 ? OPEN_CIRCLE : FILLED_CIRCLE, child->filename);
		if(child->type == FILE){
			__cio_printf(" (%d bytes)");
		}
		 _fs_print_children(child);
	}
	// #ifdef DEBUG
	// __cio_printf("Printing %d children!!!\n", entry->subdirectory->num_files);
	// __delay(STEP);
	// #endif
	return num_files;
}

int _fs_print_entry(DirectoryEntry *entry, bool_t print_children){
	// #ifdef DEBUG
	// __cio_printf("  Printing \"%s\"...\n", entry->filename);
	// __delay(STEP);
	// #endif

	if(entry == NULL){
		__cio_printf("ERROR: Could not print %s, entry is NULL\n", entry->filename);
		return -1;
	}
	if(entry->type == DIRECTORY){
		char *header = _km_page_alloc(1);
		__memclr(header, BLOCK_SIZE);
		__strcat(header, "Directory: ");
		__strcat(header, entry->filename);
		for(int i = 0; i < entry->depth; i++){
			__cio_putchar(' ');
		}
		pvl(header, '-', 1); // do this or the line below
		// __cio_printf("%s\n", header);
		if(print_children){
			_fs_print_children(entry);
		}
	}

	// #ifdef DEBUG
	// __cio_printf("  Printing \"%s\"!!!\n", entry->filename);
	// __delay(STEP);
	// #endif
	return 0;
}

#endif // END Filesystem implementation

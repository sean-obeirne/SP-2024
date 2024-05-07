/*
** File: fshelper.h
** Description: This file includes a bunch of helper functions to facilitate
** the implementation of a filesystem. Also defines useful constants
**
** @author Sean O'Beirne
*/

#ifndef FS_HELPER_H_
#define FS_HELPER_H_
#if 1  // BEG variables

// #include <stdint.h> // Include necessary standard headers
#include "common.h"
#include "filesystem.h"
#include "ramdisk.h"

// Define constants, macros, and data structures specific to FAT32 filesystem


// Print debugging info, such as current operation or success status
////////////////
// #define DEBUG
////////////////
// Pause time definitions
#define SLEEP_FACTOR 2
#define STEP 1
#define FIVER 5
#define CSTEP 1 * SLEEP_FACTOR
#define MOMENT 3 * SLEEP_FACTOR
#define SHORT_PAUSE 20 * SLEEP_FACTOR
#define DEBUG_DELAY 50 * SLEEP_FACTOR
#define LONG_PAUSE 100 * SLEEP_FACTOR
#define INF_PAUSE 1000000 * SLEEP_FACTOR

// Useful ASCII codes
#define MAX_ASCII_CHAR 127 // 128+ seems invalid
#define FILLED_CIRCLE 7
#define OPEN_CIRCLE 9

typedef struct DirectoryEntry DirectoryEntry;
typedef struct DeconstructedPath DeconstructedPath;
typedef enum EntryType EntryType;
#endif // END Variables

#if 1  // BEG functions

/*
** Show header information.
** @param horrizontal Flag indicating whether to display the
** information horizontally (true) or vertically (false).
*/
void show_header_info(bool_t horrizontal);

/*
** Print a header with the provided text.
** @param text The text to display in the header.
*/
void ph(const char *text);

/*
** Print a header with the provided text, followed by a newline 
** and clearing screen after specified ticks.
** @param text The text to display in the header.
** @param ticks The number of ticks to delay before clearing the screen.
*/
void phn(const char *text, int ticks);

/*
** Print an 80-character long line with '-'.
*/
void pl(void);

/*
** Print an 80-character long line with '-', only if DEBUG is enabled.
*/
void pl_debug(void);

/*
** Print an 80-character long line with '-'. No delay.
*/
void pln(void);

/*
** Print an 80-character line with '-'. No delay, only if DEBUG is enabled.
*/
void plnn(void);

/*
** Print an 80-character long line with the provided character.
** @param line_char The character to use for printing the line.
*/
void plw(char line_char);

/*
** Print a header followed by a full line of the provided character.
** @param header The header text to display.
** @param line_char The character to use for printing the line.
*/
void plh(const char *header, char line_char);

/*
** Print a header (or not if NULL) followed by a 'half' line.
** @param header The header text to display.
*/
void phl(const char *header);

/*
** Print a header followed by a variable length line.
** @param header The header text to display.
** @param line_char The character to use for printing the line.
** @param indent The number of spaces to indent the header.
*/
void pvl(const char *header, char line_char, int indent);

/*
** Print a newline character.
*/
void nl(void);

/*
** Print characters and their ASCII codes.
*/
void print_chars(void);

/*
** Delay for a short period of time.
*/
void d(void);

/*
** Create a box for the provided string.
** @param max_item_size The maximum size of the item to be contained in the box.
*/
void box_h(int max_item_size);

/*
** Print the provided string inside a box.
** @param to_print The string to be printed inside the box.
*/
void box_v(char *to_print);

/*
** Pad the right side of the box with spaces.
** @param longest_line The length of the longest line in the box.
*/
void box_pad_right(int longest_line);

#if 0
void show_header_info( bool_t horrizontal );

void phn( const char *header, int ticks );
void ph( const char *header );
void pl( void );
void pl_debug( void );
void pln( void );
void plnn( void );
void plw( char line_char );
void plh( const char *header, char line_char );
void phl( const char *header );
void pvl( const char *header, char line_char, int indent );
void nl( void );

void box_h( int max_item_size );
void box_v( char *to_print );
void box_pad_right( int longest_line );
void print_chars( void );
void d( void );
#endif


/*
** Initialize the filesystem buffer.
** Allocates memory for the filesystem buffer and initializes it.
*/
void init_fs_buffer( void );

/*
** Clear the contents of the filesystem buffer.
*/
void clear_fs_buffer( void );

/*
** Dump the contents of the filesystem buffer to the console.
** Prints the contents of the filesystem buffer to the console.
** @return 0 on success, -1 on failure.
*/
int dump_fs_buffer( void );

/*
** Convenience function to print the FS buffer in one line
*/
void pb( void );

/*
** Function to dump the root directory to cio.
** Prints all directoryies recursively.
*/
void dr( void );

/*
** Dump the contents of the File Allocation Table (FAT) to the console.
** Prints all entries along with their FAT status.
*/
void dump_fat( void );

/*
** Check if the provided path is the root directory.
** @param path The path to check.
** @return true if the path is the root directory, false otherwise.
*/
bool_t is_root( const char *path );

/*
** Remove leading and trailing slashes from the provided path.
** @param path The path to strip.
** @return A pointer to the stripped path.
*/
char *strip_path( const char *path );

/*
** Clean the newly parsed working directory structure.
** Resets the working directory structure to its initial state.
*/
void clean_nwd( void );

/*
** Parse the provided path into its individual components.
** @param path The path to parse.
*/
void parse_path( const char *path );

/*
** Test function to validate path parsing.
** @param path The path to test.
*/
void test_parse_path( const char *path );

/*
** Print the parsed working directory structure to the console.
*/
void print_parsed_path( void );

/*
** Allocate a new cluster in the File Allocation Table (FAT).
** @return The index of the allocated cluster.
*/
uint16_t allocate_cluster( void );

/*
** Get the next cluster in the chain from the current cluster.
** Retrieves the next cluster in the chain from the current cluster.
** @param current_cluster The current cluster.
** @return The index of the next cluster.
*/
int get_next_cluster( int current_cluster );

/*
** Update the File Allocation Table (FAT) entry for the provided cluster.
** @param cluster The cluster to update.
** @param value The value to set in the FAT entry.
*/
void update_fat_entry( uint16_t cluster, uint16_t value );

/*
** Create a new sub-entry (file or directory) in the filesystem.
** Creates with the specified parent directory and filename.
** @param parent The parent directory entry.
** @param filename The name of the new sub-entry.
** @param type The type of the new sub-entry (file or directory).
** @return A pointer to the newly created sub-entry.
*/
DirectoryEntry *create_sub_entry( DirectoryEntry *parent, const char *filename, EntryType type );

/*
** Add a sub-entry (file or directory) to the destination directory.
** @param dest The destination directory entry.
** @param insert The sub-entry to insert.
** @return 0 on success, -1 on failure.
*/
int add_sub_entry( DirectoryEntry *dest, DirectoryEntry *insert );

/*
** Find or create an entry (file or directory) in the filesystem.
** @return A pointer to the found or created entry.
*/
DirectoryEntry *find_or_create_entry( void );

/*
** Add an entry to the File Allocation Table (FAT) for the provided next cluster.
** @param next_cluster The next cluster to add to the FAT.
** @return The index of the added entry.
*/
int add_fat_entry( uint32_t next_cluster );

/*
** Check if the provided parent directory contains the specified target.
** Checks if the provided parent directory contains the specified target (file or directory).
** @param parent The parent directory entry.
** @param target The target to check.
** @return 1 if the directory contains the target, 0 otherwise.
*/
int dir_contains( DirectoryEntry *parent, const char *target );

/*
** Adjust the current working directory based on the provided path.
** @param cwd The deconstructed path structure representing the current working directory.
** @param path The path to adjust the current working directory to.
*/
void adjust_cwd( DeconstructedPath *cwd, const char *path );

/*
** Change the current working directory to its parent directory.
** @return 0 on success, -1 on failure.
*/
int cd_parent( void );


#if 0
void init_fs_buffer(void);
void clear_fs_buffer(void);
int dump_fs_buffer(void);
void pb(void);
void dr(void);
void dump_fat(void);
bool_t is_root(const char *path);

void get_path( void );
void parse_input(int in_len); // from buffer
void run_command(char **args, int word_count);
void merge_path(char *path);

char *strip_path(const char *path);
void clean_nwd( void );
void parse_path(const char *path);
void test_parse_path(const char *path);
void print_parsed_path(void);

uint16_t allocate_cluster(void);
int get_next_cluster(int current_cluster);
void update_fat_entry(uint16_t cluster, uint16_t value);

DirectoryEntry *create_sub_entry(DirectoryEntry *parent, const char *filename, EntryType type);
int add_sub_entry(DirectoryEntry *dest, DirectoryEntry *insert);
DirectoryEntry *find_or_create_entry( void );

int add_fat_entry(uint32_t next_cluster);

int get_subdirectory_count(DirectoryEntry *parent);
int dir_contains(DirectoryEntry *parent, const char *target);
void adjust_cwd( DeconstructedPath *cwd, const char *path);
int cd_parent( void );
#endif
#endif // END functions

#endif /* FS_HELPER_H_ */

#if 1  // TODO Useful REGEX queries
/*
Useful REGEX queries:

find all delays
(STEP|FIVER|CSTEP|MOMENT|SHORT_PAUSE|DEBUG_DELAY|LONG_PAUSE|INF_PAUSE)

find non-STEP delays
__delay\((?!STEP\)).*?\)

*/
#endif // TODO Useful REGEX queries
#include "ramdisk.h"
#include "kmem.h"
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output

// TODO SEAN: Could make each block its own allocation unit
//	loop through blocks unil one has enough space

typedef struct Chunk {
	uint32_t uid;
	bool_t is_allocated;
	uint32_t size;
	struct Chunk *next;
} Chunk;

// Define memory pool structure
typedef struct MemoryPool {
    struct Chunk *pool_start;
    uint32_t pool_size;
    // Add any additional fields as needed
} MemoryPool;



static MemoryPool pool = {0};
static uint32_t next_unique_id = 0;

// Function to initialize the storage backend
int storage_init(StorageInterface *storage, uint32_t pages) {
    // Check if storage pointer is valid
    if (storage == NULL) {
        return -1; // Invalid argument
    }

	// Storage interface for RAM disk
	static StorageInterface ramdisk_interface = {
		.init = ramdisk_init,
		.read = ramdisk_read,
		.write = ramdisk_write,
		.request_space = ramdisk_request_space,
		.release_space = ramdisk_release_space
	};

	// Initialize RAM disk
    int result = ramdisk_init(pages);
    if (result != 0) {
        return result; // Error initializing RAM disk
    }

    // Assign RAM disk storage interface to the provided storage pointer
    *storage = ramdisk_interface;

    return 0; // Success
}

///////////////////////////////////////////////////////////////////////////////


int ramdisk_init(uint32_t pages) {
    pool.pool_start = (Chunk *)_km_page_alloc(pages);
	if(pool.pool_start == NULL){
		__cio_printf("Failed to init ramdisk pool with %d pages", pages);
		return -1;
	}
	next_unique_id = 0;

	pool.pool_start->uid = next_unique_id++;
	pool.pool_start->size = 16;
	pool.pool_start->is_allocated = true;
	pool.pool_start->next = (Chunk *)((char *)pool.pool_start + pool.pool_start->size + sizeof(Chunk));

    pool.pool_size = pages * SZ_PAGE;


    // Your implementation here
	__cio_printf("Ramdisk memory pool initiated with %d bytes\n", pool.pool_size);
    return 0;
}

Chunk *get_chunk(const int uid){
	Chunk *chunk = pool.pool_start;

	while (chunk != NULL) {
        if (uid == chunk->uid) {
            return chunk;
        }
        chunk = chunk->next;
    }
	return NULL;
}

int ramdisk_read(const int uid, void *buffer, uint32_t size) {
    // Check if the memory pool pointer is valid
    if (pool.pool_start == NULL) {
        return -1; // Invalid memory pool
    }

    // Check if the byte count is within the bounds of the memory pool
    if (size >= pool.pool_size) {
		__cio_printf("Data size %d exceeds pool size %d.\n", size, pool.pool_size);
        return -1;
    }

    // Calculate the starting address of the block within the memory pool
	Chunk *chunk = get_chunk(uid);

	// Check if chunk can fit in buffer
	if (chunk->size > size){
		__cio_printf("Chunk size (%d) too big for full read into buffer size (%d)\n", chunk->size, size);
        return -1; // Buffer too small
	}

    // Copy data from the chunk to the buffer
	__memcpy(buffer, (void *)(chunk + 1), size);

    return 0; // Success
}

Chunk *get_free_chunk(uint32_t size) {
    __cio_puts("Finding free chunk...\n");

    Chunk *chunk = pool.pool_start;

	while (chunk != NULL) {
		chunk->next = (Chunk *)((char *)chunk + chunk->size + sizeof(Chunk));
		
        if (!chunk->is_allocated) {
			if(chunk->size >= size || chunk->size == 0){
            	return chunk;
			}
        }
        chunk = chunk->next;
    }

    __cio_puts("No free chunk of sufficient size found\n");
    return NULL;
}

int ramdisk_write(const void *data, uint32_t size) {
	__cio_printf("Ramdisk Writing...\n");
    if (pool.pool_start == NULL) {
		__cio_printf("Error with pool.\n");
        return -1; // Invalid memory pool
    }

    // Check if the byte count is within the bounds of the memory pool
    if (size >= pool.pool_size) {
		__cio_printf("Data size %d exceeds pool size %d.\n", size, pool.pool_size);
        return -1;
    }

	if (sizeof(*data) > 1){
		__cio_printf("=============>1BYTE CONDITION has been triggered\n");
	}

    Chunk *chunk_address = get_free_chunk(size);
	if (chunk_address == NULL) {
        __cio_printf("Error: No free chunk found.\n");
        return -1;
    }
	chunk_address->uid = next_unique_id++;
	chunk_address->is_allocated = true;
	if(chunk_address->size == 0){ // only resize new blocks, TODO SEAN
		chunk_address->size = size;
	}

	// __memset((void *)(chunk_address + 1), "s", size);
	__memcpy((void *)(chunk_address + 1), data, size);
	// __cio_printf("Debugging: %d\n", sizeof(Chunk));
	// __cio_printf("Debugging: %s or %d\n", (char *)(chunk_address), chunk_address-1);
	// __cio_printf("Debugging: %s or %d\n", (char *)(chunk_address), chunk_address);
	// __cio_printf("Debugging: %s or %d\n", (char *)(chunk_address), chunk_address+1);

    return chunk_address->uid;
}

int ramdisk_request_space(uint32_t size) {
	__cio_puts("Requesting space...\n");
	// Check if the memory pool pointer is valid
    if (pool.pool_start == NULL) {
        return -1; // Invalid memory pool
    }

    // Iterate through the memory pool to find a suitable free chunk
    Chunk *chunk = get_free_chunk(size);
    if (chunk == NULL) {
        // No free chunk of sufficient size found
        return -1;
    }

    // Mark the chunk as allocated and update its size
	chunk->uid = next_unique_id++;
    chunk->is_allocated = true;
    chunk->size = size;

    return chunk->uid; // Success
}

int ramdisk_release_space(const int uid) {
    // Check if the memory pool pointer is valid
    if (pool.pool_start == NULL) {
		__cio_printf("Error with pool.\n");
        return -1; // Invalid memory pool
    }

	// Iterate through the memory pool to find the chunk with the specified UID
    Chunk *chunk = pool.pool_start;
    while (chunk != NULL) {
        if (chunk->uid == uid) {
            // Found the chunk with the specified UID
            // Mark it as deallocated
            chunk->is_allocated = false;
            return 0; // Success
        }
        chunk = chunk->next;
    }

    // Chunk with the specified UID not found
	__cio_printf("Could not find chunk with UID %d\n", uid);
    return -1;
}

void ramdisk_print( void ){
	__cio_puts("Printing Ramdisk...\n");
	Chunk *chunk = pool.pool_start;
	while(chunk != NULL){
		if (chunk->size != 0){
			__cio_printf("Chunk Details:  uid %d  size %d  is_allocated %d  next %d\n", chunk->uid, chunk->size, chunk->is_allocated, chunk->next);
		}
		chunk = chunk->next;
	}
	__cio_puts("Ramdisk printed.\n");
}
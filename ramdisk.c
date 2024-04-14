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
	uint32_t data[20]; // TODO SEAN: this is bad....
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
int storage_init(StorageInterface *storage, uint32_t size) {
    // Check if storage pointer is valid
    if (storage == NULL) {
        return -1; // Invalid argument
    }

    // Initialize RAM disk with size 16
	// MemoryPool *pool = (MemoryPool *) _km_page_alloc(1);
    // int result = ramdisk_init(pool, size);
    // if (result != 0) {
	// 	__cio_printf("Error initializing ramdisk\n");
    //     return result; // Error initializing RAM disk
    // }

	// Storage interface for RAM disk
	static StorageInterface ramdisk_interface = {
		.init = ramdisk_init,
		.read = ramdisk_read,
		.write = ramdisk_write,
		.request_space = ramdisk_request_space,
		.release_space = ramdisk_release_space
	};

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
		// chunk->next = (Chunk *)((char *)chunk + chunk->size + sizeof(Chunk));
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
    // void *block_address = (char*)pool.pool_start;
	Chunk *chunk_address = get_chunk(uid);
	__cio_printf("READ Chunk Details:\n  uid %d  size %d  is_allocated %d  next %d\n", chunk_address->uid, chunk_address->size, chunk_address->is_allocated, chunk_address->next);

    // Copy data from the block to the buffer
    __memcpy(buffer, chunk_address->data, size);

    return 0; // Success
}

int ramdisk_print( void ){
	__cio_puts("Printing Ramdisk...\n");
	Chunk *chunk = pool.pool_start;
	while(chunk != NULL){
		if(chunk->is_allocated){
			__cio_printf("Chunk Details:\n  uid %d  size %d  is_allocated %d  next %d\n", chunk->uid, chunk->size, chunk->is_allocated, chunk->next);
		}
		chunk = chunk->next;
	}
	return 0;
}


Chunk *get_free_chunk(uint32_t size) {
    __cio_puts("Finding free chunk...\n");

    Chunk *chunk = pool.pool_start;
    Chunk *prev_chunk = NULL;

	while (chunk != NULL) {
		chunk->next = (Chunk *)((char *)chunk + chunk->size + sizeof(Chunk));
		
        if (!chunk->is_allocated) {
            return chunk;
        }
        prev_chunk = chunk;
        chunk = chunk->next;
    }

    // No free chunk found
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
		__cio_printf(">1BYTE CONDITION has been triggered\n");
	}

    Chunk *chunk_address = get_free_chunk(size);
	if (chunk_address == NULL) {
        __cio_printf("Error: No free chunk found.\n");
        return -1;
    }
	chunk_address->uid = next_unique_id++;
	chunk_address->size = size;
	chunk_address->is_allocated = true;

	__memcpy(chunk_address->data, data, size);

    __cio_printf("Chunk address: %d\n", (void *)chunk_address);
    __cio_printf("Chunk data: %s\n", (char *)chunk_address);

	ramdisk_print();

    return 0;
}

int ramdisk_request_space(uint32_t size) {

    return 0;
}

int ramdisk_release_space(uint32_t block, uint32_t size) {
    // Your implementation here
    return 0;
}
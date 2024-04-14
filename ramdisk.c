#include "ramdisk.h"
#include "kmem.h"
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output

// TODO SEAN: Could make each block its own allocation unit
//	loop through blocks unil one has enough space

typedef struct Chunk {
	// bool_t is_allocated;
	uint32_t size;
	// Chunk *next;
} Chunk;

// Define memory pool structure
typedef struct MemoryPool {
    Chunk *pool_start;
    uint32_t pool_size;
    // Add any additional fields as needed
} MemoryPool;



static MemoryPool pool = {0};

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
	pool.pool_start->size = 8;
	// pool.pool_start->is_allocated = true;
	// pool.pool_start->next = NULL;

    pool.pool_size = pages * SZ_PAGE;

    // Your implementation here
	__cio_printf("Ramdisk memory pool initiated with %d bytes\n", pool.pool_size);
    return 0;
}

int ramdisk_read(uint32_t block, void *buffer, uint32_t size) {
    // Check if the memory pool pointer is valid
    if (pool.pool_start == NULL) {
        return -1; // Invalid memory pool
    }

    // Check if the block number is within the bounds of the memory pool
    if (block >= pool.pool_size) {
        return -1; // Invalid block number
    }

    // Calculate the starting address of the block within the memory pool
    void *block_address = (char*)pool.pool_start + block;

    // Copy data from the block to the buffer
    __memcpy(buffer, block_address, size);

    return 0; // Success
}

Chunk *get_free_chunk(uint32_t size){

	Chunk *chunk = pool.pool_start;
	__cio_printf("CHUNK: %d\n", chunk);
	int i = 0;
	while( chunk->size != 0 ){
		__cio_printf("Incrementing through memory pool by %d\n", chunk->size);
		chunk += chunk->size;
		__cio_printf("CHUNK: %d\n", chunk);
	}
	if(chunk->size == 0){
		chunk->size = size;
		
	}
	else{
		__cio_printf("Memory pool full!");
		return NULL;
	}
	__cio_printf("Got free chunk at pool[%d] of size %d ending at pool[%d]\n", chunk-pool.pool_start, chunk->size, chunk - pool.pool_start + chunk->size);

	return chunk;
}

int ramdisk_write(const void *data, uint32_t size) {
    // Check if the memory pool pointer is valid
	__cio_printf("Ramdisk Writing...\n");
    if (pool.pool_start == NULL) {
		__cio_printf("Error with pool.\n");
        return -1; // Invalid memory pool
    }

    // Check if the byte count is within the bounds of the memory pool
    if (size >= pool.pool_size) {
		// __cio_printf("Data %s exceeds pool_size.\n", (char *)data);
		__cio_printf("Data size %d exceeds pool size %d.\n", size, pool.pool_size);
        return -1; // Invalid block number
    }

	if (sizeof(*data) > 1){
		__cio_printf("\n");
		__cio_printf(">1BYTE CONDITION has been dealt with\n");
		__cio_printf("\n");
		__cio_printf("\n");
		__cio_printf("\n");
	}

	__cio_printf("Data *: %s\n", (char *) data);
	// __cio_printf("sizeof() data *: %d\n", sizeof(data));
	// __cio_printf("Data: %c\n", *((char *) data));
	__cio_printf("sizeof() data elements: %d\n", sizeof(*data));
	__cio_printf("Number of elements in data : %d\n", size);
	// __cio_printf("Data &: %c\n", &data);
	// __cio_printf("sizeof() data &: %d\n", sizeof(&data));
	// __cio_printf("Size: %d\n", size);
	__cio_printf("\n");


    // Calculate the starting address of the block within the memory pool
    // void *block_address = (char*)pool.pool_start;
    Chunk *chunk_address = get_free_chunk(size);

	if (chunk_address == NULL) {
        __cio_printf("Error: No free chunk found.\n");
        return -1;
    }

    // Copy data to the block
    __memcpy(chunk_address, data, size);

    __cio_printf("Chunk address: %d\n", (void *)chunk_address);
    __cio_printf("Chunk data: %s\n", (char *)chunk_address);
    return 0;
}

int ramdisk_request_space(uint32_t size) {

    return 0;
}

int ramdisk_release_space(uint32_t block, uint32_t size) {
    // Your implementation here
    return 0;
}
#include "ramdisk.h"
#include "kmem.h"
#include "lib.h" // Include helpful libraries
#include "cio.h" // Include console output

// RAM disk memory size (in bytes)
#define RAMDISK_SIZE 16

// TODO SEAN: Could make each block its own allocation unit
//	loop through blocks unil one has enough space


// Define memory pool structure
typedef struct MemoryPool {
    void *pool_start;
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

    pool.pool_start = _km_page_alloc(pages);
	if(pool.pool_start == NULL){
		__cio_printf("Failed to init ramdisk pool with %d pages", pages);
		return -1;
	}
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

int ramdisk_write(uint32_t block, const void *data, uint32_t size) {
    // Check if the memory pool pointer is valid
	__cio_printf("Ramdisk Writing...\n");
    if (pool.pool_start == NULL) {
		__cio_printf("Error with pool.\n");
        return -1; // Invalid memory pool
    }

    // Check if the block number is within the bounds of the memory pool
    if (block >= pool.pool_size) {
		__cio_printf("Block %d exceeds pool_size.\n", block);
        return -1; // Invalid block number
    }

    // Calculate the starting address of the block within the memory pool
    void *block_address = (char*)pool.pool_start + block;

    // Copy data to the block
    __memcpy(block_address, data, size);

    return 0; // Success
}

int ramdisk_request_space(uint32_t size, uint32_t *block) {
    // Your implementation here
    return 0;
}

int ramdisk_release_space(uint32_t block, uint32_t size) {
    // Your implementation here
    return 0;
}
#ifndef RAMDISK_H
#define RAMDISK_H

#include "common.h" // Include custom data types
#include "kmem.h" // Include memory management

// Define memory pool structure
typedef struct MemoryPool {
    void *pool_start;
    uint32_t pool_size;
    // Add any additional fields as needed
} MemoryPool;

// Define the storage interface
typedef struct {
    int (*init)(MemoryPool *pool, uint32_t size);
    int (*read)(MemoryPool *pool, uint32_t block, void *buffer, uint32_t size);
    int (*write)(MemoryPool *pool, uint32_t block, const void *data, uint32_t size);
    int (*request_space)(MemoryPool *pool, uint32_t size, uint32_t *block);
    int (*release_space)(MemoryPool *pool, uint32_t block, uint32_t size);
} StorageInterface;

// Function to initialize the storage backend
int storage_init(StorageInterface *storage, uint32_t size);

// Function to switch storage backend
void use_storage_backend(StorageInterface *storage);


/*
** Function to initialize the RAM disk
** @param size Size of the RAM disk in bytes
** @return 0 on success, -1 on failure
*/
int ramdisk_init(MemoryPool *pool, uint32_t size);

/*
** Function to read data from the RAM disk
** @param block Starting block number to read from
** @param buffer Pointer to a buffer to store the read data
** @param size Number of bytes to read
** @return 0 on success, -1 on failure
*/
int ramdisk_read(MemoryPool *pool, uint32_t block, void *buffer, uint32_t size);

/*
** Function to write data to the RAM disk
** @param block Starting block number to write to
** @param data Pointer to the data to write
** @param size Number of bytes to write
** @return 0 on success, -1 on failure
*/
int ramdisk_write(MemoryPool *pool, uint32_t block, const void *data, uint32_t size);

/*
** Function to request space on the RAM disk
** @param size Number of bytes to allocate
** @param block Pointer to store the starting block number of the allocated space
** @return 0 on success, -1 on failure
*/
int ramdisk_request_space(MemoryPool *pool, uint32_t size, uint32_t *block);

/*
** Function to release space on the RAM disk
** @param block Starting block number of the space to release
** @param size Number of bytes to release
** @return 0 on success, -1 on failure
*/
int ramdisk_release_space(MemoryPool *pool, uint32_t block, uint32_t size);

// Additional functions as needed...

#endif /* RAMDISK_H */

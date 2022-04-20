#ifndef BLOADER_H
#define BLOADER_H

/**
 * Bloader is a threaded buffer loader module.
 * 
 * Once initialized, the bloader thread will repeatedly call inFunc() to acquire
 * items to be queued into the buffer. If the buffer is full, the bloader thread
 * will hang until bloader_get(b) is called.
 * 
 * Calling get will return the next value from the queue. If the buffer is empty,
 * the calling thread will hang until inFunc successfully returns a value.
 * 
 * Only passes pointers for efficiency reasons. It is the users responsibility
 * to manage how data is stored in memory.
 */

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

struct Bloader {
	struct Queue queue;
	bool isRunning;
	pthread_t thread;
	sem_t semMax;
	sem_t semMin;
	queuedata_t* (*inFunc)(void);
};

void bloader_init(struct Bloader* bloader
	            ,queuedata_t* (*inFunc)(void) // getter function
	            ,const int buffersize);

void bloader_cleanup(struct Bloader* bloader);

// Manually add data to the buffer. Will hang if buffer is full.
void bloader_put(struct Bloader* bloader, queuedata_t* data);

// Hangs if buffer is empty.
queuedata_t* bloader_get(struct Bloader* bloader);

#endif
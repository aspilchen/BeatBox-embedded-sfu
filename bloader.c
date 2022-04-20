#include <stdio.h>
#include "bloader.h"

static void bloader_initImpl(struct Bloader* bloader
	                        ,queuedata_t* (*inFunc)(void)
				            ,const int buffersize);

static void bloader_cleanupImpl(struct Bloader* bloader);
static void bloader_putImpl(struct Bloader* bloader, queuedata_t* data);
static void* bloader_thread(void* arg);
static queuedata_t* bloader_getImpl(struct Bloader* bloader);

/*******************************/
/********** Interface **********/

void bloader_init(struct Bloader* bloader
	            ,queuedata_t* (*inFunc)(void)
				,const int buffersize)
{
	bloader_initImpl(bloader, inFunc, buffersize);
}

void bloader_cleanup(struct Bloader* bloader)
{
	bloader_cleanupImpl(bloader);
}

void bloader_put(struct Bloader* bloader, queuedata_t* data)
{
	// if(data) {
		sem_wait(&(bloader->semMax));
		bloader_putImpl(bloader, data);
		sem_post(&(bloader->semMin));
	// }
}

queuedata_t* bloader_get(struct Bloader* bloader)
{
	queuedata_t* d;
	sem_wait(&(bloader->semMin));
	d = bloader_getImpl(bloader);
	sem_post(&(bloader->semMax));
	return d;
}

/************************************/
/********** Implementation **********/

static void bloader_initImpl(struct Bloader* bloader
	                        ,queuedata_t* (*inFunc)(void)
				            ,const int buffersize)
{
	queue_init(&(bloader->queue), buffersize);
	sem_init(&(bloader->semMax), 0, buffersize);
	sem_init(&(bloader->semMin), 0, 0);
	bloader->inFunc = inFunc;
	pthread_create(&(bloader->thread), NULL, bloader_thread, bloader);
}

static void bloader_cleanupImpl(struct Bloader* bloader)
{
	bloader->isRunning = false;
	pthread_cancel(bloader->thread);
	sem_destroy(&(bloader->semMax));
	sem_destroy(&(bloader->semMin));
	queue_cleanup(&(bloader->queue));
}

static void bloader_putImpl(struct Bloader* bloader, queuedata_t* data)
{
	queue_push(&(bloader->queue), data);
}

static queuedata_t* bloader_getImpl(struct Bloader* bloader)
{
	queuedata_t* d = queue_peek(&(bloader->queue));
	queue_pop(&(bloader->queue));
	return d;
}

static void* bloader_thread(void* arg)
{
	struct Bloader* bloader = (struct Bloader*)arg;
	queuedata_t* data;
	bloader->isRunning = true;
	while(bloader->isRunning) {
		data = bloader->inFunc();
		bloader_put(bloader, data);
	}
	return NULL;
}
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "queue.h"

static void queue_init_impl(struct Queue* queue, const int size);
static void queue_cleanup_impl(struct Queue* queue);
static void queue_push_impl(struct Queue* queue, queuedata_t* data);
static void queue_pop_impl(struct Queue* queue);
static bool queue_isFull_impl(const struct Queue* queue);
static bool queue_isEmpty_impl(const struct Queue* queue);
static queuedata_t* queue_peek_impl(const struct Queue* queue);

/*******************************/
/********** Interface **********/

void queue_init(struct Queue* queue, const int size)
{
	queue_init_impl(queue, size);
}

void queue_cleanup(struct Queue* queue)
{
	queue_cleanup_impl(queue);
}

void queue_push(struct Queue* queue, queuedata_t* data)
{
	// pthread_mutex_lock(&(queue->lock));
	queue_push_impl(queue, data);
	// pthread_mutex_unlock(&(queue->lock));
}

void queue_pop(struct Queue* queue)
{
	// pthread_mutex_lock(&(queue->lock));
	queue_pop_impl(queue);
	// pthread_mutex_unlock(&(queue->lock));
}

bool queue_isFull(const struct Queue* queue)
{
	return queue_isFull_impl(queue);
}

bool queue_isEmpty(const struct Queue* queue)
{
	return queue_isEmpty_impl(queue);
}

queuedata_t* queue_peek(const struct Queue* queue)
{
	return queue_peek_impl(queue);
}

/************************************/
/********** Implementation **********/

static int inline queue_increment(const int n, const struct Queue* q) {
	return (n + 1) % (q->maxSize);
}

static void queue_init_impl(struct Queue* queue, const int size)
{
	queue->data = malloc(sizeof(queuedata_t) * size);
	queue->maxSize = size;
	queue->size = 0;
	queue->start = 0;
	queue->end = 0;
	pthread_mutex_init(&(queue->lock), NULL);
}

static void queue_cleanup_impl(struct Queue* queue)
{
	free(queue->data);
	queue->maxSize = 0;
	queue->size = 0;
	pthread_mutex_destroy(&(queue->lock));
}

static void queue_push_impl(struct Queue* queue, queuedata_t* data)
{
	if(queue_isFull(queue)) {
		printf("asdf\n");
		return;
	}
	queue->size += 1;
	queue->data[queue->end] = data;
	queue->end = queue_increment(queue->end, queue);
}

static void queue_pop_impl(struct Queue* queue)
{
	if(queue_isEmpty(queue)) {
		return;
	}
	queue->size -= 1;
	queue->start = queue_increment(queue->start, queue);
}

static bool queue_isFull_impl(const struct Queue* queue)
{
	return queue->size == queue->maxSize;
}

static bool queue_isEmpty_impl(const struct Queue* queue)
{
	return queue->size == 0;
}

static queuedata_t* queue_peek_impl(const struct Queue* queue)
{
	if(queue_isEmpty(queue)) {
		return NULL;
	}
	return queue->data[queue->start];
}
#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

#include "wave.h"

typedef wavedata_t queuedata_t;

struct Queue {
	queuedata_t** data;
	pthread_mutex_t lock;
	int maxSize;
	int size;
	int start;
	int end;
};

void queue_init(struct Queue* queue, const int size);
void queue_cleanup(struct Queue* queue);
void queue_push(struct Queue* queue, queuedata_t* data);
void queue_pop(struct Queue* queue);
bool queue_isFull(const struct Queue* queue);
bool queue_isEmpty(const struct Queue* queue);
queuedata_t* queue_peek(const struct Queue* queue);

#endif
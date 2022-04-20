#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "joystick.h"
#include "gpio.h"

/* --------------------------------------------- */
/* ---------- static global variables ---------- */

static int8_t JOYSTICK_PINS[JOYSTICK_NUM_DIRECTIONS];

struct joystick_EpollStruct
{
	enum joystick_Direction direction;
	enum joystick_Value value;
};


/* --------------------------------------------- */
/* ---------- static function headers ---------- */

static int joystick_initImpl(struct Joystick*
	,void (*actionFunc)(enum joystick_Direction direction, enum joystick_Value value));

static void joystick_cleanupImpl(struct Joystick*);

static int joystick_exportGPIO(enum joystick_Direction);
static int joystick_initEdgeTrigger(struct Joystick*, enum joystick_Direction);
static int joystick_initFD(struct Joystick*, enum joystick_Direction);
static int joystick_initEpoll(struct Joystick*, enum joystick_Direction);
static void* joystick_threadFunc(void*);

/* ----------------------------------------- */
/* ---------- interface functions ---------- */

int joystick_init(struct Joystick* jstick
	,void (*actionFunc)(enum joystick_Direction direction, enum joystick_Value value))
{
	JOYSTICK_PINS[JOYSTICK_UP] = 26;
	JOYSTICK_PINS[JOYSTICK_CENTER] = 27;
	JOYSTICK_PINS[JOYSTICK_DOWN] = 46;
	JOYSTICK_PINS[JOYSTICK_RIGHT] = 47;
	JOYSTICK_PINS[JOYSTICK_LEFT] = 65;
	return joystick_initImpl(jstick, actionFunc);
}


void joystick_cleanup(struct Joystick* jstick)
{
	joystick_cleanupImpl(jstick);
}

/* -------------------------------------------- */
/* ---------- static function bodies ---------- */

static int joystick_initImpl(struct Joystick* jstick
	,void (*actionFunc)(enum joystick_Direction direction, enum joystick_Value value))
{
	int epFD = epoll_create(1);
	if (epFD == -1) {
		return -1;
	}
	jstick->epollFD = epFD;
	for(int i = 0; i < JOYSTICK_NUM_DIRECTIONS; i++) {
		if(joystick_exportGPIO(i) == -1) {
			return -1;
		}
		if(joystick_initFD(jstick, i) == -1) {
			return -1;
		}
		if(joystick_initEdgeTrigger(jstick, i) == -1) {
			return -1;
		}
		if(joystick_initEpoll(jstick, i) == -1) {
			return -1;
		}
	}
	jstick->actionFunc = actionFunc;
	pthread_create(&(jstick->thread), NULL, joystick_threadFunc, jstick);
	return 0;
}

static void joystick_cleanupImpl(struct Joystick* jstick)
{
	pthread_cancel(jstick->thread);
	close(jstick->epollFD);
	for (uint32_t i = 0; i < JOYSTICK_NUM_DIRECTIONS; i++) {
		close(jstick->valueFDs[i]);
		free(jstick->interestEvents[i].data.ptr);
	}
}

static int joystick_exportGPIO(enum joystick_Direction dir) {
	if(gpio_export(JOYSTICK_PINS[dir]) == GPIO_ERR) {
		return -1;
	}
	return 0;
}

static int joystick_initFD(struct Joystick* jstick, enum joystick_Direction dir)
{
	int fd = gpio_open(JOYSTICK_PINS[dir], GPIO_VALUE, O_RDONLY | O_NONBLOCK);
	if (fd == -1) {
		return -1;
	}
	jstick->valueFDs[dir] = fd;
	return 0;
}

static int joystick_initEdgeTrigger(struct Joystick* jstick, enum joystick_Direction dir)
{
	if(gpio_write(JOYSTICK_PINS[dir], GPIO_EDGE, GPIO_BOTH) != GPIO_OK) {
		return -1;
	}
	return 0;
}

static int joystick_initEpoll(struct Joystick* jstick, enum joystick_Direction dir)
{
	struct epoll_event *interestEvent;
	struct epoll_event trash[1];
	struct joystick_EpollStruct *tmp;
	int result;
	tmp = malloc(sizeof(struct joystick_EpollStruct));
	tmp->direction = dir;
	tmp->value = JOYSTICK_RELEASED;
	interestEvent = jstick->interestEvents + dir;
	interestEvent->events = EPOLLIN | EPOLLET | EPOLLPRI;
	interestEvent->data.fd = jstick->valueFDs[dir];
	interestEvent->data.ptr = tmp;
	result = epoll_ctl(jstick->epollFD, EPOLL_CTL_ADD, jstick->valueFDs[dir], interestEvent);
	if (result == -1) {
		return -1;
	}
	epoll_wait(jstick->epollFD, trash, 1, 0); // Ignore first trigger
	return 0;
}

static void joystick_toggleVal(enum joystick_Value* val) {
	*val = (*val + 1) % JOYSTICK_NVALS;
}

static void* joystick_threadFunc(void* arg)
{
	#define MAX_READY 1
	static const int32_t timeout = -1;
	struct Joystick* jstick = (struct Joystick*)arg;
	struct joystick_EpollStruct* eventData;
	struct epoll_event readyList[MAX_READY];

	jstick->isRunning = true;
	while(jstick->isRunning) {
		if(epoll_wait(jstick->epollFD, readyList, MAX_READY, timeout) == -1) {
			jstick->actionFunc(JOYSTICK_READ_ERR, JOYSTICK_RELEASED);
		} else {
			eventData = readyList->data.ptr;
			joystick_toggleVal(&(eventData->value));
			jstick->actionFunc(eventData->direction, eventData->value);
		}
	}
	return NULL;
}
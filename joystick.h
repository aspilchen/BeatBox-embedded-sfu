#ifndef JOYSTICK_H
#define JOYSTICK_H

/*
 * The joystick module provides an interface to wait for and recieve input from 
 * the joystick.
 * 
 * Designed for the Zen Cape attachment to the Beagle Bone.
 * Uses default computer architecture provided in cmpt433 at Simon Fraser 
 * University.
 * 
 * During initizliation, the function passed as an argument will be called when
 * a joystick event is detected. The function called will be passed the direction
 * and value (pressed or released) and the user may do what they wish with that data.
 */

#include <sys/epoll.h>
#include <stdint.h>
#include <stdbool.h>

enum joystick_Direction
{
	JOYSTICK_UP
	,JOYSTICK_DOWN
	,JOYSTICK_LEFT
	,JOYSTICK_RIGHT
	,JOYSTICK_CENTER
	,JOYSTICK_NUM_DIRECTIONS
	,JOYSTICK_READ_ERR
};

enum joystick_Value
{
	JOYSTICK_PRESSED
	,JOYSTICK_RELEASED
	,JOYSTICK_NVALS
};

struct Joystick {
	struct epoll_event interestEvents[JOYSTICK_NUM_DIRECTIONS];
	int valueFDs[JOYSTICK_NUM_DIRECTIONS];
	pthread_t thread;
	int epollFD;
	bool isRunning;
	void (*actionFunc)(enum joystick_Direction direction, enum joystick_Value value);
};

// actionFunc is a function pointer defined by the user of this module. It will be called whenever
// a joystick event occurrs.
int joystick_init(struct Joystick* jstick
	,void (*actionFunc)(enum joystick_Direction direction, enum joystick_Value value));

void joystick_cleanup(struct Joystick* jstick);

#endif
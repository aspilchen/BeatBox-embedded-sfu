#include <pthread.h>

#include "joystick.h"
#include "UI.h"

#define N_ACTIONS 5

static void ui_jstickInitImpl(void);
static void ui_jstickCleanupImpl(void);
static void ui_joystickAction(const enum joystick_Direction direction, const enum joystick_Value value);

static void (*actionList[N_ACTIONS]) (void);
static struct Joystick jstick;

void ui_jstickInit(void)
{
	ui_jstickInitImpl();
}

void ui_jstickCleanup(void)
{
	ui_jstickCleanupImpl();
}

static void ui_jstickInitImpl(void)
{
	actionList[JOYSTICK_UP] = ui_incrementVolume;
	actionList[JOYSTICK_DOWN] = ui_decrementVolume;
	actionList[JOYSTICK_LEFT] = ui_decrementTempo;
	actionList[JOYSTICK_RIGHT] = ui_incrementTempo;
	actionList[JOYSTICK_CENTER] = ui_cycleRythm;
	joystick_init(&jstick, ui_joystickAction);
}

static void ui_jstickCleanupImpl(void)
{
	joystick_cleanup(&jstick);
}

static void ui_joystickAction(const enum joystick_Direction direction, const enum joystick_Value value)
{
	if(value == JOYSTICK_PRESSED) {
		actionList[direction]();
	}
}
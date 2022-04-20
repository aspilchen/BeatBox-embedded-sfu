#include <time.h>
#include <stdio.h>

#include "UI.h"
#include "ui_jstick.h"
#include "ui_udp.h"
#include "drum_machine.h"

static void ui_initImpl();
static void ui_cleanupImpl(void);
static void ui_setVolumeImpl(const uint32_t);
static void ui_setTempoImpl(const uint32_t);
static void ui_cycleRythmImpl(void);
static void ui_setRythmImpl(const uint32_t);
static const char* ui_getRythmNameImpl(const uint32_t);
static uint32_t ui_nRythmsImpl();

static const uint32_t VOLUME_CHANGE = 5;
static const uint32_t TEMPO_CHANGE = 5;

void ui_init(void)
{
	ui_initImpl();
}

void ui_cleanup(void)
{
	ui_cleanupImpl();
}


void ui_incrementVolume(void)
{
	uint32_t volume = drum_getVolume();
	ui_setVolumeImpl(volume + VOLUME_CHANGE);
}

void ui_decrementVolume(void)
{
	uint32_t volume = drum_getVolume();
	ui_setVolumeImpl(volume - VOLUME_CHANGE);
}

void ui_setVolume(const uint32_t volume)
{
	ui_setVolumeImpl(volume);
}

void ui_incrementTempo(void)
{
	uint32_t tempo = drum_getTempo();
	ui_setTempoImpl(tempo + TEMPO_CHANGE);
}

void ui_decrementTempo(void)
{
	uint32_t tempo = drum_getTempo();
	ui_setTempoImpl(tempo - TEMPO_CHANGE);
}

void ui_setTempo(const uint32_t tempo)
{
	ui_setTempoImpl(tempo);
}

void ui_cycleRythm(void)
{
	ui_cycleRythmImpl();
}

void ui_setRythm(const uint32_t id)
{
	ui_setRythmImpl(id);
}

const char* ui_getRythmName(uint32_t id)
{
	return ui_getRythmNameImpl(id);
}

uint32_t ui_nRythms(void)
{
	return ui_nRythmsImpl();
}


static void ui_initImpl(void)
{
	ui_jstickInit();
	ui_udpInit();
}

void ui_cleanupImpl(void)
{
	ui_jstickCleanup();
}

static void ui_setVolumeImpl(const uint32_t volume)
{
	uint32_t vol;
	drum_setVolume(volume);
	vol = drum_getVolume();
	printf("volume: %d\n", vol);
}

static void ui_setTempoImpl(const uint32_t tempo)
{
	drum_setTempo(tempo);
	printf("Tempo: %d\n", tempo);
}

static void ui_cycleRythmImpl(void)
{
	uint32_t id = drum_getCurrRythm();
	id = (id + 1) % N_RYTHM;
	ui_setRythmImpl(id);
}

static void ui_setRythmImpl(const uint32_t id)
{
	uint32_t newId;
	drum_setRythm(id);
	newId = drum_getCurrRythm();
	const char* rythm = drum_getRythmName(newId);
	printf("Mode: %s\n", rythm);
}

static const char* ui_getRythmNameImpl(const uint32_t id)
{
	return drum_getRythmName(id);
}

static uint32_t ui_nRythmsImpl()
{
	return N_RYTHM;
}
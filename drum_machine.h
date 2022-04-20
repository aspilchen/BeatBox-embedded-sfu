#ifndef DRUM_H
#define DRUM_H
/**
 * Drum machine has a few auto-play beats or just nothing to select from.
 * Available wave sounds are stored in memory and users can play them 
 * by using the playWave function and passing it the drum_Wave you wish
 * to play.
 * 
 * Begins playing immediately once initialized.
 */

#include <stdint.h>

#define N_RYTHM 3

static const uint32_t DRUM_MAX_VOL = 100;
static const uint32_t DRUM_MIN_VOL = 0;
static const uint32_t DRUM_MIN_TEMPO = 40;
static const uint32_t DRUM_MAX_TEMPO = 300;

enum drum_Wave {
	BASS
	,HAT
	,SNARE
	,N_WAVES
};

void drum_init(void);
void drum_cleanup(void);

void drum_setVolume(const uint32_t newVol);
uint32_t drum_getVolume(void);

void drum_setTempo(const uint32_t newTemp);
uint32_t drum_getTempo(void);

const char* drum_getRythmName(uint32_t id);
uint32_t drum_getCurrRythm(void);
void drum_setRythm(const uint32_t id);

const char* drum_getWaveName(enum drum_Wave id);
void drum_playWave(enum drum_Wave id);


#endif
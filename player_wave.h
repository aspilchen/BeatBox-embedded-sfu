#ifndef PLAYER_WAVE_H
#define PLAYER_WAVE_H

/**
 * Plays wave files and allows users to set the volume.
 */

#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "wave.h"

static const int PLAYER_MAX_VOLUME = 100;

struct Player {
	bool isRunning;
	uint32_t volume;
	pthread_t thread;
	snd_pcm_t* pcm;
	wavedata_t* (*loader)(void);
};


void player_init(struct Player*, wavedata_t* (*loader)(void));
void player_setVolume(struct Player*, uint32_t);
uint32_t player_getVolume(const struct Player*);
void player_readIn(struct Player*, wavedata_t*);
void player_cleanup(struct Player*);

#endif
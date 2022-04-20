#include <stdio.h>
#include <alloca.h>

#include "player_wave.h"

#define SAMPLE_RATE   44100
#define NUM_CHANNELS  1

static void player_initImpl(struct Player*, wavedata_t* (*loader)(void));
static void player_cleanupImpl(struct Player*);
static void player_setVolumeImpl(struct Player*, uint32_t);
static uint32_t player_getVolumeImpl(const struct Player*);
static snd_pcm_t *player_openDevice();
static void player_playWave(snd_pcm_t *handle, wavedata_t *pWaveData);
static void* player_threadFunc(void* arg);

void player_init(struct Player* player, wavedata_t* (*loader)(void))
{
	player_initImpl(player, loader);
}

void player_setVolume(struct Player* player, uint32_t volume)
{
	player_setVolumeImpl(player, volume);
}

uint32_t player_getVolume(const struct Player* player)
{
	return player_getVolumeImpl(player);
}

void player_cleanup(struct Player* player)
{
	player_cleanupImpl(player);
}

static void player_initImpl(struct Player* player, wavedata_t* (*loader)(void))
{
	player->pcm = player_openDevice();
	player->loader = loader;
	pthread_create(&(player->thread), NULL, player_threadFunc, player);
}

static void player_cleanupImpl(struct Player* player)
{
	player->isRunning = false;
	pthread_cancel(player->thread);
	snd_pcm_drain(player->pcm);
	snd_pcm_hw_free(player->pcm);
	snd_pcm_close(player->pcm);
}

static void* player_threadFunc(void* arg)
{
	struct Player* player = (struct Player*)(arg);
	wavedata_t* wave;
	player->isRunning = true;
	while(player->isRunning) {
		wave = player->loader();
		while(wave) {
			player_playWave(player->pcm, wave);
			wave = player->loader();
		}
	}
	return NULL;
}

static snd_pcm_t *player_openDevice()
{
	snd_pcm_t *handle;
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Play-back open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	err = snd_pcm_set_params(handle
			,SND_PCM_FORMAT_S16_LE
			,SND_PCM_ACCESS_RW_INTERLEAVED
			,NUM_CHANNELS
			,SAMPLE_RATE
			,1
			,50000);
	if (err < 0) {
		printf("Play-back configuration error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	return handle;
}

void player_playWave(snd_pcm_t *handle, wavedata_t *pWaveData)
{
	fflush(stdout);
	snd_pcm_sframes_t frames = snd_pcm_writei(handle, pWaveData->pData, pWaveData->numSamples);
	if (frames < 0) {
		frames = snd_pcm_recover(handle, frames, 0);
	}
	if (frames < 0) {
		fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n", frames);
		exit(EXIT_FAILURE);
	}
	if (frames > 0 && frames < pWaveData->numSamples)
		printf("Short write (expected %d, wrote %li)\n", pWaveData->numSamples, frames);
}

static void player_setVolumeImpl(struct Player* player, uint32_t volume)
{
	if (volume < 0 || volume > PLAYER_MAX_VOLUME) {
		return;
	}
	player->volume = volume;
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);
    snd_mixer_close(handle);
}

static uint32_t player_getVolumeImpl(const struct Player* player)
{
	return player->volume;
}
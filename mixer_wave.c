
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "mixer_wave.h"

struct MixerChannel {
	wavedata_t* data;
	int offset;
	bool isActive;
};

static void mixer_initImpl(struct Mixer* mixer, int nChannels);
static void mixer_cleanupImpl(struct Mixer* mixer);
static bool mixer_addWaveImpl(struct Mixer* mixer, wavedata_t* wave);
static int mixer_nextImpl(wavedata_t* dest, struct Mixer* src);
static void mixer_mixChannel(wavedata_t* dest, struct MixerChannel* src);

void mixer_init(struct Mixer* mixer, int nChannels)
{
	mixer_initImpl(mixer, nChannels);
}

void mixer_cleanup(struct Mixer* mixer)
{
	mixer_cleanupImpl(mixer);
}

bool mixer_addWave(struct Mixer* mixer, wavedata_t* wave)
{
	return mixer_addWaveImpl(mixer, wave);
}

int mixer_next(wavedata_t* dest, struct Mixer* src)
{
	return mixer_nextImpl(dest, src);
}

static void mixer_initImpl(struct Mixer* mixer, int nChannels)
{
	mixer->channels = calloc(nChannels, sizeof(*(mixer->channels)));
	mixer->nChannels = nChannels;
	mixer->activeChannels = 0;
}

static void mixer_cleanupImpl(struct Mixer* mixer)
{
	if(mixer->channels) {
		free(mixer->channels);
	}
	mixer->channels = NULL;
	mixer->activeChannels = 0;
	mixer->nChannels = 0;
}

static bool mixer_addWaveImpl(struct Mixer* mixer, wavedata_t* wave)
{
	struct MixerChannel* channels = mixer->channels;
	for(int i = 0; i < mixer->nChannels; i++) {
		if(!channels[i].isActive) {
			channels[i].data = wave;
			channels[i].offset = 0;
			channels[i].isActive = true;
			return true;
		}
	}
	return false;
}

static int mixer_nextImpl(wavedata_t* dest, struct Mixer* src)
{
	struct MixerChannel* channels = src->channels;
	int channelsRead = 0;
	memset(dest->pData, 0, SAMPLE_SIZE * dest->numSamples);
	for(int i = 0; i < src->nChannels; i++) {
		if(channels[i].isActive) {
			mixer_mixChannel(dest, channels + i);
			channelsRead += 1;
		}
	}
	return channelsRead;
}

static void mixer_mixChannel(wavedata_t* dest, struct MixerChannel* channel)
{
	wavedata_t* srcWave = channel->data;
	int offset = channel->offset;
	int i = 0;
	int mixed; // Use int to prevent overflow
	while(i < dest->numSamples && offset < srcWave->numSamples) {
		mixed = dest->pData[i] + srcWave->pData[offset];
		// Ensure SHRT_MIN <= mixed <= SHRT_MAX to prevent overflow during cast.
		if(SHRT_MAX < mixed) {
			mixed = SHRT_MAX;
		} else if(mixed < SHRT_MIN) {
			mixed = SHRT_MIN;
		}
		dest->pData[i] = (short)mixed;
		i += 1;
		offset += 1;
	}
	if(offset == srcWave->numSamples) {
		channel->isActive = false;
	}
	channel->offset = offset;
}
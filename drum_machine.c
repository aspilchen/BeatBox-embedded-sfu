
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "drum_machine.h"
#include "bloader.h"
#include "mixer_wave.h"
#include "player_wave.h"

#define HAT_FILE    "beatbox-wav-files/100053__menegass__gui-drum-cc.wav"
#define BASS_FILE   "beatbox-wav-files/100051__menegass__gui-drum-bd-hard.wav"
#define SNARE_FILE  "beatbox-wav-files/100059__menegass__gui-drum-snare-soft.wav"

#define MIXERBUFF_SIZE 20

struct drum_Rythm {
	const char* name;
	void (*rythFunc)(const uint32_t);
};

static void drum_initImpl();
static void drum_cleanupImpl();

static uint32_t drum_getVolumeImpl();
static void drum_setVolumeImpl(const uint32_t);

static uint32_t drum_getTempoImpl();
static void drum_setTempoImpl(const uint32_t);

static uint32_t drum_getCurrRythmImpl(void);
static const char* drum_getRythmNameImpl(uint32_t);
static void drum_setRythmImpl(const uint32_t);

static void drum_emptyRythm(const uint32_t);
static void drum_myRythm(const uint32_t);
static void drum_rockRythm(const uint32_t);
static wavedata_t* drum_playerLoader(void);
static wavedata_t* drum_bloaderFunc(void);
static void* drum_threadFunc(void*);

static const char* drum_getWaveNameImpl(enum drum_Wave id);
static void drum_playWaveImpl(enum drum_Wave id);

static bool isRunning = false;
static uint32_t currTempo = 120;
static uint32_t currRythm = 0;
static struct timespec beatWait = {0,0};
static pthread_t thread;
static struct Mixer mixer;
static struct Bloader bloader;
static struct Player player;
static wavedata_t mixerBuff[MIXERBUFF_SIZE];

static wavedata_t WAVES[N_WAVES];
static char* WAVE_NAMES[N_WAVES] = {
	"Bass"
	,"Hi-hat"
	,"Snare"
};

static struct drum_Rythm RYTHMS[N_RYTHM] = {
	{"Rock", drum_rockRythm}
	,{"Personal", drum_myRythm}
	,{"None", drum_emptyRythm}
};

void drum_init()
{
	drum_initImpl();
}

void drum_cleanup()
{
	drum_cleanupImpl();
}

uint32_t drum_getVolume()
{
	return drum_getVolumeImpl();
}

uint32_t drum_getTempo()
{
	return drum_getTempoImpl();
}

void drum_setVolume(const uint32_t volume)
{
	drum_setVolumeImpl(volume);
}

void drum_setTempo(const uint32_t tempo)
{
	drum_setTempoImpl(tempo);
}

const char* drum_getRythmName(uint32_t id)
{
	return drum_getRythmNameImpl(id);
}

uint32_t drum_getCurrRythm(void)
{
	return drum_getCurrRythmImpl();
}

void drum_setRythm(const uint32_t id)
{
	drum_setRythmImpl(id);
}

const char* drum_getWaveName(enum drum_Wave id)
{
	return drum_getWaveNameImpl(id);
}

void drum_playWave(enum drum_Wave id)
{
	drum_playWaveImpl(id);
}

static wavedata_t* drum_bloaderFunc(void) {
	static uint8_t i = 0;
	wavedata_t* samples = NULL;
	mixer_next(mixerBuff+i, &mixer);
	samples = mixerBuff+i;
	i = (i+1) % (MIXERBUFF_SIZE);
	return samples;
}

static wavedata_t* drum_playerLoader(void) {
	return bloader_get(&bloader);
}

static void drum_initImpl()
{
	static const uint32_t MIXER_CHANNELS = 10;
	static const uint32_t BLOADER_SIZE = 10;
	static const uint32_t WAVE_CHUNKSIZE = 64;
	static const uint32_t INIT_VOLUME = 80;
	wave_readWaveFile(&WAVES[BASS], BASS_FILE);
	wave_readWaveFile(&WAVES[HAT], HAT_FILE);
	wave_readWaveFile(&WAVES[SNARE], SNARE_FILE);
	for(int i = 0; i < MIXERBUFF_SIZE; i++) {
		wave_init(mixerBuff+i, WAVE_CHUNKSIZE);
	}
	drum_setTempoImpl(currTempo);
	drum_setVolumeImpl(INIT_VOLUME);
	mixer_init(&mixer, MIXER_CHANNELS);
	bloader_init(&bloader, drum_bloaderFunc, BLOADER_SIZE);
	player_init(&player, drum_playerLoader);
	pthread_create(&thread, NULL, drum_threadFunc, NULL);
}

static void drum_cleanupImpl()
{
	isRunning = false;
	player_cleanup(&player);
	bloader_cleanup(&bloader);
	mixer_cleanup(&mixer);
	for(int i = 0; i < MIXERBUFF_SIZE; i++) {
		wave_destroy(mixerBuff+i);
	}
	pthread_cancel(thread);
}

static uint32_t drum_getVolumeImpl()
{
	return player_getVolume(&player);
}

static uint32_t drum_getTempoImpl()
{
	return currTempo;
}

static void drum_setVolumeImpl(const uint32_t volume)
{
	player_setVolume(&player, volume);
}

static void drum_setTempoImpl(const uint32_t tempo)
{
	const uint64_t SECOND_IN_NANOSEC = 1000000000;
	if(DRUM_MIN_TEMPO <= tempo && tempo <= DRUM_MAX_TEMPO) {
		double perSecond = ((double)tempo)/60;
		double waitFull = 1/perSecond;
		long bpsWait = waitFull * SECOND_IN_NANOSEC;
		beatWait.tv_nsec = bpsWait;
		currTempo = tempo;
	}
}

static uint32_t drum_getCurrRythmImpl(void)
{
	return currRythm;
}

static const char* drum_getRythmNameImpl(uint32_t id)
{
	if (0 <= id && id < N_RYTHM) {
		return RYTHMS[id].name;
	}
	return NULL;
}

static void drum_setRythmImpl(const uint32_t id)
{
	const uint64_t MILISEC_IN_NANOSEC = 100000000;
	const struct timespec req = {0, MILISEC_IN_NANOSEC};
	struct timespec rem;
	if (0 <= id && id < N_RYTHM) {
		currRythm = id;
		nanosleep(&req, &rem); // wait about 1ms to drain buffer
	}
}

static const char* drum_getWaveNameImpl(enum drum_Wave id)
{
	if(0 <= id && id < N_WAVES) {
		return WAVE_NAMES[id];
	}
	return NULL;
}


static void drum_playWaveImpl(enum drum_Wave id)
{
	if(0 <= id && id < N_WAVES) {
		mixer_addWave(&mixer, WAVES + id);
	}
}

static void* drum_threadFunc(void* arg)
{
	static const uint32_t MAX_BEATS = 8;
	uint32_t beat = 0;
	isRunning = true;
	while(isRunning) {
		RYTHMS[currRythm].rythFunc(beat);
		beat = (beat + 1) % MAX_BEATS;
	}
	return NULL;
}

static void drum_emptyRythm(const uint32_t beat) {
	static struct timespec rem;
	nanosleep(&beatWait, &rem);
}

static void drum_myRythm(const uint32_t beat) {
	static uint32_t i = 0;
	double qTime = beatWait.tv_nsec / 4;
	struct timespec quarterBeat = {0, (long)qTime};
	static struct timespec rem;
	switch (beat%4)
	{
	case 0:
		mixer_addWave(&mixer, WAVES + BASS);
		nanosleep(&quarterBeat, &rem);
		break;
	case 1:
		mixer_addWave(&mixer, WAVES + BASS);
		nanosleep(&quarterBeat, &rem);
		break;
	case 2:
		if (i == 0) {
			mixer_addWave(&mixer, WAVES + SNARE);
		}
		mixer_addWave(&mixer, WAVES + HAT);
		i = (i+1) % 2;
		nanosleep(&quarterBeat, &rem);
		break;
	case 3:
		mixer_addWave(&mixer, WAVES + BASS);
		nanosleep(&quarterBeat, &rem);
		break;
	default:
		break;
	}
}

static void drum_rockRythm(const uint32_t beat) {
	static struct timespec quarterBeat = {0, 0};
	static struct timespec rem;
	quarterBeat.tv_nsec = beatWait.tv_nsec / 4;
	switch (beat%4)
	{
	case 0:
		mixer_addWave(&mixer, WAVES + BASS);
		mixer_addWave(&mixer, WAVES + HAT);
		nanosleep(&quarterBeat, &rem);
		break;
	case 1:
		mixer_addWave(&mixer, WAVES + HAT);
		nanosleep(&quarterBeat, &rem);
		break;
	case 2:
		mixer_addWave(&mixer, WAVES + HAT);
		mixer_addWave(&mixer, WAVES + SNARE);
		nanosleep(&quarterBeat, &rem);
		break;
	case 3:
		mixer_addWave(&mixer, WAVES + HAT);
		nanosleep(&quarterBeat, &rem);
		break;
	default:
		break;
	}
}
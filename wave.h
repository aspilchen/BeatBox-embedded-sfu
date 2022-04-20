#ifndef WAVE_H
#define WAVE_H

#define SAMPLE_SIZE   (sizeof(short)) 	// bytes per sample

typedef struct {
	int numSamples;
	short *pData;
} wavedata_t;

void wave_init(wavedata_t* wave, int size);
int wave_readWaveFile(wavedata_t *wave, char *fileName);
void wave_destroy(wavedata_t* wave);
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "wave.h"


void wave_init(wavedata_t* wave, int size)
{
	wave->pData = malloc(SAMPLE_SIZE * size);
	wave->numSamples = size;
}

// Read in the file to dynamically allocated memory.
// !! Client code must free memory in wavedata_t !!
int wave_readWaveFile(wavedata_t *wave, char *fileName)
{
	assert(wave);

	// Wave file has 44 bytes of header data. This code assumes file
	// is correct format.
	const int DATA_OFFSET_INTO_WAVE = 44;

	// Open file
	FILE *file = fopen(fileName, "r");
	if (file == NULL) {
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		fclose(file);
		return -1;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - DATA_OFFSET_INTO_WAVE;
	fseek(file, DATA_OFFSET_INTO_WAVE, SEEK_SET);
	wave->numSamples = sizeInBytes / SAMPLE_SIZE;

	// Allocate Space
	wave->pData = malloc(sizeInBytes);
	if (wave->pData == NULL) {
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		fclose(file);
		return -1;
	}

	// Read data:
	int samplesRead = fread(wave->pData, SAMPLE_SIZE, wave->numSamples, file);
	if (samplesRead != wave->numSamples) {
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				wave->numSamples, fileName, samplesRead);
		fclose(file);
		return -1;
	}

	fclose(file);
	return 0;
}

void wave_destroy(wavedata_t* wave)
{
	free(wave->pData);
	wave->pData = NULL;
	wave->numSamples = 0;
}
#ifndef MIXER_H
#define MIXER_H

/**
 * mixer_wave is a module to mix wave files.
 * 
 * A Mixer will have nChannels after init. nChannels is the maximum
 * number of wave files that can be mixed at a single time.
 * 
 * Use mixer_addWave to insert a new wave file for mixing.
 * Use mixer_next to extract mixed wave data.
 */

#include "wave.h"
#include <pthread.h>
#include <stdbool.h>

struct Mixer {
	struct MixerChannel* channels;
	int activeChannels;
	int nChannels;
};

// nChannels is the maximum number of wave files that can be mixed at once.
void mixer_init(struct Mixer* mixer, int nChannels);
void mixer_cleanup(struct Mixer* mixer);

// Inserts a new wave file to be mixed with the current waves.
// Returns true if successful. Returns false if it was unable to add the new file.
// A false return is likely the result of all channels being in use.
bool mixer_addWave(struct Mixer* mixer, wavedata_t* wave);

// Fills dest with mixed samples.
// Existing sound data in dest will be lost.
// Any call to mixer_next when there are no waves queued for mixing will result
// in a return of 0 (0 channels mixed) and dest will contain all 0s.
// PRE:
//     dest must be inilitized.
// RETURN:
//     returns the number of channels (aka wave files) mixed into dest.
// EXAMPLE:
//   wavedata_t* tmp;
//   while(mixer_next(tmp, &mixer)) {
//       do_something(tmp);
//   }
int mixer_next(wavedata_t* dest, struct Mixer* src);

#endif
#ifndef PHYSICAL_H
#define PHYSICAL_H

#include <stdint.h>

void ui_init(void);
void ui_cleanup(void);
void ui_incrementVolume(void);
void ui_decrementVolume(void);
void ui_setVolume(const uint32_t volume);
void ui_incrementTempo(void);
void ui_decrementTempo(void);
void ui_setTempo(const uint32_t tempo);
void ui_cycleRythm(void);
void ui_setRythm(const uint32_t id);
const char* ui_getRythmName(uint32_t id);
uint32_t ui_nRythms(void);


#endif
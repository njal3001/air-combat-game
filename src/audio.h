#pragma once
#include <stdbool.h>

bool audio_init();
void audio_shutdown();

void audio_play(const char *name);
void audio_set_volume(float val);
void audio_mute();

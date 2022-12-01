#pragma once
#include <stdbool.h>
#include "asset.h"

bool audio_init();
void audio_shutdown();

void audio_play(enum asset_audio handle);
void audio_set_volume(float val);
void audio_mute();

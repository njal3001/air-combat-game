#pragma once
#include <stdint.h>

void timer_init();
void timer_preupdate();
void timer_postupdate();

float timer_delta();
uint32_t timer_ticks();
float timer_elapsed();
uint32_t timer_fps();

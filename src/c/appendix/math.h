#pragma once

#include <pebble.h>

void min_max(int16_t *array, int n, int *min, int *max);

int f_to_c(int temp_f);

int get_dew_point(int temp_f, int rh);

uint32_t gcolor_to_hex(GColor color);

const char* get_wind_direction_string(int degrees);
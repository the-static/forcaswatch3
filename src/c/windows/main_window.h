#pragma once

#include <pebble.h>

void main_window_create();

void main_window_refresh();

int16_t main_window_get_top_content(void);

void main_window_set_clay_active(bool active);

void main_window_destroy();
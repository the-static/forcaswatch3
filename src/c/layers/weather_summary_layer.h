#pragma once

#include <pebble.h>

void weather_summary_layer_create(Layer* parent_layer, GRect frame);
void weather_summary_layer_refresh();
void weather_summary_layer_destroy();

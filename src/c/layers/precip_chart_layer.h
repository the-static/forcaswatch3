#pragma once
#include <pebble.h>

void precip_chart_layer_create(Layer *parent_layer, GRect frame);
void precip_chart_layer_destroy();
void precip_chart_layer_refresh();

#pragma once

#include <pebble.h>

#ifndef PBL_PLATFORM_APLITE
void pws_layer_create(Layer* parent_layer, GRect frame);
void pws_layer_refresh();
void pws_layer_destroy();
#else
#define pws_layer_create(parent_layer, frame)
#define pws_layer_refresh()
#define pws_layer_destroy()
#endif

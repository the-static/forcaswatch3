#include "weather_summary_layer.h"
#include "c/appendix/persist.h"
#include "c/appendix/config.h"

static Layer *s_weather_summary_layer;
static TextLayer *s_city_layer;
static TextLayer *s_detail_layer;
static TextLayer *s_temp_layer;

static void weather_summary_layer_update_proc(Layer *layer, GContext *ctx) {
    // Optional: Draw some background or lines if needed
}

void weather_summary_layer_create(Layer* parent_layer, GRect frame) {
    s_weather_summary_layer = layer_create(frame);
    GRect bounds = layer_get_bounds(s_weather_summary_layer);

    s_city_layer = text_layer_create(GRect(0, 0, bounds.size.w, 18));
    text_layer_set_background_color(s_city_layer, GColorClear);
    text_layer_set_text_color(s_city_layer, GColorWhite);
    text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);

    s_temp_layer = text_layer_create(GRect(0, 18, bounds.size.w, 24));
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);

    s_detail_layer = text_layer_create(GRect(0, 42, bounds.size.w, 14));
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, GColorWhite);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);

    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_city_layer));
    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_temp_layer));
    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_detail_layer));

    layer_set_update_proc(s_weather_summary_layer, weather_summary_layer_update_proc);
    layer_add_child(parent_layer, s_weather_summary_layer);

    weather_summary_layer_refresh();
}

void weather_summary_layer_refresh() {
    if (!s_weather_summary_layer) return;

    static char s_city_buffer[32];
    persist_get_city(s_city_buffer, sizeof(s_city_buffer));
    text_layer_set_text(s_city_layer, s_city_buffer);

    static char s_temp_buffer[16];
    snprintf(s_temp_buffer, sizeof(s_temp_buffer), "%d°", config_localize_temp(persist_get_current_temp()));
    text_layer_set_text(s_temp_layer, s_temp_buffer);

    static char s_detail_buffer[32];
    snprintf(s_detail_buffer, sizeof(s_detail_buffer), "H: %d°  L: %d°",
             config_localize_temp(persist_get_temp_hi()),
             config_localize_temp(persist_get_temp_lo()));
    text_layer_set_text(s_detail_layer, s_detail_buffer);

    layer_mark_dirty(s_weather_summary_layer);
}

void weather_summary_layer_destroy() {
    if (!s_weather_summary_layer) return;
    text_layer_destroy(s_city_layer);
    text_layer_destroy(s_temp_layer);
    text_layer_destroy(s_detail_layer);
    layer_destroy(s_weather_summary_layer);
    s_weather_summary_layer = NULL;
}

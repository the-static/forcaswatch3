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

static const char* wind_direction(int degrees) {
    static const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int index = ((degrees + 22) / 45) % 8;
    return directions[index];
}

void weather_summary_layer_create(Layer* parent_layer, GRect frame) {
    s_weather_summary_layer = layer_create(frame);
    GRect bounds = layer_get_bounds(s_weather_summary_layer);

    s_city_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20)); // Used for Date now
    text_layer_set_background_color(s_city_layer, GColorClear);
    text_layer_set_text_color(s_city_layer, GColorWhite);
    text_layer_set_font(s_city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);

    s_temp_layer = text_layer_create(GRect(0, 20, bounds.size.w, 24)); // Used for Wind
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);

    s_detail_layer = text_layer_create(GRect(0, 40, bounds.size.w, 18)); // Used for Humidity
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, GColorWhite);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
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

    static char s_date_buffer[16];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tm_now);
    text_layer_set_text(s_city_layer, s_date_buffer);

    static char s_wind_buffer[24];
    int speed = persist_get_wind_speed();
    const char* unit = g_config->celsius ? "m/s" : "mph"; // Simplification
    snprintf(s_wind_buffer, sizeof(s_wind_buffer), "Wind: %d %s %s", 
             speed, unit, wind_direction(persist_get_wind_deg()));
    text_layer_set_text(s_temp_layer, s_wind_buffer);

    static char s_hum_buffer[16];
    snprintf(s_hum_buffer, sizeof(s_hum_buffer), "Humidity: %d%%", persist_get_humidity());
    text_layer_set_text(s_detail_layer, s_hum_buffer);

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

#include "weather_summary_layer.h"
#include "c/appendix/persist.h"
#include "c/appendix/config.h"

static Layer *s_weather_summary_layer;
static TextLayer *s_detail_layer;
static TextLayer *s_temp_layer;

static const GPathInfo WIND_ARROW_POINTS = {
  .num_points = 4,
  .points = (GPoint []) {{0, -12}, {8, 12}, {0, 5}, {-8, 12}}
};
static GPath *s_wind_arrow_path;

static void weather_summary_layer_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    
    // Draw wind arrow
    int wind_deg = persist_get_wind_deg();
    s_wind_arrow_path = gpath_create(&WIND_ARROW_POINTS);
    // Position it to the right of the wind text
    gpath_move_to(s_wind_arrow_path, GPoint(bounds.size.w - 15, 12)); 
    gpath_rotate_to(s_wind_arrow_path, DEG_TO_TRIGANGLE(wind_deg));
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_wind_arrow_path);
    gpath_destroy(s_wind_arrow_path);
}

// Removed wind_direction string helper as we use an arrow now

void weather_summary_layer_create(Layer* parent_layer, GRect frame) {
    s_weather_summary_layer = layer_create(frame);
    GRect bounds = layer_get_bounds(s_weather_summary_layer);

    s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, 24)); // Used for Wind
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);

    s_detail_layer = text_layer_create(GRect(0, 20, bounds.size.w, 18)); // Used for Humidity
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, GColorWhite);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);

    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_temp_layer));
    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_detail_layer));

    layer_set_update_proc(s_weather_summary_layer, weather_summary_layer_update_proc);
    layer_add_child(parent_layer, s_weather_summary_layer);

    weather_summary_layer_refresh();
}

void weather_summary_layer_refresh() {
    if (!s_weather_summary_layer) return;

    static char s_wind_buffer[24];
    int speed = persist_get_wind_speed();
    int gust = persist_get_wind_gust();
    const char* unit = g_config->celsius ? "m/s" : "mph";
    if (gust > speed) {
        snprintf(s_wind_buffer, sizeof(s_wind_buffer), "Wind: %d(%d) %s", 
                 speed, gust, unit);
    } else {
        snprintf(s_wind_buffer, sizeof(s_wind_buffer), "Wind: %d %s", 
                 speed, unit);
    }
    text_layer_set_text(s_temp_layer, s_wind_buffer);
    
    // Reposition wind text to make room for arrow
    GRect bounds = layer_get_bounds(s_weather_summary_layer);
    layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(0, 0, bounds.size.w - 20, 24));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);

    static char s_hum_buffer[16];
    snprintf(s_hum_buffer, sizeof(s_hum_buffer), "Humidity: %d%%", persist_get_humidity());
    text_layer_set_text(s_detail_layer, s_hum_buffer);

    layer_mark_dirty(s_weather_summary_layer);
}

void weather_summary_layer_destroy() {
    if (!s_weather_summary_layer) return;
    text_layer_destroy(s_temp_layer);
    text_layer_destroy(s_detail_layer);
    layer_destroy(s_weather_summary_layer);
    s_weather_summary_layer = NULL;
}

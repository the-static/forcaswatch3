#include "weather_summary_layer.h"
#include "c/appendix/persist.h"
#include "c/appendix/config.h"

static Layer *s_weather_summary_layer;
static TextLayer *s_detail_layer;
static TextLayer *s_temp_layer;

#if defined(PBL_PLATFORM_EMERY)
#define WIND_ARROW_SCALE_X 11
#define WIND_ARROW_SCALE_Y 16
#define WIND_ARROW_MID_Y 7
#define MOON_RADIUS 9
#define MOON_X_OFFSET 20
#define MOON_Y_OFFSET 42
#define ARROW_Y_OFFSET 16
#else
#define WIND_ARROW_SCALE_X 8
#define WIND_ARROW_SCALE_Y 12
#define WIND_ARROW_MID_Y 5
#define MOON_RADIUS 6
#define MOON_X_OFFSET 15
#define MOON_Y_OFFSET 30
#define ARROW_Y_OFFSET 12
#endif

static const GPathInfo WIND_ARROW_POINTS = {
  .num_points = 4,
  .points = (GPoint []) {{0, -WIND_ARROW_SCALE_Y}, {WIND_ARROW_SCALE_X, WIND_ARROW_SCALE_Y}, {0, WIND_ARROW_MID_Y}, {-WIND_ARROW_SCALE_X, WIND_ARROW_SCALE_Y}}
};
static GPath *s_wind_arrow_path;

static void weather_summary_layer_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    
    // Draw wind arrow
    int wind_deg = persist_get_wind_deg();
    s_wind_arrow_path = gpath_create(&WIND_ARROW_POINTS);
    // Position it to the right of the wind text
    gpath_move_to(s_wind_arrow_path, GPoint(bounds.size.w - MOON_X_OFFSET, ARROW_Y_OFFSET)); 
    gpath_rotate_to(s_wind_arrow_path, DEG_TO_TRIGANGLE(wind_deg));
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_wind_arrow_path);
    gpath_destroy(s_wind_arrow_path);

    // Draw moon phase
    time_t t = time(NULL);
    long diff = ((long)t - 592200);
    while (diff < 0) diff += 2551443;
    int phase = (diff % 2551443) * 28 / 2551443; // 0 to 27
    
    GPoint moon_center = GPoint(bounds.size.w - MOON_X_OFFSET, MOON_Y_OFFSET);
    int r = MOON_RADIUS;
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_circle(ctx, moon_center, r);
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    if (phase == 0) {
        // New moon
        graphics_fill_circle(ctx, moon_center, r - 1);
    } else if (phase < 7) {
        // Waxing crescent. Sliver on the right.
        int offset = phase; // 1 to 6
        graphics_fill_circle(ctx, GPoint(moon_center.x - offset, moon_center.y), r);
    } else if (phase == 7) {
        // First quarter. Right half lit.
        graphics_fill_rect(ctx, GRect(moon_center.x - r - 1, moon_center.y - r - 1, r + 1, 2*r + 3), 0, GCornerNone);
    } else if (phase < 14) {
        // Waxing gibbous. Mostly right lit, small dark on left.
        int width = 14 - phase; // 1 to 6
        graphics_fill_rect(ctx, GRect(moon_center.x - r - 1, moon_center.y - r - 1, width, 2*r + 3), 0, GCornerNone);
    } else if (phase == 14) {
        // Full moon
        // do nothing, fully white
    } else if (phase < 21) {
        // Waning gibbous. Mostly left lit, small dark on right.
        int width = phase - 14; // 1 to 6
        graphics_fill_rect(ctx, GRect(moon_center.x + r + 2 - width, moon_center.y - r - 1, width + 1, 2*r + 3), 0, GCornerNone);
    } else if (phase == 21) {
        // Last quarter. Left half lit.
        graphics_fill_rect(ctx, GRect(moon_center.x, moon_center.y - r - 1, r + 2, 2*r + 3), 0, GCornerNone);
    } else if (phase < 28) {
        // Waning crescent. Sliver on the left.
        int offset = 28 - phase; // 1 to 6
        graphics_fill_circle(ctx, GPoint(moon_center.x + offset, moon_center.y), r);
    }
    
    // Draw Earthshine (makes dark side identifiable via outline)
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_circle(ctx, moon_center, r);
}

// Removed wind_direction string helper as we use an arrow now

void weather_summary_layer_create(Layer* parent_layer, GRect frame) {
    s_weather_summary_layer = layer_create(frame);
    GRect bounds = layer_get_bounds(s_weather_summary_layer);

#if defined(PBL_PLATFORM_EMERY)
    s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30)); // Used for Wind
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

    s_detail_layer = text_layer_create(GRect(0, 26, bounds.size.w, 30)); // Used for Humidity
#else
    s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, 24)); // Used for Wind
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

    s_detail_layer = text_layer_create(GRect(0, 20, bounds.size.w, 18)); // Used for Humidity
#endif
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, GColorWhite);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentLeft);

    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_temp_layer));
    layer_add_child(s_weather_summary_layer, text_layer_get_layer(s_detail_layer));

    layer_set_update_proc(s_weather_summary_layer, weather_summary_layer_update_proc);
    layer_add_child(parent_layer, s_weather_summary_layer);

    weather_summary_layer_refresh();
}

void weather_summary_layer_refresh() {
    if (!s_weather_summary_layer) return;

    static char s_wind_buffer[32];
    int speed = persist_get_wind_speed();
    int gust = persist_get_wind_gust();
    int pollen = persist_get_pollen_index();
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
#if defined(PBL_PLATFORM_EMERY)
    layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(4, 0, bounds.size.w - 24, 30));
    layer_set_frame(text_layer_get_layer(s_detail_layer), GRect(4, 26, bounds.size.w - 24, 30));
#else
    layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(4, 0, bounds.size.w - 24, 24));
    layer_set_frame(text_layer_get_layer(s_detail_layer), GRect(4, 20, bounds.size.w - 24, 18));
#endif
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

    static char s_hum_buffer[32];
    int press = persist_get_pressure();
    if (pollen >= 0) {
        snprintf(s_hum_buffer, sizeof(s_hum_buffer), "Hum: %d%%, Pol: %d/5", persist_get_humidity(), pollen);
    } else {
        snprintf(s_hum_buffer, sizeof(s_hum_buffer), "Hum: %d%%, %d.%02din", persist_get_humidity(), press / 100, press % 100);
    }
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

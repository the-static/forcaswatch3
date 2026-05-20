#include "pws_layer.h"
#ifndef PBL_PLATFORM_APLITE
#include "c/appendix/persist.h"
#include "c/appendix/config.h"
#include "c/appendix/math.h"
#include <stdlib.h>

static Layer *s_pws_layer;
static TextLayer *s_detail_layer;
static TextLayer *s_temp_layer;

static char *s_temp_buffer = NULL;
static char *s_detail_buffer = NULL;

// emery: adjust arrow dimensions for the larger display
#if defined(PBL_PLATFORM_EMERY)
#define WIND_ARROW_SCALE_X 10
#define WIND_ARROW_SCALE_Y 16
#define WIND_ARROW_MID_Y 7
#define ARROW_X_OFFSET 20
#else
#define WIND_ARROW_SCALE_X 8
#define WIND_ARROW_SCALE_Y 12
#define WIND_ARROW_MID_Y 5
#define ARROW_X_OFFSET 15
#endif

static const GPathInfo WIND_ARROW_POINTS = {
  .num_points = 4,
  .points = (GPoint []) {{0, -WIND_ARROW_SCALE_Y}, {WIND_ARROW_SCALE_X, WIND_ARROW_SCALE_Y}, {0, WIND_ARROW_MID_Y}, {-WIND_ARROW_SCALE_X, WIND_ARROW_SCALE_Y}}
};
static GPath *s_wind_arrow_path;

static void pws_layer_update_proc(Layer *layer, GContext *ctx) {
    char station_id[16];
    persist_get_pws_station_id(station_id, sizeof(station_id));
    if (strcmp(station_id, "ERROR") == 0) {
        return;
    }

    GRect bounds = layer_get_bounds(layer);
    
    // Draw wind arrow centered vertically on the right edge
    int wind_deg = persist_get_pws_wind_deg();
    s_wind_arrow_path = gpath_create(&WIND_ARROW_POINTS);
    gpath_move_to(s_wind_arrow_path, GPoint(bounds.size.w - ARROW_X_OFFSET, bounds.size.h / 2)); 
    gpath_rotate_to(s_wind_arrow_path, DEG_TO_TRIGANGLE(wind_deg));
    
    graphics_context_set_fill_color(ctx, GColorWhite);
    gpath_draw_filled(ctx, s_wind_arrow_path);
    gpath_destroy(s_wind_arrow_path);
}

void pws_layer_create(Layer* parent_layer, GRect frame) {
    s_pws_layer = layer_create(frame);
    GRect bounds = layer_get_bounds(s_pws_layer);

    s_temp_buffer = malloc(48);
    s_detail_buffer = malloc(48);

    // emery: define larger height and font layers
#if defined(PBL_PLATFORM_EMERY)
    s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30)); 
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

    s_detail_layer = text_layer_create(GRect(0, 26, bounds.size.w, 30)); 
#else
    s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, 24)); 
    text_layer_set_background_color(s_temp_layer, GColorClear);
    text_layer_set_text_color(s_temp_layer, GColorWhite);
    text_layer_set_font(s_temp_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

    s_detail_layer = text_layer_create(GRect(0, 20, bounds.size.w, 18)); 
#endif
    text_layer_set_background_color(s_detail_layer, GColorClear);
    text_layer_set_text_color(s_detail_layer, GColorWhite);
    text_layer_set_font(s_detail_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentLeft);

    layer_add_child(s_pws_layer, text_layer_get_layer(s_temp_layer));
    layer_add_child(s_pws_layer, text_layer_get_layer(s_detail_layer));

    layer_set_update_proc(s_pws_layer, pws_layer_update_proc);
    layer_add_child(parent_layer, s_pws_layer);

    pws_layer_refresh();
}

void pws_layer_refresh() {
    if (!s_pws_layer || !s_temp_buffer || !s_detail_buffer) return;

    char station_id[16];
    persist_get_pws_station_id(station_id, sizeof(station_id));
    if (strcmp(station_id, "ERROR") == 0) {
        snprintf(s_temp_buffer, 48, "Invalid PWS ID");
        snprintf(s_detail_buffer, 48, "Check settings");
        text_layer_set_text(s_temp_layer, s_temp_buffer);
        text_layer_set_text(s_detail_layer, s_detail_buffer);
        GRect bounds = layer_get_bounds(s_pws_layer);
        layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(0, 4, bounds.size.w, bounds.size.h / 2));
        layer_set_frame(text_layer_get_layer(s_detail_layer), GRect(0, (bounds.size.h / 2) + 2, bounds.size.w, bounds.size.h / 2));
        text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
        text_layer_set_text_alignment(s_detail_layer, GTextAlignmentCenter);
        layer_mark_dirty(s_pws_layer);
        return;
    }

    int temp = config_localize_temp(persist_get_pws_temp());
    int speed = persist_get_pws_wind_speed();
    int gust = persist_get_pws_wind_gust();
    const char* unit = g_config->celsius ? "m/s" : "mph";

    int deg = persist_get_pws_wind_deg();
    const char* dir = get_wind_direction_string(deg);

    if (gust > speed) {
        snprintf(s_temp_buffer, 48, "%d° %s: %d(%d) %s", 
                 temp, dir, speed, gust, unit);
    } else {
        snprintf(s_temp_buffer, 48, "%d° %s: %d %s", 
                 temp, dir, speed, unit);
    }
    text_layer_set_text(s_temp_layer, s_temp_buffer);
    
    // Reposition layers to make room for arrow
    GRect bounds = layer_get_bounds(s_pws_layer);
    // emery: position larger text layer frames
#if defined(PBL_PLATFORM_EMERY)
    layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(4, 0, bounds.size.w - 24, 30));
    layer_set_frame(text_layer_get_layer(s_detail_layer), GRect(4, 26, bounds.size.w - 24, 30));
#else
    layer_set_frame(text_layer_get_layer(s_temp_layer), GRect(4, 0, bounds.size.w - 24, 24));
    layer_set_frame(text_layer_get_layer(s_detail_layer), GRect(4, 20, bounds.size.w - 24, 18));
#endif
    text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);
    text_layer_set_text_alignment(s_detail_layer, GTextAlignmentLeft);

    int total_hundredths = persist_get_pws_precip_total();
    int rate_hundredths = persist_get_pws_precip_rate();
    if (g_config->celsius) {
        // Convert hundredths of an inch to hundredths of a mm (e.g. 45 -> 1143 hundredths of mm)
        int total_val = (total_hundredths * 254) / 10;
        int rate_val = (rate_hundredths * 254) / 10;
        snprintf(s_detail_buffer, 48, "Rain: %d.%02d %d.%02d/h",
                 total_val / 100, total_val % 100, rate_val / 100, rate_val % 100);
    } else {
        snprintf(s_detail_buffer, 48, "Rain: %d.%02din %d.%02d/h",
                 total_hundredths / 100, total_hundredths % 100, rate_hundredths / 100, rate_hundredths % 100);
    }
    text_layer_set_text(s_detail_layer, s_detail_buffer);

    layer_mark_dirty(s_pws_layer);
}

void pws_layer_destroy() {
    if (!s_pws_layer) return;
    text_layer_destroy(s_temp_layer);
    text_layer_destroy(s_detail_layer);
    if (s_temp_buffer) {
        free(s_temp_buffer);
        s_temp_buffer = NULL;
    }
    if (s_detail_buffer) {
        free(s_detail_buffer);
        s_detail_buffer = NULL;
    }
    layer_destroy(s_pws_layer);
    s_pws_layer = NULL;
}
#endif

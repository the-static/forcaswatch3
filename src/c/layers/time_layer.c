#include "time_layer.h"
#include "c/appendix/config.h"
#include "c/appendix/memory_log.h"

// MT = Margin Top
#if defined(PBL_PLATFORM_EMERY)
#define MT_TIME 19
#define MT_AM_PM 10
#else
#define MT_TIME 14
#define MT_AM_PM 7
#endif


static TextLayer *s_container_layer;
static TextLayer *s_time_layer;
static TextLayer *s_am_pm_layer;

static bool s_zulu_time = false;

void time_layer_toggle_zulu() {
    s_zulu_time = !s_zulu_time;
    time_layer_refresh();
}

void time_layer_create(Layer* parent_layer, GRect frame) {
    s_container_layer = text_layer_create(frame);
    s_time_layer = text_layer_create(GRect(0, 0, frame.size.w, frame.size.h));
    s_am_pm_layer = text_layer_create(GRect(0, 0, 30, frame.size.h));

    text_layer_set_background_color(s_container_layer, GColorClear);

    // Main time formatting
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text(s_time_layer, "00:00:00");
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);

    // AM/PM formatting
    text_layer_set_font(s_am_pm_layer, fonts_get_system_font(SYS_FONT_18));
    text_layer_set_background_color(s_am_pm_layer, GColorClear);
    text_layer_set_text_color(s_am_pm_layer, GColorWhite);
    text_layer_set_text(s_am_pm_layer, "PM");
    text_layer_set_text_alignment(s_am_pm_layer, GTextAlignmentRight);

    layer_add_child(text_layer_get_layer(s_container_layer), text_layer_get_layer(s_time_layer));
    layer_add_child(text_layer_get_layer(s_container_layer), text_layer_get_layer(s_am_pm_layer));
    layer_add_child(parent_layer, text_layer_get_layer(s_container_layer));
    MEMORY_LOG_HEAP("after_time_layer_create");

}

// 12:30 -> 12:30
// 13:30 -> 1:30
// 00:30 -> 12:30

static void text_layer_move_frame(TextLayer *text_layer, GRect frame) {
    layer_set_frame(text_layer_get_layer(text_layer), frame);
}

void time_layer_tick() {
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = s_zulu_time ? gmtime(&temp) : localtime(&temp);

    // Format the time into a buffer
    static char s_buffer[12];
    if (s_zulu_time) {
        strftime(s_buffer, sizeof(s_buffer), "%H:%M:%S", tick_time);
    } else {
        config_format_time(s_buffer, 12, tick_time);
    }

    // Update the time and AM/PM indicator
    text_layer_set_text(s_time_layer, s_buffer);
    if (s_zulu_time) {
        text_layer_set_text(s_am_pm_layer, "Z");
    } else if (g_config->show_am_pm) {
        text_layer_set_text(s_am_pm_layer, tick_time->tm_hour < 12 ? "AM" : "PM");
    }
    
    // Reposition everything
    GRect bounds = layer_get_bounds(text_layer_get_layer(s_container_layer));
    text_layer_move_frame(s_time_layer, GRect(0, 0, bounds.size.w, bounds.size.h)); // Reset for size calculation
    GSize time_size = text_layer_get_content_size(s_time_layer);

    // Calculate some landmarks
    bool show_suffix = s_zulu_time || g_config->show_am_pm;
    int text_h = time_size.h - MT_TIME; // Remove top margin, approximately
    int text_top = -MT_TIME + (bounds.size.h/2 - text_h/2);
    int padding = 5;

    // Update layer positions and visibility
    text_layer_move_frame(s_time_layer, GRect(padding, text_top, bounds.size.w - 35 - padding, time_size.h));
    if (show_suffix) {
        text_layer_move_frame(s_am_pm_layer, GRect(bounds.size.w - 35 - padding, text_top + MT_TIME - MT_AM_PM, 35, time_size.h));
    }
    layer_set_hidden(text_layer_get_layer(s_am_pm_layer), !show_suffix);
}

void time_layer_refresh() {
    if (!s_time_layer) return;
    text_layer_set_font(s_time_layer, config_time_font());
    text_layer_set_text_color(s_time_layer, PBL_IF_COLOR_ELSE(g_config->color_time, GColorWhite));
    time_layer_tick();  // Update main time text and layer positions
}

void time_layer_destroy() {
    if (!s_container_layer) return;
    MEMORY_LOG_HEAP("time_layer_destroy:before");
    text_layer_destroy(s_time_layer);
    s_time_layer = NULL;
    text_layer_destroy(s_container_layer);
    s_container_layer = NULL;
    MEMORY_LOG_HEAP("time_layer_destroy:after");
}

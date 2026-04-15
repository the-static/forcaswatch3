#include "main_window.h"
#include "c/layers/time_layer.h"
#include "c/layers/forecast_layer.h"
#include "c/layers/weather_status_layer.h"
#include "c/layers/calendar_layer.h"
#include "c/layers/calendar_status_layer.h"
#include "c/layers/weather_summary_layer.h"
#include "c/layers/precip_chart_layer.h"
#include "c/layers/loading_layer.h"
#include "c/appendix/app_message.h"
#include "c/appendix/persist.h"
#include "c/appendix/memory_log.h"

#if defined(PBL_PLATFORM_EMERY)
#define FORECAST_HEIGHT 69
#define WEATHER_STATUS_HEIGHT 19
#define TIME_HEIGHT 61
#define CALENDAR_HEIGHT 61
#define CALENDAR_STATUS_HEIGHT 18
#else
#define FORECAST_HEIGHT 51
#define WEATHER_STATUS_HEIGHT 14
#define TIME_HEIGHT 45
#define CALENDAR_HEIGHT 45
#define CALENDAR_STATUS_HEIGHT 13
#endif

static Window *s_main_window;
static int16_t s_current_top_content;
static Layer *s_window_layer;

static void main_window_load(Window *window) {
    // Get information about the Window
    s_window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(s_window_layer);
    int w = bounds.size.w;
    int h = bounds.size.h;
    window_set_background_color(window, GColorBlack);
    
    weather_status_layer_create(s_window_layer,
            GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT, w, WEATHER_STATUS_HEIGHT));
    time_layer_create(s_window_layer,
            GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT - TIME_HEIGHT,
            bounds.size.w, TIME_HEIGHT));

    s_current_top_content = g_config->top_content;
    calendar_status_layer_create(s_window_layer,
            GRect(0, 0, bounds.size.w, CALENDAR_STATUS_HEIGHT + 1));

    if (s_current_top_content == TOP_CONTENT_CALENDAR) {
        calendar_layer_create(s_window_layer,
                GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
        forecast_layer_create(s_window_layer,
                GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    } else {
        weather_summary_layer_create(s_window_layer,
                GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
        precip_chart_layer_create(s_window_layer,
                GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    }

    loading_layer_create(s_window_layer,
            GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT, w, FORECAST_HEIGHT + WEATHER_STATUS_HEIGHT));
    loading_layer_refresh();
    app_message_send_startup_state(loading_layer_has_valid_data());
    MEMORY_LOG_HEAP("after_window_load");
}

static void main_window_unload(Window *window) {
    MEMORY_LOG_HEAP("before_window_unload");
    time_layer_destroy();
    weather_status_layer_destroy();
    forecast_layer_destroy();
    calendar_layer_destroy();
    calendar_status_layer_destroy();
    weather_summary_layer_destroy();
    precip_chart_layer_destroy();
    loading_layer_destroy();
    MEMORY_LOG_HEAP("after_window_unload");
}

static void minute_handler(struct tm *tick_time, TimeUnits units_changed) {
    time_layer_tick();
    /* tm_hour==0 missed day changes from emulator time jumps (same clock, new date). */
    if (units_changed & DAY_UNIT) {
        calendar_layer_refresh();
        calendar_status_layer_refresh();
        weather_summary_layer_refresh();
        precip_chart_layer_refresh();
    }
    status_icons_refresh();
    loading_layer_refresh();
}

static bool s_tap_locked = false;

static void tap_unlock_callback(void *data) {
    s_tap_locked = false;
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
    if (s_tap_locked) return;
    s_tap_locked = true;
    app_timer_register(1000, tap_unlock_callback, NULL);

    g_config->top_content = (g_config->top_content == TOP_CONTENT_CALENDAR) ? TOP_CONTENT_WEATHER : TOP_CONTENT_CALENDAR;
    main_window_refresh();
}

/*----------------------------
-------- EXTERNAL ------------
----------------------------*/

void main_window_create() {
    // Create main Window element and assign to pointer
    s_main_window = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT | DAY_UNIT, minute_handler);
    accel_tap_service_subscribe(tap_handler);

    // Show the window on the watch with animated=true
    window_stack_push(s_main_window, true);
    time_layer_refresh();
}

void main_window_refresh() {
    if (s_current_top_content != g_config->top_content) {
        // Swap ONLY the content layers
        calendar_layer_destroy();
        weather_summary_layer_destroy();
        forecast_layer_destroy();
        precip_chart_layer_destroy();
        
        GRect bounds = layer_get_bounds(s_window_layer);
        int w = bounds.size.w;
        int h = bounds.size.h;
        s_current_top_content = g_config->top_content;

        if (s_current_top_content == TOP_CONTENT_CALENDAR) {
            calendar_layer_create(s_window_layer,
                    GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
            forecast_layer_create(s_window_layer,
                    GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
        } else {
            weather_summary_layer_create(s_window_layer,
                    GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
            precip_chart_layer_create(s_window_layer,
                    GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
        }
    }

    time_layer_refresh();
    weather_status_layer_refresh();
    calendar_status_layer_refresh();
    
    // Refresh active content layers
    calendar_layer_refresh();
    weather_summary_layer_refresh();
    forecast_layer_refresh();
    precip_chart_layer_refresh();
}

void main_window_destroy() {
    accel_tap_service_unsubscribe();
    // Interface for destroying the main window (implicitly unloads contents)
    window_destroy(s_main_window);
}

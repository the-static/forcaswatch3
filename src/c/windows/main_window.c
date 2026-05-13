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
static Layer *s_window_layer;

#define BOTTOM_CONTENT_FORECAST 0
#define BOTTOM_CONTENT_PRECIP 1

static int16_t s_target_top_content;
static int16_t s_target_bottom_content;
static int16_t s_drawn_top_content = -1;
static int16_t s_drawn_bottom_content = -1;
static int16_t s_last_config_top_content;


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

    s_last_config_top_content = g_config->top_content;
    s_target_top_content = g_config->top_content;
    s_target_bottom_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? BOTTOM_CONTENT_FORECAST : BOTTOM_CONTENT_PRECIP;

    calendar_status_layer_create(s_window_layer,
            GRect(0, 0, bounds.size.w, CALENDAR_STATUS_HEIGHT + 1));

    if (s_target_top_content == TOP_CONTENT_CALENDAR) {
        calendar_layer_create(s_window_layer,
                GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
    } else {
        weather_summary_layer_create(s_window_layer,
                GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
    }

    if (s_target_bottom_content == BOTTOM_CONTENT_FORECAST) {
        forecast_layer_create(s_window_layer,
                GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    } else {
        precip_chart_layer_create(s_window_layer,
                GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    }

    s_drawn_top_content = s_target_top_content;
    s_drawn_bottom_content = s_target_bottom_content;

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

    s_target_top_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? TOP_CONTENT_WEATHER : TOP_CONTENT_CALENDAR;
    s_target_bottom_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? BOTTOM_CONTENT_FORECAST : BOTTOM_CONTENT_PRECIP;
    main_window_refresh();
}

static void touch_handler(const TouchEvent *event, void *context) {
    if (s_tap_locked) return;
    
    if (event->type == TouchEvent_Touchdown) {
        s_tap_locked = true;
        app_timer_register(1000, tap_unlock_callback, NULL);

        GRect bounds = layer_get_bounds(s_window_layer);
        if (event->y < bounds.size.h / 2) {
            s_target_top_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? TOP_CONTENT_WEATHER : TOP_CONTENT_CALENDAR;
        } else {
            s_target_bottom_content = (s_target_bottom_content == BOTTOM_CONTENT_FORECAST) ? BOTTOM_CONTENT_PRECIP : BOTTOM_CONTENT_FORECAST;
        }
        main_window_refresh();
    }
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
#if defined(PBL_PLATFORM_EMERY)
    if (touch_service_is_enabled()) {
        touch_service_subscribe(touch_handler, NULL);
    } else {
        accel_tap_service_subscribe(tap_handler);
    }
#else
    accel_tap_service_subscribe(tap_handler);
#endif


    // Show the window on the watch with animated=true
    window_stack_push(s_main_window, true);
    time_layer_refresh();
}

void main_window_refresh() {
    if (s_last_config_top_content != g_config->top_content) {
        s_last_config_top_content = g_config->top_content;
        s_target_top_content = g_config->top_content;
        s_target_bottom_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? BOTTOM_CONTENT_FORECAST : BOTTOM_CONTENT_PRECIP;
    }

    GRect bounds = layer_get_bounds(s_window_layer);
    int w = bounds.size.w;
    int h = bounds.size.h;

    if (s_drawn_top_content != s_target_top_content) {
        if (s_drawn_top_content == TOP_CONTENT_CALENDAR) {
            calendar_layer_destroy();
        } else if (s_drawn_top_content == TOP_CONTENT_WEATHER) {
            weather_summary_layer_destroy();
        }
        
        s_drawn_top_content = s_target_top_content;
        
        if (s_drawn_top_content == TOP_CONTENT_CALENDAR) {
            calendar_layer_create(s_window_layer,
                    GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
        } else {
            weather_summary_layer_create(s_window_layer,
                    GRect(0, CALENDAR_STATUS_HEIGHT, bounds.size.w, CALENDAR_HEIGHT));
        }
    }

    if (s_drawn_bottom_content != s_target_bottom_content) {
        if (s_drawn_bottom_content == BOTTOM_CONTENT_FORECAST) {
            forecast_layer_destroy();
        } else if (s_drawn_bottom_content == BOTTOM_CONTENT_PRECIP) {
            precip_chart_layer_destroy();
        }
        
        s_drawn_bottom_content = s_target_bottom_content;
        
        if (s_drawn_bottom_content == BOTTOM_CONTENT_FORECAST) {
            forecast_layer_create(s_window_layer,
                    GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
        } else {
            precip_chart_layer_create(s_window_layer,
                    GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
        }
    }

    time_layer_refresh();
    weather_status_layer_refresh();
    calendar_status_layer_refresh();
    
    // Refresh active content layers
    if (s_drawn_top_content == TOP_CONTENT_CALENDAR) {
        calendar_layer_refresh();
    } else {
        weather_summary_layer_refresh();
    }
    
    if (s_drawn_bottom_content == BOTTOM_CONTENT_FORECAST) {
        forecast_layer_refresh();
    } else {
        precip_chart_layer_refresh();
    }
}

void main_window_destroy() {
#if defined(PBL_PLATFORM_EMERY)
    if (touch_service_is_enabled()) {
        touch_service_unsubscribe();
    } else {
        accel_tap_service_unsubscribe();
    }
#else
    accel_tap_service_unsubscribe();
#endif
    // Interface for destroying the main window (implicitly unloads contents)
    window_destroy(s_main_window);
}

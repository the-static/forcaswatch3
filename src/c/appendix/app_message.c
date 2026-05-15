#include "app_message.h"
#include "persist.h"
#include "math.h"
#include "c/layers/forecast_layer.h"
#include "c/layers/weather_status_layer.h"
#include "c/layers/loading_layer.h"
#include "c/layers/calendar_layer.h"
#include "c/layers/calendar_status_layer.h"
#include "c/layers/weather_summary_layer.h"
#include "c/windows/main_window.h"
#include "memory_log.h"
#include <stdlib.h>
#include <string.h>

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Message received!");
    // Weather data
    Tuple *temp_trend_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_TREND_INT16);
    Tuple *precip_trend_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP_TREND_UINT8);
    Tuple *forecast_start_tuple = dict_find(iterator, MESSAGE_KEY_FORECAST_START);
    Tuple *num_entries_tuple = dict_find(iterator, MESSAGE_KEY_NUM_ENTRIES);
    Tuple *current_temp_tuple = dict_find(iterator, MESSAGE_KEY_CURRENT_TEMP);
    Tuple *city_tuple = dict_find(iterator, MESSAGE_KEY_CITY);
    Tuple *sun_events_tuple = dict_find(iterator, MESSAGE_KEY_SUN_EVENTS);
    Tuple *wind_speed_tuple = dict_find(iterator, MESSAGE_KEY_WIND_SPEED);
    Tuple *wind_deg_tuple = dict_find(iterator, MESSAGE_KEY_WIND_DEG);
    Tuple *humidity_tuple = dict_find(iterator, MESSAGE_KEY_HUMIDITY);
    Tuple *wind_gust_tuple = dict_find(iterator, MESSAGE_KEY_WIND_GUST);
    Tuple *precip_str_tuple = dict_find(iterator, MESSAGE_KEY_PRECIP_7DAY_STR);
    Tuple *temp_hi_str_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_7DAY_HI_STR);
    Tuple *temp_lo_str_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_7DAY_LO_STR);
    Tuple *pressure_tuple = dict_find(iterator, MESSAGE_KEY_PRESSURE);
    Tuple *pollen_tuple = dict_find(iterator, MESSAGE_KEY_POLLEN_INDEX);
    Tuple *app_fetch_time_tuple = dict_find(iterator, MESSAGE_KEY_APP_FETCH_TIME);
    Tuple *clay_active_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_ACTIVE);

    // Clay config options
    Tuple *clay_celsius_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_CELSIUS);
    Tuple *clay_time_lead_zero_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_TIME_LEAD_ZERO);
    Tuple *clay_axis_12h_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_AXIS_12H);
    Tuple *clay_start_mon_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_START_MON);
    Tuple *clay_prev_week_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_PREV_WEEK);
    Tuple *clay_color_today_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_COLOR_TODAY);
    Tuple *clay_time_font_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_TIME_FONT);
    Tuple *clay_vibe_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_VIBE);
    Tuple *clay_show_qt_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_SHOW_QT);
    Tuple *clay_show_bt_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_SHOW_BT);
    Tuple *clay_show_bt_disconnect_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_SHOW_BT_DISCONNECT);
    Tuple *clay_show_am_pm_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_SHOW_AM_PM);
    Tuple *clay_color_saturday_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_COLOR_SATURDAY);
    Tuple *clay_color_sunday_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_COLOR_SUNDAY);
    Tuple *clay_color_us_federal_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_COLOR_US_FEDERAL);
    Tuple *clay_color_time_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_COLOR_TIME);
    Tuple *clay_day_night_shading_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_DAY_NIGHT_SHADING);
    Tuple *clay_top_content_tuple = dict_find(iterator, MESSAGE_KEY_CLAY_TOP_CONTENT);

    bool handled = false;

    if(temp_trend_tuple && precip_trend_tuple && forecast_start_tuple && num_entries_tuple && city_tuple && sun_events_tuple) {
        handled = true;
        // Weather data received
        APP_LOG(APP_LOG_LEVEL_INFO, "Weather data received!");
        persist_set_forecast_start((time_t)forecast_start_tuple->value->int32);
        const int num_entries = ((int)num_entries_tuple->value->int32);
        persist_set_num_entries(num_entries);
#ifdef FCW2_ENABLE_MEMORY_LOGGING
        APP_LOG(APP_LOG_LEVEL_DEBUG, "MEM|forecast_payload|entries=%d|free=%lu|used=%lu",
                num_entries,
                (unsigned long)heap_bytes_free(),
                (unsigned long)heap_bytes_used());
#endif
        int16_t *temp_data = (int16_t*) temp_trend_tuple->value->data;
        persist_set_temp_trend(temp_data, num_entries);
        uint8_t *precip_data = (uint8_t*) precip_trend_tuple->value->data;
        persist_set_precip_trend(precip_data, num_entries);
        persist_set_city((char*)city_tuple->value->cstring);
        int lo, hi;
        min_max(temp_data, num_entries, &lo, &hi);
        persist_set_temp_lo(lo);
        persist_set_temp_hi(hi);
        persist_set_current_temp((int)current_temp_tuple->value->int32);
        uint8_t sun_event_start_type = (uint8_t) sun_events_tuple->value->uint8;
        time_t *sun_event_times = (time_t*) (sun_events_tuple->value->data + 1);
        persist_set_sun_event_start_type(sun_event_start_type);
        persist_set_sun_event_times(sun_event_times, 2);
        if (wind_speed_tuple) persist_set_wind_speed((int)wind_speed_tuple->value->int32);
        if (wind_deg_tuple) persist_set_wind_deg((int)wind_deg_tuple->value->int32);
        if (humidity_tuple) persist_set_humidity((int)humidity_tuple->value->int32);
        if (wind_gust_tuple) persist_set_wind_gust((int)wind_gust_tuple->value->int32);
        if (pressure_tuple) persist_set_pressure((int)pressure_tuple->value->int32);
        if (pollen_tuple) persist_set_pollen_index((int)pollen_tuple->value->int32);
        persist_set_app_fetch_time((time_t)app_fetch_time_tuple->value->int32);
        persist_set_last_sync_time(time(NULL));
        
        if (precip_str_tuple) {
            uint8_t precip_values[7];
            char *str = (char*)precip_str_tuple->value->cstring;
            char *curr = str;
            for (int i = 0; i < 7; i++) {
                precip_values[i] = (uint8_t)atoi(curr);
                curr = strchr(curr, ',');
                if (curr) curr++;
                else break;
            }
            persist_set_precip_7day(precip_values);
        }
        if (temp_hi_str_tuple) {
            int16_t temp_hi_values[7];
            char *str = (char*)temp_hi_str_tuple->value->cstring;
            char *curr = str;
            for (int i = 0; i < 7; i++) {
                temp_hi_values[i] = (int16_t)atoi(curr);
                curr = strchr(curr, ',');
                if (curr) curr++;
                else break;
            }
            persist_set_temp_7day_hi(temp_hi_values);
        }
        if (temp_lo_str_tuple) {
            int16_t temp_lo_values[7];
            char *str = (char*)temp_lo_str_tuple->value->cstring;
            char *curr = str;
            for (int i = 0; i < 7; i++) {
                temp_lo_values[i] = (int16_t)atoi(curr);
                curr = strchr(curr, ',');
                if (curr) curr++;
                else break;
            }
            persist_set_temp_7day_lo(temp_lo_values);
        }
        loading_layer_refresh();
        forecast_layer_refresh();
        weather_status_layer_refresh();
        calendar_layer_refresh();
        calendar_status_layer_refresh();
        weather_summary_layer_refresh();
    }
    
    if (clay_celsius_tuple && clay_time_lead_zero_tuple && clay_axis_12h_tuple && clay_start_mon_tuple && clay_prev_week_tuple
        && clay_color_today_tuple && clay_time_font_tuple && clay_vibe_tuple && clay_show_qt_tuple && clay_show_bt_tuple
        && clay_show_bt_disconnect_tuple && clay_show_am_pm_tuple && clay_color_saturday_tuple && clay_color_sunday_tuple
        && clay_color_us_federal_tuple && clay_color_time_tuple && clay_day_night_shading_tuple && clay_top_content_tuple) {
        
        handled = true;
        // Clay config data received
        bool clay_celsius = (bool) (clay_celsius_tuple->value->int16);
        bool time_lead_zero = (bool) (clay_time_lead_zero_tuple->value->int16);
        bool axis_12h = (bool) (clay_axis_12h_tuple->value->int16);
        bool start_mon = (bool) (clay_start_mon_tuple->value->int16);
        bool prev_week = (bool) (clay_prev_week_tuple->value->int16);
        bool vibe = (bool) (clay_vibe_tuple->value->int16);
        bool show_qt = (bool) (clay_show_qt_tuple->value->int16);
        bool show_bt = (bool) (clay_show_bt_tuple->value->int16);
        bool show_bt_disconnect = (bool) (clay_show_bt_disconnect_tuple->value->int16);
        bool show_am_pm = (bool) (clay_show_am_pm_tuple->value->int16);
        bool day_night_shading = (bool) (clay_day_night_shading_tuple->value->int16);
        int16_t time_font = clay_time_font_tuple->value->int16;
        GColor color_today = GColorFromHEX(clay_color_today_tuple->value->int32);
        GColor color_saturday = GColorFromHEX(clay_color_saturday_tuple->value->int32);
        GColor color_sunday = GColorFromHEX(clay_color_sunday_tuple->value->int32);
        GColor color_us_federal = GColorFromHEX(clay_color_us_federal_tuple->value->int32);
        GColor color_time = GColorFromHEX(clay_color_time_tuple->value->int32);
        Config config = (Config) {
            .celsius = clay_celsius,
            .time_lead_zero = time_lead_zero,
            .axis_12h = axis_12h,
            .start_mon = start_mon,
            .prev_week = prev_week,
            .time_font = time_font,
            .color_today = color_today,
            .vibe = vibe,
            .show_qt = show_qt,
            .show_bt = show_bt,
            .show_bt_disconnect = show_bt_disconnect,
            .show_am_pm = show_am_pm,
            .color_saturday = color_saturday,
            .color_sunday = color_sunday,
            .color_us_federal = color_us_federal,
            .color_time = color_time,
            .day_night_shading = day_night_shading,
            .top_content = clay_top_content_tuple->value->int16
        };
        persist_set_config(config);
        main_window_refresh();
    }
    
    if (clay_active_tuple) {
        handled = true;
        main_window_set_clay_active(clay_active_tuple->value->uint8 == 1);
    }
    
    if (!handled) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Unhandled AppMessage received");
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void app_message_send_startup_state(bool has_forecast_data) {
    DictionaryIterator *outbox;
    AppMessageResult result = app_message_outbox_begin(&outbox);

    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to begin startup outbox: %d", result);
        return;
    }

    dict_write_uint8(outbox, MESSAGE_KEY_WATCH_HAS_FORECAST_DATA, has_forecast_data ? 1 : 0);
    time_t forecast_start = persist_get_forecast_start();
    if (forecast_start > 0) {
        dict_write_uint32(outbox, MESSAGE_KEY_FORECAST_START, (uint32_t)forecast_start);
    }
    
    time_t app_fetch_time = persist_get_app_fetch_time();
    if (app_fetch_time > 0) {
        dict_write_uint32(outbox, MESSAGE_KEY_APP_FETCH_TIME, (uint32_t)app_fetch_time);
    }

    // Send current config to sync phone-side settings
    if (g_config) {
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_CELSIUS, g_config->celsius ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_TIME_LEAD_ZERO, g_config->time_lead_zero ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_AXIS_12H, g_config->axis_12h ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_START_MON, g_config->start_mon ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_PREV_WEEK, g_config->prev_week ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_TIME_FONT, g_config->time_font);
        dict_write_int32(outbox, MESSAGE_KEY_CLAY_COLOR_TODAY, (int32_t)gcolor_to_hex(g_config->color_today));
        dict_write_int32(outbox, MESSAGE_KEY_CLAY_COLOR_SATURDAY, (int32_t)gcolor_to_hex(g_config->color_saturday));
        dict_write_int32(outbox, MESSAGE_KEY_CLAY_COLOR_SUNDAY, (int32_t)gcolor_to_hex(g_config->color_sunday));
        dict_write_int32(outbox, MESSAGE_KEY_CLAY_COLOR_US_FEDERAL, (int32_t)gcolor_to_hex(g_config->color_us_federal));
        dict_write_int32(outbox, MESSAGE_KEY_CLAY_COLOR_TIME, (int32_t)gcolor_to_hex(g_config->color_time));
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_VIBE, g_config->vibe ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_SHOW_QT, g_config->show_qt ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_SHOW_BT, g_config->show_bt ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_SHOW_BT_DISCONNECT, g_config->show_bt_disconnect ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_SHOW_AM_PM, g_config->show_am_pm ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_DAY_NIGHT_SHADING, g_config->day_night_shading ? 1 : 0);
        dict_write_int16(outbox, MESSAGE_KEY_CLAY_TOP_CONTENT, g_config->top_content);
    }

    result = app_message_outbox_send();

    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to send startup state: %d", result);
    }
}

void app_message_request_weather() {
    DictionaryIterator *outbox;
    AppMessageResult result = app_message_outbox_begin(&outbox);

    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to begin weather request outbox: %d", result);
        return;
    }

    dict_write_uint8(outbox, MESSAGE_KEY_REQUEST_WEATHER, 1);

    result = app_message_outbox_send();

    if (result != APP_MSG_OK) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Unable to send weather request: %d", result);
    }
}

void app_message_init() {
    // Register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);

    // Open AppMessage
    const int inbox_size = 1024;
    const int outbox_size = 1024;
    APP_LOG(APP_LOG_LEVEL_INFO, "AppMessage buffer sizes: inbox=%d outbox=%d", inbox_size, outbox_size);
    app_message_open(inbox_size, outbox_size);
    MEMORY_LOG_HEAP("after_app_message_open");
}

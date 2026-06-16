#include "persist.h"
#include "config.h"

enum key {
    TEMP_LO, TEMP_HI, TEMP_TREND, PRECIP_TREND, FORECAST_START, CITY, SUN_EVENT_START_TYPE, SUN_EVENT_TIMES, NUM_ENTRIES,
    CURRENT_TEMP, BATTERY_LEVEL, CONFIG,
    WIND_SPEED, WIND_DEG, HUMIDITY, WIND_GUST, PRECIP_7DAY, TEMP_7DAY_HI, TEMP_7DAY_LO, PRESSURE, POLLEN_INDEX,
    APP_FETCH_TIME, LAST_SYNC_TIME,
    PWS_TEMP, PWS_PRECIP_RATE, PWS_PRECIP_TOTAL, PWS_WIND_SPEED, PWS_WIND_DEG, PWS_WIND_GUST, PWS_STATION_ID,
#ifdef PBL_PLATFORM_EMERY
    WIND_TREND,
    HUMIDITY_TREND
#endif
}; // Deprecated: BATTERY_LEVEL

void persist_init() {
#ifdef PBL_PLATFORM_EMERY
    if (!persist_exists(WIND_TREND)) {
        uint8_t data[48] = {0};
        persist_write_data(WIND_TREND, (void*) data, sizeof(data));
    }
    if (!persist_exists(HUMIDITY_TREND)) {
        uint8_t data[48] = {0};
        persist_write_data(HUMIDITY_TREND, (void*) data, sizeof(data));
    }
#endif
    if (!persist_exists(TEMP_LO)) {
        persist_write_int(TEMP_LO, 2);
    }
    if (!persist_exists(TEMP_HI)) {
        persist_write_int(TEMP_HI, 12);
    }
    if (!persist_exists(TEMP_TREND)) {
        int16_t data[] = {2, 2, 2, 4, 7, 9, 11, 12, 12, 12, 11, 9};
        persist_write_data(TEMP_TREND, (void*) data, 12*sizeof(int16_t));
    }
    if (!persist_exists(PRECIP_TREND)) {
        uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        persist_write_data(PRECIP_TREND, (void*) data, sizeof(data));
    }
    if (!persist_exists(FORECAST_START)) {
        persist_write_int(FORECAST_START, 0);
    }
    if (!persist_exists(NUM_ENTRIES)) {
        persist_write_int(NUM_ENTRIES, 12);
    }
    if (!persist_exists(CURRENT_TEMP)) {
        persist_write_int(CURRENT_TEMP, 1);
    }
    if (!persist_exists(CITY)) {
        persist_write_string(CITY, "Koji");
    }
    if (!persist_exists(SUN_EVENT_START_TYPE)) {
        persist_write_int(SUN_EVENT_START_TYPE, 0);
    }
    if (!persist_exists(SUN_EVENT_TIMES)) {
        uint32_t data[] = {0, 0};
        persist_write_data(SUN_EVENT_TIMES, (void*) data, 2*sizeof(uint32_t));
    }
    if (!persist_exists(CONFIG)) {
        Config config = (Config) {
            .celsius = false,
            .time_lead_zero = false,
            .axis_12h = false,
            .start_mon = false,
            .prev_week = true,
            .time_font = TIME_FONT_ROBOTO,
            .color_today = GColorBlack,
            .show_qt = true,
            .show_bt = true,
            .show_bt_disconnect = true,
            .vibe = false,
            .show_am_pm = false,
            .color_saturday = GColorWhite,	
            .color_sunday = GColorWhite,
            .color_us_federal = GColorWhite,
            .color_time = GColorWhite,
            .day_night_shading = true,
            .top_content = TOP_CONTENT_WEATHER
        };
        persist_set_config(config);
    }
}

int persist_get_temp_lo() {
    return persist_read_int(TEMP_LO);
}

int persist_get_temp_hi() {
    return persist_read_int(TEMP_HI);
}

int persist_get_temp_trend(int16_t *buffer, const size_t buffer_size) {
    return persist_read_data(TEMP_TREND, (void*) buffer, buffer_size * sizeof(int16_t));
}

int persist_get_precip_trend(uint8_t *buffer, const size_t buffer_size) {
    return persist_read_data(PRECIP_TREND, (void*) buffer, buffer_size * sizeof(uint8_t));
}

#ifdef PBL_PLATFORM_EMERY
int persist_get_wind_trend(uint8_t *buffer, const size_t buffer_size) {
    return persist_read_data(WIND_TREND, (void*) buffer, buffer_size * sizeof(uint8_t));
}
int persist_get_humidity_trend(uint8_t *buffer, const size_t buffer_size) {
    return persist_read_data(HUMIDITY_TREND, (void*) buffer, buffer_size * sizeof(uint8_t));
}
#endif

time_t persist_get_forecast_start() {
    return (time_t) persist_read_int(FORECAST_START);
}

int persist_get_num_entries() {
    return persist_read_int(NUM_ENTRIES);
}

int persist_get_current_temp() {
    return persist_read_int(CURRENT_TEMP);
}

int persist_get_wind_speed() {
    return persist_exists(WIND_SPEED) ? persist_read_int(WIND_SPEED) : 0;
}

int persist_get_wind_deg() {
    return persist_exists(WIND_DEG) ? persist_read_int(WIND_DEG) : 0;
}

int persist_get_humidity() {
    return persist_exists(HUMIDITY) ? persist_read_int(HUMIDITY) : 0;
}
 
int persist_get_pressure() {
    return persist_exists(PRESSURE) ? persist_read_int(PRESSURE) : 2992;
}

int persist_get_wind_gust() {
    return persist_exists(WIND_GUST) ? persist_read_int(WIND_GUST) : 0;
}
 
void persist_get_precip_7day(uint8_t *buffer) {
    if (persist_exists(PRECIP_7DAY)) {
        persist_read_data(PRECIP_7DAY, buffer, 7);
    } else {
        memset(buffer, 0, 7);
    }
}

void persist_get_temp_7day_hi(int16_t *buffer) {
    if (persist_exists(TEMP_7DAY_HI)) {
        persist_read_data(TEMP_7DAY_HI, buffer, 7 * sizeof(int16_t));
    } else {
        memset(buffer, 0, 7 * sizeof(int16_t));
    }
}

void persist_get_temp_7day_lo(int16_t *buffer) {
    if (persist_exists(TEMP_7DAY_LO)) {
        persist_read_data(TEMP_7DAY_LO, buffer, 7 * sizeof(int16_t));
    } else {
        memset(buffer, 0, 7 * sizeof(int16_t));
    }
}

int persist_get_city(char *buffer, const size_t buffer_size) {
    return persist_read_string(CITY, buffer, buffer_size);
}

int persist_get_sun_event_start_type() {
    return persist_read_int(SUN_EVENT_START_TYPE);
}

int persist_get_sun_event_times(time_t *buffer, const size_t buffer_size) {
    return persist_read_data(SUN_EVENT_TIMES, (void*) buffer, buffer_size * sizeof(time_t));
}

int persist_get_config(Config *config) {
    return persist_read_data(CONFIG, config, sizeof(Config));
}

void persist_set_temp_lo(int val) {
    persist_write_int(TEMP_LO, val);
}

void persist_set_temp_hi(int val) {
    persist_write_int(TEMP_HI, val);
}

void persist_set_temp_trend(int16_t *data, const size_t size) {
    persist_write_data(TEMP_TREND, (void*) data, size * sizeof(int16_t));
}

void persist_set_precip_trend(uint8_t *data, const size_t size) {
    persist_write_data(PRECIP_TREND, (void*) data, size * sizeof(uint8_t));
}

#ifdef PBL_PLATFORM_EMERY
void persist_set_wind_trend(uint8_t *data, const size_t size) {
    persist_write_data(WIND_TREND, (void*) data, size * sizeof(uint8_t));
}
void persist_set_humidity_trend(uint8_t *data, const size_t size) {
    persist_write_data(HUMIDITY_TREND, (void*) data, size * sizeof(uint8_t));
}
#endif

void persist_set_forecast_start(time_t val) {
    persist_write_int(FORECAST_START, (int) val);
}

void persist_set_num_entries(int val) {
    persist_write_int(NUM_ENTRIES, val);
}

void persist_set_current_temp(int val) {
    persist_write_int(CURRENT_TEMP, val);
}

void persist_set_wind_speed(int val) {
    persist_write_int(WIND_SPEED, val);
}

void persist_set_wind_deg(int val) {
    persist_write_int(WIND_DEG, val);
}

void persist_set_humidity(int val) {
    persist_write_int(HUMIDITY, val);
}
 
void persist_set_pressure(int val) {
    persist_write_int(PRESSURE, val);
}

void persist_set_wind_gust(int val) {
    persist_write_int(WIND_GUST, val);
}
 
void persist_set_precip_7day(uint8_t *val) {
    persist_write_data(PRECIP_7DAY, val, 7);
}

void persist_set_temp_7day_hi(int16_t *val) {
    persist_write_data(TEMP_7DAY_HI, val, 7 * sizeof(int16_t));
}

void persist_set_temp_7day_lo(int16_t *val) {
    persist_write_data(TEMP_7DAY_LO, val, 7 * sizeof(int16_t));
}

void persist_set_city(char *val) {
    persist_write_string(CITY, val);
}

void persist_set_sun_event_start_type(int val) {
    persist_write_int(SUN_EVENT_START_TYPE, val);
}

void persist_set_sun_event_times(time_t *data, const size_t size) {
    persist_write_data(SUN_EVENT_TIMES, (void*) data, size * sizeof(time_t));
}

int persist_get_pollen_index() {
    return persist_exists(POLLEN_INDEX) ? persist_read_int(POLLEN_INDEX) : -1;
}

void persist_set_pollen_index(int val) {
    persist_write_int(POLLEN_INDEX, val);
}

time_t persist_get_app_fetch_time() {
    return persist_exists(APP_FETCH_TIME) ? (time_t)persist_read_int(APP_FETCH_TIME) : 0;
}

void persist_set_app_fetch_time(time_t val) {
    persist_write_int(APP_FETCH_TIME, (int)val);
}

time_t persist_get_last_sync_time() {
    return persist_exists(LAST_SYNC_TIME) ? (time_t)persist_read_int(LAST_SYNC_TIME) : 0;
}

void persist_set_last_sync_time(time_t val) {
    persist_write_int(LAST_SYNC_TIME, (int)val);
}

void persist_set_config(Config config) {
    persist_write_data(CONFIG, &config, sizeof(Config));
    config_refresh();  // Refresh global config variable
}

int persist_get_pws_temp() {
    return persist_exists(PWS_TEMP) ? persist_read_int(PWS_TEMP) : 0;
}

int persist_get_pws_precip_rate() {
    return persist_exists(PWS_PRECIP_RATE) ? persist_read_int(PWS_PRECIP_RATE) : 0;
}

int persist_get_pws_precip_total() {
    return persist_exists(PWS_PRECIP_TOTAL) ? persist_read_int(PWS_PRECIP_TOTAL) : 0;
}

int persist_get_pws_wind_speed() {
    return persist_exists(PWS_WIND_SPEED) ? persist_read_int(PWS_WIND_SPEED) : 0;
}

int persist_get_pws_wind_deg() {
    return persist_exists(PWS_WIND_DEG) ? persist_read_int(PWS_WIND_DEG) : 0;
}

int persist_get_pws_wind_gust() {
    return persist_exists(PWS_WIND_GUST) ? persist_read_int(PWS_WIND_GUST) : 0;
}

void persist_set_pws_temp(int val) {
    persist_write_int(PWS_TEMP, val);
}

void persist_set_pws_precip_rate(int val) {
    persist_write_int(PWS_PRECIP_RATE, val);
}

void persist_set_pws_precip_total(int val) {
    persist_write_int(PWS_PRECIP_TOTAL, val);
}

void persist_set_pws_wind_speed(int val) {
    persist_write_int(PWS_WIND_SPEED, val);
}

void persist_set_pws_wind_deg(int val) {
    persist_write_int(PWS_WIND_DEG, val);
}

void persist_set_pws_wind_gust(int val) {
    persist_write_int(PWS_WIND_GUST, val);
}

void persist_get_pws_station_id(char *buffer, const size_t buffer_size) {
    if (persist_exists(PWS_STATION_ID)) {
        persist_read_string(PWS_STATION_ID, buffer, buffer_size);
    } else {
        strncpy(buffer, "PWS", buffer_size);
    }
}

void persist_set_pws_station_id(const char *val) {
    persist_write_string(PWS_STATION_ID, val);
}

bool persist_has_pws_station_id(void) {
#ifndef PBL_PLATFORM_APLITE
    if (persist_exists(PWS_STATION_ID)) {
        char buf[16];
        persist_read_string(PWS_STATION_ID, buf, sizeof(buf));
        return (buf[0] != '\0');
    }
#endif
    return false;
}

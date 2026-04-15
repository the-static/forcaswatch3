#pragma once

#include <pebble.h>

#if defined(PBL_PLATFORM_EMERY)
#define SYS_FONT_14 FONT_KEY_GOTHIC_18
#define SYS_FONT_14_BOLD FONT_KEY_GOTHIC_18_BOLD
#define SYS_FONT_18 FONT_KEY_GOTHIC_24
#define SYS_FONT_18_BOLD FONT_KEY_GOTHIC_24_BOLD
#else
#define SYS_FONT_14 FONT_KEY_GOTHIC_14
#define SYS_FONT_14_BOLD FONT_KEY_GOTHIC_14_BOLD
#define SYS_FONT_18 FONT_KEY_GOTHIC_18
#define SYS_FONT_18_BOLD FONT_KEY_GOTHIC_18_BOLD
#endif

enum TimeFont {
    TIME_FONT_ROBOTO = 0,
    TIME_FONT_LECO = 1,
    TIME_FONT_BITHAM = 2,
};

enum TopContent {
    TOP_CONTENT_CALENDAR = 0,
    TOP_CONTENT_WEATHER = 1,
};

typedef struct {
    bool celsius;
    bool time_lead_zero;
    bool axis_12h;
    bool start_mon;
    bool prev_week;
    bool show_qt;
    bool show_bt;
    bool show_bt_disconnect;
    bool vibe;
    bool show_am_pm;
    int16_t time_font;
    GColor color_today;
    GColor color_saturday;
    GColor color_sunday;
    GColor color_us_federal;
    GColor color_time;
    bool day_night_shading;
    int16_t top_content;
} Config;

extern Config *g_config;

void config_load();

void config_refresh();

void config_unload();

int config_localize_temp(int temp_f);

int config_format_time(char *s, size_t maxsize, const struct tm * tm_p);

int config_axis_hour(int hour);

int config_n_today();

GFont config_time_font();

bool config_highlight_holidays();

bool config_highlight_sundays();

bool config_highlight_saturdays();

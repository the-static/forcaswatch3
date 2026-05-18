import sys

with open("src/c/windows/main_window.c", "r") as f:
    content = f.read()

bad_block = """#else
#define FORECAST_HEIGHT 51
#define WEATHER_STATUS_HEIGHT 14
#define TIME_HEIGHT 45
#define CALENDAR_HEIGHT 45
#define EMERY_WINDOW_PAD_X 2
#define EMERY_WINDOW_PAD_TOP 2
#define EMERY_WINDOW_PAD_BOTTOM 4
#endif"""

good_block = """#define EMERY_WINDOW_PAD_X 2
#define EMERY_WINDOW_PAD_TOP 2
#define EMERY_WINDOW_PAD_BOTTOM 4
#else
#define FORECAST_HEIGHT 51
#define WEATHER_STATUS_HEIGHT 14
#define TIME_HEIGHT 45
#define CALENDAR_HEIGHT 45
#endif"""

content = content.replace(bad_block, good_block)

with open("src/c/windows/main_window.c", "w") as f:
    f.write(content)

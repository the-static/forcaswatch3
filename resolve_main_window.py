import sys

with open("src/c/windows/main_window.c", "r") as f:
    lines = f.readlines()

out = []
in_main_window_load = False
for line in lines:
    if line.startswith("static void main_window_load"):
        in_main_window_load = True
        out.append(line)
        out.append("""    s_window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(s_window_layer);
    int w = bounds.size.w;
    int h = bounds.size.h;
    window_set_background_color(window, GColorBlack);

    s_last_config_top_content = g_config->top_content;
    s_target_top_content = g_config->top_content;
    s_target_bottom_content = (s_target_top_content == TOP_CONTENT_CALENDAR) ? BOTTOM_CONTENT_FORECAST : BOTTOM_CONTENT_PRECIP;
    s_drawn_top_content = s_target_top_content;
    s_drawn_bottom_content = s_target_bottom_content;

#ifdef PBL_PLATFORM_EMERY
    int content_x = EMERY_WINDOW_PAD_X;
    int content_y = EMERY_WINDOW_PAD_TOP;
    int content_w = w - EMERY_WINDOW_PAD_X * 2;
    int forecast_w = w - content_x;
    int content_h = h - EMERY_WINDOW_PAD_TOP - EMERY_WINDOW_PAD_BOTTOM - CALENDAR_STATUS_HEIGHT - WEATHER_STATUS_HEIGHT;
    int calendar_h, time_h, forecast_h;
    compute_content_layout(content_h, &calendar_h, &time_h, &forecast_h);

    int calendar_y = content_y + CALENDAR_STATUS_HEIGHT;
    int time_y = calendar_y + calendar_h;
    int weather_status_y = time_y + time_h;
    int forecast_y = weather_status_y + WEATHER_STATUS_HEIGHT;

    calendar_status_layer_create(s_window_layer, GRect(content_x, content_y, content_w, CALENDAR_STATUS_HEIGHT + 1));
    if (s_target_top_content == TOP_CONTENT_CALENDAR) {
        calendar_layer_create(s_window_layer, GRect(content_x, calendar_y, content_w, calendar_h));
    } else {
        weather_summary_layer_create(s_window_layer, GRect(content_x, calendar_y, content_w, calendar_h));
    }
    time_layer_create(s_window_layer, GRect(content_x, time_y, content_w, time_h));
    weather_status_layer_create(s_window_layer, GRect(content_x, weather_status_y, content_w, WEATHER_STATUS_HEIGHT));
    if (s_target_bottom_content == BOTTOM_CONTENT_FORECAST) {
        forecast_layer_create(s_window_layer, GRect(content_x, forecast_y, forecast_w, forecast_h));
    } else {
        precip_chart_layer_create(s_window_layer, GRect(content_x, forecast_y, forecast_w, forecast_h));
    }
    loading_layer_create(s_window_layer, GRect(content_x, weather_status_y, content_w, h - EMERY_WINDOW_PAD_BOTTOM - weather_status_y));
#else
    calendar_status_layer_create(s_window_layer, GRect(0, 0, w, CALENDAR_STATUS_HEIGHT + 1));
    if (s_target_top_content == TOP_CONTENT_CALENDAR) {
        calendar_layer_create(s_window_layer, GRect(0, CALENDAR_STATUS_HEIGHT, w, CALENDAR_HEIGHT));
    } else {
        weather_summary_layer_create(s_window_layer, GRect(0, CALENDAR_STATUS_HEIGHT, w, CALENDAR_HEIGHT));
    }
    time_layer_create(s_window_layer, GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT - TIME_HEIGHT, w, TIME_HEIGHT));
    weather_status_layer_create(s_window_layer, GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT, w, WEATHER_STATUS_HEIGHT));
    if (s_target_bottom_content == BOTTOM_CONTENT_FORECAST) {
        forecast_layer_create(s_window_layer, GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    } else {
        precip_chart_layer_create(s_window_layer, GRect(0, h - FORECAST_HEIGHT, w, FORECAST_HEIGHT));
    }
    loading_layer_create(s_window_layer, GRect(0, h - FORECAST_HEIGHT - WEATHER_STATUS_HEIGHT, w, FORECAST_HEIGHT + WEATHER_STATUS_HEIGHT));
#endif

    loading_layer_refresh();
    app_message_send_startup_state(loading_layer_has_valid_data());
    reset_idle_timer();
    MEMORY_LOG_HEAP("after_window_load");
}
""")
        continue

    if in_main_window_load:
        if line.startswith("static void main_window_unload"):
            in_main_window_load = False
            out.append(line)
        continue

    out.append(line)

with open("src/c/windows/main_window.c", "w") as f:
    f.writelines(out)

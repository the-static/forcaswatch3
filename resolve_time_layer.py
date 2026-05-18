import sys

with open("src/c/layers/time_layer.c", "r") as f:
    content = f.read()

# Fix conflict 1
content = content.replace("""<<<<<<< HEAD
#endif
=======
#define MT_TIME_LECO 2
#define MT_AM_PM_LECO 2
>>>>>>> upstream/main""", """#endif
#define MT_TIME_LECO 2
#define MT_AM_PM_LECO 2""")

# Fix conflict 2
c2_head = """    time_t temp = time(NULL);
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
    text_layer_set_text_color(s_am_pm_layer, GColorWhite);
    text_layer_set_background_color(s_am_pm_layer, GColorClear);

    // Update status text
    static char status_buffer[64];
    time_t last_sync = persist_get_last_sync_time();
    time_t app_fetch = persist_get_app_fetch_time();
    
    char f_buffer[12] = "--:--:--";
    char a_buffer[12] = "--:--:--";
    
    if (last_sync > 0) {
        struct tm *f_time = localtime(&last_sync);
        strftime(f_buffer, sizeof(f_buffer), "%H:%M:%S", f_time);
    }
    
    if (app_fetch > 0) {
        struct tm *a_time = localtime(&app_fetch);
        strftime(a_buffer, sizeof(a_buffer), "%H:%M:%S", a_time);
    }

    snprintf(status_buffer, sizeof(status_buffer), "App: %s | Phone: %s", f_buffer, a_buffer);
    text_layer_set_text(s_status_layer, status_buffer);"""

c2_upstream = """    struct tm tick_time = watch_services_localtime();

    // Format the time into a buffer
    static char s_buffer[8];
    config_format_time(s_buffer, 8, &tick_time);

    // Update the time and AM/PM indicator
    text_layer_set_text(s_time_layer, s_buffer);
    if (g_config->show_am_pm)
        text_layer_set_text(s_am_pm_layer, tick_time.tm_hour < 12 ? "AM" : "PM");"""

c2_resolved = """    time_t temp = time(NULL);
    struct tm tick_time = watch_services_localtime();
    struct tm *tick_time_ptr = s_zulu_time ? gmtime(&temp) : &tick_time;

    // Format the time into a buffer
    static char s_buffer[12];
    if (s_zulu_time) {
        strftime(s_buffer, sizeof(s_buffer), "%H:%M:%S", tick_time_ptr);
    } else {
        config_format_time(s_buffer, 12, tick_time_ptr);
    }

    // Update the time and AM/PM indicator
    text_layer_set_text(s_time_layer, s_buffer);
    if (s_zulu_time) {
        text_layer_set_text(s_am_pm_layer, "Z");
    } else if (g_config->show_am_pm) {
        text_layer_set_text(s_am_pm_layer, tick_time_ptr->tm_hour < 12 ? "AM" : "PM");
    }
    text_layer_set_text_color(s_am_pm_layer, GColorWhite);
    text_layer_set_background_color(s_am_pm_layer, GColorClear);

    // Update status text
    static char status_buffer[64];
    time_t last_sync = persist_get_last_sync_time();
    time_t app_fetch = persist_get_app_fetch_time();
    
    char f_buffer[12] = "--:--:--";
    char a_buffer[12] = "--:--:--";
    
    if (last_sync > 0) {
        struct tm *f_time = localtime(&last_sync);
        strftime(f_buffer, sizeof(f_buffer), "%H:%M:%S", f_time);
    }
    
    if (app_fetch > 0) {
        struct tm *a_time = localtime(&app_fetch);
        strftime(a_buffer, sizeof(a_buffer), "%H:%M:%S", a_time);
    }

    snprintf(status_buffer, sizeof(status_buffer), "App: %s | Phone: %s", f_buffer, a_buffer);
    text_layer_set_text(s_status_layer, status_buffer);"""

content = content.replace("<<<<<<< HEAD\n" + c2_head + "\n=======\n" + c2_upstream + "\n>>>>>>> upstream/main", c2_resolved)

# Fix conflict 3
c3_head = """    if (show_suffix)
        text_layer_move_frame(s_am_pm_layer, GRect(time_size.w, MT_TIME - MT_AM_PM, 30, time_size.h));
    layer_set_hidden(text_layer_get_layer(s_am_pm_layer), !show_suffix);"""

c3_upstream = """    if (g_config->show_am_pm) {
        int am_pm_y = MT_TIME - MT_AM_PM;
        // emery: nudge LECO AM/PM down slightly to align with larger time numerals.
#ifdef PBL_PLATFORM_EMERY
        if (g_config->time_font == TIME_FONT_LECO) {
            am_pm_y += MT_AM_PM_LECO;
        }
#endif
        text_layer_move_frame(s_am_pm_layer, GRect(time_size.w, am_pm_y, 30, time_size.h));
    }
    layer_set_hidden(text_layer_get_layer(s_am_pm_layer), !g_config->show_am_pm);"""

c3_resolved = """    if (show_suffix) {
        int am_pm_y = MT_TIME - MT_AM_PM;
        // emery: nudge LECO AM/PM down slightly to align with larger time numerals.
#ifdef PBL_PLATFORM_EMERY
        if (g_config->time_font == TIME_FONT_LECO) {
            am_pm_y += MT_AM_PM_LECO;
        }
#endif
        text_layer_move_frame(s_am_pm_layer, GRect(time_size.w, am_pm_y, 30, time_size.h));
    }
    layer_set_hidden(text_layer_get_layer(s_am_pm_layer), !show_suffix);"""

content = content.replace("<<<<<<< HEAD\n" + c3_head + "\n=======\n" + c3_upstream + "\n>>>>>>> upstream/main", c3_resolved)

with open("src/c/layers/time_layer.c", "w") as f:
    f.write(content)

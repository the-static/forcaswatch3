#include "precip_chart_layer.h"
#include "c/appendix/persist.h"
#include "c/appendix/config.h"

static Layer *s_precip_chart_layer;

// We no longer need the solid background helper since they don't overlap
static void precip_chart_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    uint8_t precips[7];
    int16_t temps_hi[7];
    int16_t temps_lo[7];
    persist_get_precip_7day(precips);
    persist_get_temp_7day_hi(temps_hi);
    persist_get_temp_7day_lo(temps_lo);

    int bar_w = bounds.size.w / 7;
    int label_h = 16;
    int chart_h = bounds.size.h - label_h; 

    // Draw baseline
    graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite));
    graphics_draw_line(ctx, GPoint(0, chart_h), GPoint(bounds.size.w, chart_h));
    
    // Draw top line for 100% capacity
    graphics_draw_line(ctx, GPoint(0, 0), GPoint(bounds.size.w, 0));

    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorPictonBlue, GColorWhite));
    graphics_context_set_text_color(ctx, GColorWhite);

    for (int i = 0; i < 7; i++) {
        int bar_h = (precips[i] * chart_h) / 100;
        if (precips[i] > 0 && bar_h == 0) bar_h = 1;
        
        if (bar_h > 0) {
            // Draw a thinner bar to the left
            GRect bar = GRect(i * bar_w + 2, chart_h - bar_h, 4, bar_h);
            // Sharp corners (0) look much better on monochrome and for small bars
            graphics_fill_rect(ctx, bar, 0, GCornerNone);
        }

        // Temps
        int loc_hi = config_localize_temp((int)temps_hi[i]);
        int loc_lo = config_localize_temp((int)temps_lo[i]);
        // If data is populated (avoid drawing '0' endlessly if not fetched yet)
        if (temps_hi[i] != 0 || temps_lo[i] != 0 || loc_hi != 0 || loc_lo != 0) {
            char hi_str[8];
            char lo_str[8];
            snprintf(hi_str, sizeof(hi_str), "%d", loc_hi);
            snprintf(lo_str, sizeof(lo_str), "%d", loc_lo);
            
            int hi_y = 2;
            int lo_y = chart_h - 16;
            
            // Draw hi temp to the right of the bar
            graphics_draw_text(ctx, hi_str, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                               GRect(i * bar_w + 7, hi_y, bar_w - 7, 14),
                               GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
            
            // Draw lo temp to the right of the bar
            graphics_draw_text(ctx, lo_str, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                               GRect(i * bar_w + 7, lo_y, bar_w - 7, 14),
                               GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
        }

        // Reset text color for day label
        graphics_context_set_text_color(ctx, GColorWhite);

        // Day label
        time_t day_time = time(NULL) + (i * 24 * 60 * 60);
        struct tm *t = localtime(&day_time);
        char label[4];
        strftime(label, sizeof(label), "%a", t);
        label[1] = '\0';
        
        graphics_draw_text(ctx, label, fonts_get_system_font(FONT_KEY_GOTHIC_14),
                           GRect(i * bar_w, chart_h, bar_w, label_h),
                           GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }
}

void precip_chart_layer_create(Layer *parent_layer, GRect frame) {
    s_precip_chart_layer = layer_create(frame);
    layer_set_update_proc(s_precip_chart_layer, precip_chart_update_proc);
    layer_add_child(parent_layer, s_precip_chart_layer);
}

void precip_chart_layer_destroy() {
    if (s_precip_chart_layer) {
        layer_destroy(s_precip_chart_layer);
        s_precip_chart_layer = NULL;
    }
}

void precip_chart_layer_refresh() {
    if (s_precip_chart_layer) {
        layer_mark_dirty(s_precip_chart_layer);
    }
}

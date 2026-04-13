#include "precip_chart_layer.h"
#include "c/appendix/persist.h"
#include "c/appendix/config.h"

static Layer *s_precip_chart_layer;

static void precip_chart_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    uint8_t precips[7];
    persist_get_precip_7day(precips);

    int bar_w = bounds.size.w / 7;
    int label_h = 16;
    int chart_h = bounds.size.h - label_h; 

    // Draw baseline
    graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorDarkGray, GColorWhite));
    graphics_draw_line(ctx, GPoint(0, chart_h), GPoint(bounds.size.w, chart_h));

    graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorPictonBlue, GColorWhite));
    graphics_context_set_text_color(ctx, GColorWhite);

    for (int i = 0; i < 7; i++) {
        int bar_h = (precips[i] * chart_h) / 100;
        if (precips[i] > 0 && bar_h == 0) bar_h = 1;
        
        if (bar_h > 0) {
            GRect bar = GRect(i * bar_w + 3, chart_h - bar_h, bar_w - 6, bar_h);
            // Sharp corners (0) look much better on monochrome and for small bars
            graphics_fill_rect(ctx, bar, 0, GCornerNone);
        }

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

#include "math.h"
#include <pebble.h>
#include "config.h"
#include "persist.h"

void min_max(int16_t *array, int n, int *min, int *max) {
    // It is assumed that the array has at least one element
    *min = array[0];
    *max = array[0];
    for (int i = 1; i < n; ++i) {
        if (array[i] < *min) {
            *min = array[i];
        }
        else if (array[i] > *max) {
            *max = array[i];
        }
    }
}

int roundFloat(float num) 
{ 
    return num < 0 ? num - 0.5 : num + 0.5; 
} 
  

static float my_logf(float x) {
    if (x <= 0.0f) return -99.0f;
    float t = (x - 1.0f) / (x + 1.0f);
    float t2 = t * t;
    float ans = t;
    float term = t;
    
    // term 3
    term *= t2;
    ans += term / 3.0f;
    
    // term 5
    term *= t2;
    ans += term / 5.0f;
    
    // term 7
    term *= t2;
    ans += term / 7.0f;
    
    return 2.0f * ans;
}

int get_dew_point(int temp_f, int rh) {
    if (rh <= 0) rh = 1;
    if (rh > 100) rh = 100;
    
    float temp_c = (temp_f - 32) * 5.0f / 9.0f;
    float a = 17.27f;
    float b = 237.7f;
    float alpha = ((a * temp_c) / (b + temp_c)) + my_logf((float)rh / 100.0f);
    float dew_point_c = (b * alpha) / (a - alpha);
    float dew_point_f = (dew_point_c * 9.0f / 5.0f) + 32.0f;
    
    return roundFloat(dew_point_f);
}

int f_to_c(int temp_f) {
    // Convert a fahrenheit temperature to celcius
    return roundFloat((temp_f - 32) * 5.0 / 9);
}

uint32_t gcolor_to_hex(GColor color) {
#if defined(PBL_COLOR)
    return ((color.r * 85) << 16) | ((color.g * 85) << 8) | (color.b * 85);
#else
    return gcolor_equal(color, GColorWhite) ? 0xFFFFFF : 0x000000;
#endif
}

const char* get_wind_direction_string(int degrees) {
    static const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int index = ((degrees + 22) % 360) / 45;
    return directions[index];
}
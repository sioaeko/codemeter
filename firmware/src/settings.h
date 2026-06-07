#pragma once
#include <stdint.h>

enum display_metric_t {
    DISPLAY_METRIC_USED = 0,
    DISPLAY_METRIC_USAGE = 1,
};

enum display_theme_t {
    DISPLAY_THEME_LIGHT = 0,
    DISPLAY_THEME_DARK = 1,
};

enum accent_theme_t {
    ACCENT_THEME_CLAUDE = 0,
    ACCENT_THEME_CODEX = 1,
};

void settings_init(void);
display_metric_t settings_display_metric(void);
void settings_set_display_metric(display_metric_t metric);
display_metric_t settings_toggle_display_metric(void);

display_theme_t settings_display_theme(void);
void settings_set_display_theme(display_theme_t theme);
display_theme_t settings_toggle_display_theme(void);

accent_theme_t settings_accent_theme(void);
void settings_set_accent_theme(accent_theme_t accent);
accent_theme_t settings_toggle_accent_theme(void);

bool settings_night_enabled(void);
uint16_t settings_night_start_min(void);
uint16_t settings_night_end_min(void);
int16_t settings_timezone_offset_min(void);
uint16_t settings_night_wake_minutes(void);
void settings_set_night_mode(bool enabled, uint16_t start_min, uint16_t end_min,
                             int16_t timezone_offset_min, uint16_t wake_minutes);
void settings_set_night_enabled(bool enabled);
void settings_set_night_start_min(uint16_t start_min);
void settings_set_night_end_min(uint16_t end_min);

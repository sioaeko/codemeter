#include "settings.h"
#include <Arduino.h>
#include <Preferences.h>

static Preferences prefs;
static bool prefs_ready = false;
static display_metric_t current_metric = DISPLAY_METRIC_USED;
static display_theme_t current_theme = DISPLAY_THEME_DARK;
static accent_theme_t current_accent = ACCENT_THEME_GREEN;
static bool current_night_enabled = false;
static uint16_t current_night_start_min = 23 * 60;
static uint16_t current_night_end_min = 7 * 60;
static int16_t current_timezone_offset_min = 9 * 60;
static uint16_t current_night_wake_minutes = 5;

static uint16_t sanitize_time_min(uint16_t value, uint16_t fallback) {
    return (value < 1440) ? value : fallback;
}

static int16_t sanitize_timezone_min(int16_t value) {
    if (value < -12 * 60) return -12 * 60;
    if (value > 14 * 60) return 14 * 60;
    return value;
}

static uint16_t sanitize_wake_minutes(uint16_t value) {
    if (value < 1) return 1;
    if (value > 60) return 60;
    return value;
}

void settings_init(void) {
    if (!prefs_ready) {
        prefs_ready = prefs.begin("codexmeter", false);
    }

    uint8_t stored = prefs_ready ? prefs.getUChar("metric", DISPLAY_METRIC_USED)
                                 : DISPLAY_METRIC_USED;
    current_metric = (stored == DISPLAY_METRIC_USAGE)
                         ? DISPLAY_METRIC_USAGE
                         : DISPLAY_METRIC_USED;

    stored = prefs_ready ? prefs.getUChar("theme", DISPLAY_THEME_DARK)
                         : DISPLAY_THEME_DARK;
    current_theme = (stored == DISPLAY_THEME_DARK)
                        ? DISPLAY_THEME_DARK
                        : DISPLAY_THEME_LIGHT;

    stored = prefs_ready ? prefs.getUChar("accent", ACCENT_THEME_GREEN)
                         : ACCENT_THEME_GREEN;
    current_accent = (stored == ACCENT_THEME_GREEN)
                         ? ACCENT_THEME_GREEN
                         : ACCENT_THEME_WARM;

    current_night_enabled = prefs_ready ? prefs.getBool("night_en", false) : false;
    current_night_start_min = sanitize_time_min(
        prefs_ready ? prefs.getUShort("night_st", 23 * 60) : 23 * 60,
        23 * 60);
    current_night_end_min = sanitize_time_min(
        prefs_ready ? prefs.getUShort("night_end", 7 * 60) : 7 * 60,
        7 * 60);
    current_timezone_offset_min = sanitize_timezone_min(
        prefs_ready ? prefs.getShort("tz_min", 9 * 60) : 9 * 60);
    current_night_wake_minutes = sanitize_wake_minutes(
        prefs_ready ? prefs.getUShort("wake_min", 5) : 5);
}

display_metric_t settings_display_metric(void) {
    return current_metric;
}

void settings_set_display_metric(display_metric_t metric) {
    current_metric = metric;
    if (prefs_ready) {
        prefs.putUChar("metric", (uint8_t)current_metric);
    }
}

display_metric_t settings_toggle_display_metric(void) {
    display_metric_t next = (current_metric == DISPLAY_METRIC_USED)
                                ? DISPLAY_METRIC_USAGE
                                : DISPLAY_METRIC_USED;
    settings_set_display_metric(next);
    return next;
}

display_theme_t settings_display_theme(void) {
    return current_theme;
}

void settings_set_display_theme(display_theme_t theme) {
    current_theme = theme;
    if (prefs_ready) {
        prefs.putUChar("theme", (uint8_t)current_theme);
    }
}

display_theme_t settings_toggle_display_theme(void) {
    display_theme_t next = (current_theme == DISPLAY_THEME_LIGHT)
                               ? DISPLAY_THEME_DARK
                               : DISPLAY_THEME_LIGHT;
    settings_set_display_theme(next);
    return next;
}

accent_theme_t settings_accent_theme(void) {
    return current_accent;
}

void settings_set_accent_theme(accent_theme_t accent) {
    current_accent = accent;
    if (prefs_ready) {
        prefs.putUChar("accent", (uint8_t)current_accent);
    }
}

accent_theme_t settings_toggle_accent_theme(void) {
    accent_theme_t next = (current_accent == ACCENT_THEME_WARM)
                              ? ACCENT_THEME_GREEN
                              : ACCENT_THEME_WARM;
    settings_set_accent_theme(next);
    return next;
}

bool settings_night_enabled(void) {
    return current_night_enabled;
}

uint16_t settings_night_start_min(void) {
    return current_night_start_min;
}

uint16_t settings_night_end_min(void) {
    return current_night_end_min;
}

int16_t settings_timezone_offset_min(void) {
    return current_timezone_offset_min;
}

uint16_t settings_night_wake_minutes(void) {
    return current_night_wake_minutes;
}

void settings_set_night_mode(bool enabled, uint16_t start_min, uint16_t end_min,
                             int16_t timezone_offset_min, uint16_t wake_minutes) {
    current_night_enabled = enabled;
    current_night_start_min = sanitize_time_min(start_min, 23 * 60);
    current_night_end_min = sanitize_time_min(end_min, 7 * 60);
    current_timezone_offset_min = sanitize_timezone_min(timezone_offset_min);
    current_night_wake_minutes = sanitize_wake_minutes(wake_minutes);

    if (prefs_ready) {
        prefs.putBool("night_en", current_night_enabled);
        prefs.putUShort("night_st", current_night_start_min);
        prefs.putUShort("night_end", current_night_end_min);
        prefs.putShort("tz_min", current_timezone_offset_min);
        prefs.putUShort("wake_min", current_night_wake_minutes);
    }
}

void settings_set_night_enabled(bool enabled) {
    settings_set_night_mode(enabled, current_night_start_min, current_night_end_min,
                            current_timezone_offset_min, current_night_wake_minutes);
}

void settings_set_night_start_min(uint16_t start_min) {
    settings_set_night_mode(current_night_enabled, start_min, current_night_end_min,
                            current_timezone_offset_min, current_night_wake_minutes);
}

void settings_set_night_end_min(uint16_t end_min) {
    settings_set_night_mode(current_night_enabled, current_night_start_min, end_min,
                            current_timezone_offset_min, current_night_wake_minutes);
}

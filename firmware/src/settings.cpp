#include "settings.h"
#include <Preferences.h>

static Preferences prefs;
static bool prefs_ready = false;
static display_metric_t current_metric = DISPLAY_METRIC_USED;
static display_theme_t current_theme = DISPLAY_THEME_DARK;
static accent_theme_t current_accent = ACCENT_THEME_CLAUDE;

void settings_init(void) {
    prefs_ready = prefs.begin("codexmeter", false);
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

    stored = prefs_ready ? prefs.getUChar("accent", ACCENT_THEME_CLAUDE)
                         : ACCENT_THEME_CLAUDE;
    current_accent = (stored == ACCENT_THEME_CODEX)
                         ? ACCENT_THEME_CODEX
                         : ACCENT_THEME_CLAUDE;
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
    accent_theme_t next = (current_accent == ACCENT_THEME_CLAUDE)
                              ? ACCENT_THEME_CODEX
                              : ACCENT_THEME_CLAUDE;
    settings_set_accent_theme(next);
    return next;
}

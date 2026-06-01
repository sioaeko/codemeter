#pragma once

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

#pragma once

enum display_metric_t {
    DISPLAY_METRIC_USED = 0,
    DISPLAY_METRIC_REMAINING = 1,
};

void settings_init(void);
display_metric_t settings_display_metric(void);
void settings_set_display_metric(display_metric_t metric);
display_metric_t settings_toggle_display_metric(void);

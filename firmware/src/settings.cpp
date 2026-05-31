#include "settings.h"
#include <Preferences.h>

static Preferences prefs;
static bool prefs_ready = false;
static display_metric_t current_metric = DISPLAY_METRIC_USED;

void settings_init(void) {
    prefs_ready = prefs.begin("codexmeter", false);
    uint8_t stored = prefs_ready ? prefs.getUChar("metric", DISPLAY_METRIC_USED)
                                 : DISPLAY_METRIC_USED;
    current_metric = (stored == DISPLAY_METRIC_REMAINING)
                         ? DISPLAY_METRIC_REMAINING
                         : DISPLAY_METRIC_USED;
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
                                ? DISPLAY_METRIC_REMAINING
                                : DISPLAY_METRIC_USED;
    settings_set_display_metric(next);
    return next;
}

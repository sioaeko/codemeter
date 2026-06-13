#pragma once
#include <stdint.h>
#include <lvgl.h>

// Initialize the generic splash/status module inside `parent`.
void splash_init(lv_obj_t *parent);

// Advance the abstract meter animation if its hold time elapsed.
void splash_tick(void);

// Advance to the next abstract animation phase.
void splash_next(void);

// Show/hide the splash container.
void splash_show(void);
void splash_hide(void);

// Refresh the splash accent for the current usage-rate group.
void splash_pick_for_current_rate(void);

// True when splash is currently rendering.
bool splash_is_active(void);

// True when a screen coordinate lands on the central splash art.
bool splash_point_hits_art(int16_t x, int16_t y);

// Root container (so ui.cpp can attach a click event).
lv_obj_t* splash_get_root(void);

#include "splash.h"
#include "usage_rate.h"
#include "hal/board_caps.h"
#include <Arduino.h>

#define BAR_COUNT 5

static lv_obj_t* splash_container = nullptr;
static lv_obj_t* title_label = nullptr;
static lv_obj_t* subtitle_label = nullptr;
static lv_obj_t* bars[BAR_COUNT] = {};
static bool active = false;
static uint8_t phase = 0;
static uint32_t last_tick_ms = 0;
static int16_t art_x = 0;
static int16_t art_y = 0;
static int16_t art_w = 0;
static int16_t art_h = 0;

static lv_color_t bg_color(void) { return lv_color_hex(0x05070a); }
static lv_color_t panel_color(void) { return lv_color_hex(0x141a20); }
static lv_color_t text_color(void) { return lv_color_hex(0xf6f8fa); }
static lv_color_t dim_color(void) { return lv_color_hex(0x8d99a6); }

static lv_color_t accent_color(void) {
    switch (usage_rate_group()) {
    case 1: return lv_color_hex(0x5db8a6);
    case 2: return lv_color_hex(0xd9a441);
    case 3: return lv_color_hex(0xd9825b);
    default: return lv_color_hex(0x18a085);
    }
}

static const lv_font_t* title_font(void) {
    return board_caps().height <= 260 ? &lv_font_montserrat_24 : &lv_font_montserrat_28;
}

static const lv_font_t* body_font(void) {
    return board_caps().height <= 260 ? &lv_font_montserrat_12 : &lv_font_montserrat_14;
}

static void layout_art_bounds(void) {
    const BoardCaps& c = board_caps();
    int16_t min_dim = (c.width < c.height) ? c.width : c.height;
    art_w = (int16_t)(min_dim * 2 / 3);
    if (art_w < 120) art_w = 120;
    if (art_w > 220) art_w = 220;
    art_h = (int16_t)(art_w * 3 / 4);
    art_x = (int16_t)((c.width - art_w) / 2);
    art_y = (int16_t)((c.height - art_h) / 2);
}

static void update_bars(void) {
    if (!bars[0]) return;

    static const uint8_t heights[BAR_COUNT][BAR_COUNT] = {
        { 32, 54, 78, 50, 36 },
        { 44, 72, 48, 84, 40 },
        { 66, 40, 88, 58, 76 },
        { 84, 56, 42, 74, 94 },
        { 52, 88, 64, 44, 70 },
    };

    const lv_color_t accent = accent_color();
    const int16_t gap = 8;
    const int16_t bar_w = (art_w - gap * (BAR_COUNT - 1)) / BAR_COUNT;
    const int16_t baseline = art_y + art_h - 18;
    const uint8_t row = phase % BAR_COUNT;

    for (int i = 0; i < BAR_COUNT; i++) {
        int16_t h = (int16_t)((art_h - 34) * heights[row][i] / 100);
        lv_obj_set_size(bars[i], bar_w, h);
        lv_obj_set_pos(bars[i], art_x + i * (bar_w + gap), baseline - h);
        lv_obj_set_style_bg_color(bars[i], accent, 0);
    }
}

void splash_init(lv_obj_t* parent) {
    const BoardCaps& c = board_caps();
    layout_art_bounds();

    splash_container = lv_obj_create(parent);
    lv_obj_set_size(splash_container, c.width, c.height);
    lv_obj_set_pos(splash_container, 0, 0);
    lv_obj_set_style_bg_color(splash_container, bg_color(), 0);
    lv_obj_set_style_bg_opa(splash_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(splash_container, 0, 0);
    lv_obj_set_style_pad_all(splash_container, 0, 0);
    lv_obj_clear_flag(splash_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(splash_container, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* panel = lv_obj_create(splash_container);
    lv_obj_set_size(panel, art_w + 26, art_h + 28);
    lv_obj_set_pos(panel, art_x - 13, art_y - 14);
    lv_obj_set_style_bg_color(panel, panel_color(), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 8, 0);
    lv_obj_set_style_pad_all(panel, 0, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);

    for (int i = 0; i < BAR_COUNT; i++) {
        bars[i] = lv_obj_create(splash_container);
        lv_obj_set_style_bg_color(bars[i], accent_color(), 0);
        lv_obj_set_style_bg_opa(bars[i], LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(bars[i], 0, 0);
        lv_obj_set_style_radius(bars[i], 4, 0);
        lv_obj_set_style_pad_all(bars[i], 0, 0);
        lv_obj_clear_flag(bars[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(bars[i], LV_OBJ_FLAG_EVENT_BUBBLE);
    }

    title_label = lv_label_create(splash_container);
    lv_label_set_text(title_label, "CodeMeter");
    lv_obj_set_style_text_font(title_label, title_font(), 0);
    lv_obj_set_style_text_color(title_label, text_color(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, c.height <= 260 ? 18 : 32);
    lv_obj_add_flag(title_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    subtitle_label = lv_label_create(splash_container);
    lv_label_set_text(subtitle_label, "BLE display");
    lv_obj_set_style_text_font(subtitle_label, body_font(), 0);
    lv_obj_set_style_text_color(subtitle_label, dim_color(), 0);
    lv_obj_align(subtitle_label, LV_ALIGN_BOTTOM_MID, 0, c.height <= 260 ? -14 : -26);
    lv_obj_add_flag(subtitle_label, LV_OBJ_FLAG_EVENT_BUBBLE);

    update_bars();
    lv_obj_add_flag(splash_container, LV_OBJ_FLAG_HIDDEN);
}

void splash_tick(void) {
    if (!active) return;
    uint32_t now = millis();
    if (now - last_tick_ms < 180) return;
    last_tick_ms = now;
    phase = (uint8_t)((phase + 1) % BAR_COUNT);
    update_bars();
}

void splash_next(void) {
    phase = (uint8_t)((phase + 1) % BAR_COUNT);
    update_bars();
}

void splash_pick_for_current_rate(void) {
    update_bars();
}

bool splash_is_active(void) {
    return active;
}

bool splash_point_hits_art(int16_t x, int16_t y) {
    return active &&
           x >= art_x && x < art_x + art_w &&
           y >= art_y && y < art_y + art_h;
}

void splash_show(void) {
    if (splash_container) lv_obj_clear_flag(splash_container, LV_OBJ_FLAG_HIDDEN);
    active = true;
    last_tick_ms = millis();
    update_bars();
}

void splash_hide(void) {
    if (splash_container) lv_obj_add_flag(splash_container, LV_OBJ_FLAG_HIDDEN);
    active = false;
}

lv_obj_t* splash_get_root(void) {
    return splash_container;
}

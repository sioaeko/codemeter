#include "ui.h"
#include "splash.h"
#include <lvgl.h>
#include "logo.h"
#include "logo_badge_tiny.h"
#include "icons.h"
#include "icons_small.h"
#include "hal/board_caps.h"
#include "settings.h"

// Custom fonts (scaled for 314 PPI, ~1.9x from original 165 PPI)
LV_FONT_DECLARE(font_tiempos_56);
LV_FONT_DECLARE(font_tiempos_34);
LV_FONT_DECLARE(font_styrene_48);
LV_FONT_DECLARE(font_styrene_28);
LV_FONT_DECLARE(font_styrene_24);
LV_FONT_DECLARE(font_styrene_20);
LV_FONT_DECLARE(font_styrene_16);
LV_FONT_DECLARE(font_styrene_14);
LV_FONT_DECLARE(font_styrene_12);
LV_FONT_DECLARE(font_mono_32);
LV_FONT_DECLARE(font_mono_18);

// Layout values computed from the active board's geometry. Populated once
// in ui_init() and treated as const for the rest of the program. Adding a
// new display size means extending compute_layout() with another
// breakpoint — never editing the screen-builder functions below.
struct Layout {
    int16_t scr_w, scr_h;
    int16_t margin;
    int16_t title_y;
    int16_t title_x_offset;
    int16_t content_y;
    int16_t content_w;
    int16_t panel_pad;
    int16_t panel_pad_y;
    bool show_logo;

    // Usage screen
    int16_t usage_panel_h;
    int16_t usage_panel_gap;
    int16_t usage_bar_y;
    int16_t usage_bar_h;
    int16_t usage_reset_y;
    int16_t spinner_bottom;
    const lv_font_t* title_font;
    const lv_font_t* usage_pct_font;
    const lv_font_t* usage_pill_font;
    const lv_font_t* usage_reset_font;
    const lv_font_t* spinner_font;

    // Bluetooth screen
    int16_t bt_info_panel_h;
    int16_t bt_reset_zone_h;
    const lv_font_t* bt_title_font;
    const lv_font_t* bt_status_font;
    const lv_font_t* bt_device_font;
    const lv_font_t* bt_credit_1_font;
    const lv_font_t* bt_credit_2_font;
};
static Layout L = {};

// Pick layout values from the active board's pixel dimensions. The two
// existing boards happen to land on the two breakpoints below; new ports
// inherit the closer one — visually OK, may need a polish pass for
// pixel-perfect alignment but never blocks the port from booting.
static void compute_layout(const BoardCaps& c) {
    L.scr_w = c.width;
    L.scr_h = c.height;
    L.margin = 20;
    L.title_y = 30;
    L.title_x_offset = 16;
    L.panel_pad = 16;
    L.panel_pad_y = 12;
    L.show_logo = true;

    if (c.height >= 460) {
        // Large layout — tuned for 480x480 (AMOLED-2.16).
        L.content_y = 100;
        L.usage_panel_h = 150;
        L.usage_panel_gap = 16;
        L.usage_bar_y = 56;
        L.usage_bar_h = 24;
        L.usage_reset_y = 94;
        L.spinner_bottom = 15;
        L.title_font      = &font_tiempos_56;
        L.usage_pct_font  = &font_styrene_48;
        L.usage_pill_font = &font_styrene_28;
        L.usage_reset_font = &font_styrene_28;
        L.spinner_font    = &font_mono_32;
        L.bt_info_panel_h = 160;
        L.bt_reset_zone_h = 110;
        L.bt_title_font    = &font_tiempos_56;
        L.bt_status_font   = &font_styrene_48;
        L.bt_device_font   = &font_styrene_28;
        L.bt_credit_1_font = &font_styrene_24;
        L.bt_credit_2_font = &font_styrene_20;
    } else if (c.width >= 300 && c.height <= 260) {
        // Tiny landscape layout — tuned for 320x240 CYD.
        L.margin = 16;
        L.title_y = 12;
        L.title_x_offset = 0;
        L.content_y = 60;
        L.panel_pad = 10;
        L.panel_pad_y = 6;
        L.show_logo = true;
        L.usage_panel_h = 72;
        L.usage_panel_gap = 8;
        L.usage_bar_y = 34;
        L.usage_bar_h = 12;
        L.usage_reset_y = 48;
        L.spinner_bottom = 3;
        L.title_font      = &font_tiempos_34;
        L.usage_pct_font  = &font_styrene_24;
        L.usage_pill_font = &font_styrene_14;
        L.usage_reset_font = &font_styrene_12;
        L.spinner_font    = &font_mono_18;
        L.bt_info_panel_h = 92;
        L.bt_reset_zone_h = 44;
        L.bt_title_font    = &font_tiempos_34;
        L.bt_status_font   = &font_styrene_20;
        L.bt_device_font   = &font_styrene_14;
        L.bt_credit_1_font = &font_styrene_14;
        L.bt_credit_2_font = &font_styrene_12;
    } else if (c.width <= 260 && c.height <= 340) {
        // Tiny portrait layout — tuned for 240x320 CYD.
        L.margin = 20;
        L.title_y = 18;
        L.title_x_offset = 0;
        L.content_y = 70;
        L.panel_pad = 10;
        L.panel_pad_y = 8;
        L.show_logo = true;
        L.usage_panel_h = 86;
        L.usage_panel_gap = 8;
        L.usage_bar_y = 42;
        L.usage_bar_h = 16;
        L.usage_reset_y = 58;
        L.spinner_bottom = 8;
        L.title_font      = &font_tiempos_34;
        L.usage_pct_font  = &font_styrene_28;
        L.usage_pill_font = &font_styrene_16;
        L.usage_reset_font = &font_styrene_14;
        L.spinner_font    = &font_mono_18;
        L.bt_info_panel_h = 108;
        L.bt_reset_zone_h = 60;
        L.bt_title_font    = &font_tiempos_34;
        L.bt_status_font   = &font_styrene_20;
        L.bt_device_font   = &font_styrene_14;
        L.bt_credit_1_font = &font_styrene_14;
        L.bt_credit_2_font = &font_styrene_14;
    } else {
        // Compact layout — tuned for 368x448 (AMOLED-1.8).
        L.content_y = 85;
        L.usage_panel_h = 130;
        L.usage_panel_gap = 12;
        L.usage_bar_y = 48;
        L.usage_bar_h = 24;
        L.usage_reset_y = 78;
        L.spinner_bottom = 15;
        L.title_font      = &font_tiempos_56;
        L.usage_pct_font  = &font_styrene_48;
        L.usage_pill_font = &font_styrene_28;
        L.usage_reset_font = &font_styrene_28;
        L.spinner_font    = &font_mono_32;
        L.bt_info_panel_h = 140;
        L.bt_reset_zone_h = 90;
        L.bt_title_font    = &font_tiempos_34;
        L.bt_status_font   = &font_styrene_28;
        L.bt_device_font   = &font_styrene_20;
        L.bt_credit_1_font = &font_styrene_16;
        L.bt_credit_2_font = &font_styrene_14;
    }

    L.content_w = L.scr_w - 2 * L.margin;
}

// Base design tokens live in theme.h; runtime palette choices are persisted
// in settings.cpp and resolved here.
#include "theme.h"

static bool theme_is_light(void) {
    return settings_display_theme() == DISPLAY_THEME_LIGHT;
}

static bool accent_is_claude(void) {
    return settings_accent_theme() == ACCENT_THEME_CLAUDE;
}

static lv_color_t col_accent(void) {
    return accent_is_claude() ? lv_color_hex(0xd97757) : lv_color_hex(0x10a37f);
}

static lv_color_t col_bg(void) {
    return theme_is_light() ? lv_color_hex(0xf3eadb) : lv_color_hex(0x000000);
}

static lv_color_t col_panel(void) {
    return theme_is_light() ? lv_color_hex(0xfffbf2) : lv_color_hex(0x1f1f1e);
}

static lv_color_t col_text(void) {
    return theme_is_light() ? lv_color_hex(0x2d2b27) : lv_color_hex(0xfaf9f5);
}

static lv_color_t col_dim(void) {
    return theme_is_light() ? lv_color_hex(0x6f6258) : lv_color_hex(0xb0aea5);
}

static lv_color_t col_green(void) {
    if (accent_is_claude()) return theme_is_light() ? lv_color_hex(0x6f7f4e) : lv_color_hex(0x788c5d);
    return lv_color_hex(0x10a37f);
}

static lv_color_t col_amber(void) {
    return accent_is_claude() ? lv_color_hex(0xd97757) : lv_color_hex(0xd9a441);
}

static lv_color_t col_red(void) {
    return theme_is_light() ? lv_color_hex(0xb84432) : lv_color_hex(0xc0392b);
}

static lv_color_t col_bar_bg(void) {
    return theme_is_light() ? lv_color_hex(0xe3d3bf) : lv_color_hex(0x2a2a28);
}

#define COL_BG        col_bg()
#define COL_PANEL     col_panel()
#define COL_TEXT      col_text()
#define COL_DIM       col_dim()
#define COL_ACCENT    col_accent()
#define COL_GREEN     col_green()
#define COL_AMBER     col_amber()
#define COL_RED       col_red()
#define COL_BAR_BG    col_bar_bg()

// ---- Usage screen widgets ----
static lv_obj_t* usage_container;
static lv_obj_t* lbl_title;
static lv_obj_t* bar_session;
static lv_obj_t* lbl_session_pct;
static lv_obj_t* lbl_session_label;
static lv_obj_t* lbl_session_reset;
static lv_obj_t* bar_weekly;
static lv_obj_t* lbl_weekly_pct;
static lv_obj_t* lbl_weekly_label;
static lv_obj_t* lbl_weekly_reset;
static lv_obj_t* lbl_anim;

// ---- Bluetooth screen widgets ----
static lv_obj_t* ble_container;
static lv_obj_t* lbl_ble_status;
static lv_obj_t* lbl_ble_device;
static lv_obj_t* lbl_ble_mac;

// ---- Settings screen widgets ----
static lv_obj_t* settings_container;
static lv_obj_t* lbl_metric_value;
static lv_obj_t* lbl_theme_value;
static lv_obj_t* lbl_accent_value;
static lv_obj_t* lbl_bluetooth_value;
static lv_obj_t* lbl_settings_note;

// ---- Battery indicator (shared, on top) ----
static lv_obj_t* battery_img;
static lv_obj_t* logo_img;
static lv_obj_t* img_ble_icon;
static lv_obj_t* img_trash_icon;
static lv_image_dsc_t battery_dscs[5];  // empty, low, medium, full, charging

// ---- Shared ----
static lv_image_dsc_t logo_dsc;
static lv_image_dsc_t logo_badge_tiny_dsc;
static screen_t current_screen = SCREEN_USAGE;
static UsageData last_usage = {};
static ble_state_t last_ui_ble_state = BLE_STATE_INIT;
static uint32_t splash_started_ms = 0;
static bool splash_hold_requested = false;

#define SPLASH_AUTO_USAGE_MS 3500

#define MAX_THEME_OBJS 64
static lv_obj_t* theme_panels[MAX_THEME_OBJS];
static uint8_t theme_panel_count = 0;
static lv_obj_t* theme_bars[MAX_THEME_OBJS];
static uint8_t theme_bar_count = 0;
static lv_obj_t* theme_pills[MAX_THEME_OBJS];
static uint8_t theme_pill_count = 0;
static lv_obj_t* theme_primary_labels[MAX_THEME_OBJS];
static uint8_t theme_primary_count = 0;
static lv_obj_t* theme_dim_labels[MAX_THEME_OBJS];
static uint8_t theme_dim_count = 0;
static lv_obj_t* theme_accent_labels[MAX_THEME_OBJS];
static uint8_t theme_accent_count = 0;

static void register_obj(lv_obj_t** list, uint8_t* count, lv_obj_t* obj) {
    if (!obj || *count >= MAX_THEME_OBJS) return;
    list[(*count)++] = obj;
}

static void register_panel(lv_obj_t* obj)  { register_obj(theme_panels, &theme_panel_count, obj); }
static void register_bar(lv_obj_t* obj)    { register_obj(theme_bars, &theme_bar_count, obj); }
static void register_pill(lv_obj_t* obj)   { register_obj(theme_pills, &theme_pill_count, obj); }
static void register_primary(lv_obj_t* obj){ register_obj(theme_primary_labels, &theme_primary_count, obj); }
static void register_dim(lv_obj_t* obj)    { register_obj(theme_dim_labels, &theme_dim_count, obj); }
static void register_accent(lv_obj_t* obj) { register_obj(theme_accent_labels, &theme_accent_count, obj); }

// Animation state
static uint32_t anim_last_ms = 0;
static uint8_t anim_spinner_idx = 0;
static uint8_t anim_phase = 0;
static uint8_t anim_msg_idx = 0;
static uint32_t anim_msg_start = 0;
#define ANIM_MSG_MS     4000

static const char* const spinner_frames[] = {
    "\xC2\xB7", "\xE2\x9C\xBB", "\xE2\x9C\xBD",
    "\xE2\x9C\xB6", "\xE2\x9C\xB3", "\xE2\x9C\xA2",
};
#define SPINNER_COUNT 6
#define SPINNER_PHASES (2 * (SPINNER_COUNT - 1))  // 10: ping-pong 0..5..0

static const uint16_t spinner_ms[SPINNER_COUNT] = {
    260, 130, 130, 130, 130, 260,
};

static const char* const anim_messages[] = {
    "Accomplishing", "Elucidating", "Perusing",
    "Actioning", "Enchanting", "Philosophising",
    "Actualizing", "Envisioning", "Pondering",
    "Baking", "Finagling", "Pontificating",
    "Booping", "Flibbertigibbeting", "Processing",
    "Brewing", "Forging", "Puttering",
    "Calculating", "Forming", "Puzzling",
    "Cerebrating", "Frolicking", "Reticulating",
    "Channelling", "Generating", "Ruminating",
    "Churning", "Germinating", "Scheming",
    "Clauding", "Hatching", "Schlepping",
    "Coalescing", "Herding", "Shimmying",
    "Cogitating", "Honking", "Shucking",
    "Combobulating", "Hustling", "Simmering",
    "Computing", "Ideating", "Smooshing",
    "Concocting", "Imagining", "Spelunking",
    "Conjuring", "Incubating", "Spinning",
    "Considering", "Inferring", "Stewing",
    "Contemplating", "Jiving", "Sussing",
    "Cooking", "Manifesting", "Synthesizing",
    "Crafting", "Marinating", "Thinking",
    "Creating", "Meandering", "Tinkering",
    "Crunching", "Moseying", "Transmuting",
    "Deciphering", "Mulling", "Unfurling",
    "Deliberating", "Mustering", "Unravelling",
    "Determining", "Musing", "Vibing",
    "Discombobulating", "Noodling", "Wandering",
    "Divining", "Percolating", "Whirring",
    "Doing", "Wibbling",
    "Effecting", "Wizarding",
    "Working", "Wrangling",
};
#define ANIM_MSG_COUNT (sizeof(anim_messages) / sizeof(anim_messages[0]))

static lv_color_t pct_color(float pct) {
    if (pct >= 80.0f) return COL_RED;
    if (pct >= 50.0f) return COL_AMBER;
    return COL_GREEN;
}

static float clamp_pct(float pct) {
    if (pct < 0.0f) return 0.0f;
    if (pct > 100.0f) return 100.0f;
    return pct;
}

static float display_pct_from_used(float used_pct) {
    float used = clamp_pct(used_pct);
    if (settings_display_metric() == DISPLAY_METRIC_USAGE) {
        return clamp_pct(100.0f - used);
    }
    return used;
}

static lv_color_t display_pct_color(float shown_pct) {
    (void)shown_pct;
    return COL_ACCENT;
}

static const char* metric_label(void) {
    return (settings_display_metric() == DISPLAY_METRIC_USAGE)
               ? "Usage"
               : "Used";
}

static const char* theme_label(void) {
    return (settings_display_theme() == DISPLAY_THEME_LIGHT)
               ? "Light"
               : "Dark";
}

static const char* accent_label(void) {
    return (settings_accent_theme() == ACCENT_THEME_CLAUDE)
               ? "Claude"
               : "Codex";
}

static void refresh_settings_labels(void) {
    const char* label = metric_label();
    if (lbl_title) lv_label_set_text(lbl_title, "Usage");
    if (lbl_metric_value) lv_label_set_text(lbl_metric_value, label);
    if (lbl_theme_value) lv_label_set_text(lbl_theme_value, theme_label());
    if (lbl_accent_value) lv_label_set_text(lbl_accent_value, accent_label());
    if (lbl_bluetooth_value) lv_label_set_text(lbl_bluetooth_value, "Open");
    if (lbl_settings_note) lv_label_set_text(lbl_settings_note, "BACK exits");
}

static void format_reset_time(int mins, char* buf, size_t len) {
    if (mins < 0) {
        snprintf(buf, len, "---");
    } else if (L.scr_h <= 340) {
        if (mins < 60) {
            snprintf(buf, len, "Reset %dm", mins);
        } else if (mins < 1440) {
            snprintf(buf, len, "Reset %dh %dm", mins / 60, mins % 60);
        } else {
            snprintf(buf, len, "Reset %dd %dh", mins / 1440, (mins % 1440) / 60);
        }
    } else if (mins < 60) {
        snprintf(buf, len, "Resets in %dm", mins);
    } else if (mins < 1440) {
        snprintf(buf, len, "Resets in %dh %dm", mins / 60, mins % 60);
    } else {
        snprintf(buf, len, "Resets in %dd %dh", mins / 1440, (mins % 1440) / 60);
    }
}

// Forward decls — callbacks defined near ui_show_screen below
static void global_click_cb(lv_event_t* e);
static void logo_click_cb(lv_event_t* e);
static void ble_reset_click_cb(lv_event_t* e);
static void settings_metric_click_cb(lv_event_t* e);
static void settings_theme_click_cb(lv_event_t* e);
static void settings_accent_click_cb(lv_event_t* e);
static void settings_bluetooth_click_cb(lv_event_t* e);
static void settings_button_click_cb(lv_event_t* e);
static void settings_back_click_cb(lv_event_t* e);
static void apply_theme_styles(void);

static int panel_radius(void) {
    return 8;
}

static lv_obj_t* make_panel(lv_obj_t* parent, int x, int y, int w, int h) {
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_set_pos(panel, x, y);
    lv_obj_set_size(panel, w, h);
    lv_obj_set_style_bg_color(panel, COL_PANEL, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(panel, panel_radius(), 0);
    lv_obj_set_style_border_width(panel, (L.scr_h <= 340) ? 0 : 1, 0);
    lv_obj_set_style_border_color(panel, COL_BAR_BG, 0);
    lv_obj_set_style_border_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_left(panel, L.panel_pad, 0);
    lv_obj_set_style_pad_right(panel, L.panel_pad, 0);
    lv_obj_set_style_pad_top(panel, L.panel_pad_y, 0);
    lv_obj_set_style_pad_bottom(panel, L.panel_pad_y, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);
    register_panel(panel);
    return panel;
}

static lv_obj_t* make_bar(lv_obj_t* parent, int x, int y, int w, int h) {
    lv_obj_t* bar = lv_bar_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, w, h);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar, COL_BAR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, COL_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 6, LV_PART_INDICATOR);
    register_bar(bar);
    return bar;
}

static void init_icon_dsc_rgb565a8(lv_image_dsc_t* dsc, int w, int h, const uint8_t* data) {
    dsc->header.w = w;
    dsc->header.h = h;
    dsc->header.cf = LV_COLOR_FORMAT_RGB565A8;
    dsc->header.stride = w * 2;
    dsc->data = data;
    dsc->data_size = w * h * 3;
}

static lv_obj_t* make_pill(lv_obj_t* parent, const char* text) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, L.usage_pill_font, 0);
    lv_obj_set_style_text_color(lbl, COL_TEXT, 0);
    lv_obj_set_style_bg_color(lbl, COL_BAR_BG, 0);
    lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(lbl, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_left(lbl, L.panel_pad, 0);
    lv_obj_set_style_pad_right(lbl, L.panel_pad, 0);
    lv_obj_set_style_pad_top(lbl, 6, 0);
    lv_obj_set_style_pad_bottom(lbl, 6, 0);
    register_pill(lbl);
    return lbl;
}

static lv_obj_t* make_settings_button(lv_obj_t* parent) {
    lv_obj_t* btn = lv_obj_create(parent);
    bool tiny = L.scr_h <= 340;
    lv_obj_set_size(btn, tiny ? 54 : 68, tiny ? 28 : 36);
    lv_obj_set_ext_click_area(btn, tiny ? 8 : 6);
    lv_obj_set_style_bg_color(btn, COL_BAR_BG, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(btn, settings_button_click_cb, LV_EVENT_CLICKED, NULL);
    register_pill(btn);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "SET");
    lv_obj_set_style_text_font(label, L.usage_reset_font, 0);
    lv_obj_set_style_text_color(label, COL_ACCENT, 0);
    lv_obj_center(label);
    register_accent(label);

    int edge = (L.scr_h <= 340) ? 6 : L.margin;
    int y = tiny ? 7 : L.title_y - 1;
    int battery_offset = board_caps().has_battery ? 54 : 0;
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -edge - battery_offset, y);
    return btn;
}

static lv_obj_t* make_back_button(lv_obj_t* parent) {
    lv_obj_t* btn = lv_obj_create(parent);
    bool tiny = L.scr_h <= 340;
    lv_obj_set_size(btn, tiny ? 58 : 76, tiny ? 28 : 36);
    lv_obj_set_ext_click_area(btn, tiny ? 8 : 6);
    lv_obj_set_style_bg_color(btn, COL_BAR_BG, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(btn, settings_back_click_cb, LV_EVENT_CLICKED, NULL);
    register_pill(btn);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, "BACK");
    lv_obj_set_style_text_font(label, L.usage_reset_font, 0);
    lv_obj_set_style_text_color(label, COL_ACCENT, 0);
    lv_obj_center(label);
    register_accent(label);

    int edge = (L.scr_h <= 340) ? 6 : L.margin;
    int y = tiny ? 7 : L.title_y - 1;
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, edge, y);
    return btn;
}

static void init_battery_icons(void) {
    init_icon_dsc_rgb565a8(&battery_dscs[0], ICON_BATTERY_W, ICON_BATTERY_H, icon_battery_data);
    init_icon_dsc_rgb565a8(&battery_dscs[1], ICON_BATTERY_LOW_W, ICON_BATTERY_LOW_H, icon_battery_low_data);
    init_icon_dsc_rgb565a8(&battery_dscs[2], ICON_BATTERY_MEDIUM_W, ICON_BATTERY_MEDIUM_H, icon_battery_medium_data);
    init_icon_dsc_rgb565a8(&battery_dscs[3], ICON_BATTERY_FULL_W, ICON_BATTERY_FULL_H, icon_battery_full_data);
    init_icon_dsc_rgb565a8(&battery_dscs[4], ICON_BATTERY_CHARGING_W, ICON_BATTERY_CHARGING_H, icon_battery_charging_data);
}

// ======== Usage Screen ========

static void make_usage_panel(lv_obj_t* parent, int y, const char* pill_text,
                             lv_obj_t** out_pct, lv_obj_t** out_pill,
                             lv_obj_t** out_bar, lv_obj_t** out_reset) {
    lv_obj_t* panel = make_panel(parent, L.margin, y, L.content_w, L.usage_panel_h);

    *out_pct = lv_label_create(panel);
    lv_label_set_text(*out_pct, "---%");
    lv_obj_set_style_text_font(*out_pct, L.usage_pct_font, 0);
    lv_obj_set_style_text_color(*out_pct, COL_TEXT, 0);
    lv_obj_set_pos(*out_pct, 0, 0);
    register_primary(*out_pct);

    *out_pill = make_pill(panel, pill_text);
    lv_obj_align(*out_pill, LV_ALIGN_TOP_RIGHT, 0, 1);

    *out_bar = make_bar(panel, 0, L.usage_bar_y,
                        L.content_w - (2 * L.panel_pad), L.usage_bar_h);

    *out_reset = lv_label_create(panel);
    lv_label_set_text(*out_reset, "---");
    lv_obj_set_style_text_font(*out_reset, L.usage_reset_font, 0);
    lv_obj_set_style_text_color(*out_reset, COL_DIM, 0);
    lv_obj_set_pos(*out_reset, 0, L.usage_reset_y);
    lv_obj_set_width(*out_reset, L.content_w - (2 * L.panel_pad));
    lv_label_set_long_mode(*out_reset, LV_LABEL_LONG_CLIP);
    register_dim(*out_reset);
}

static void init_usage_screen(lv_obj_t* scr) {
    usage_container = lv_obj_create(scr);
    lv_obj_set_size(usage_container, L.scr_w, L.scr_h);
    lv_obj_set_pos(usage_container, 0, 0);
    lv_obj_set_style_bg_opa(usage_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(usage_container, 0, 0);
    lv_obj_set_style_pad_all(usage_container, 0, 0);
    lv_obj_clear_flag(usage_container, LV_OBJ_FLAG_SCROLLABLE);

    lbl_title = lv_label_create(usage_container);
    lv_label_set_text(lbl_title, "Usage");
    lv_obj_set_style_text_font(lbl_title, L.title_font, 0);
    lv_obj_set_style_text_color(lbl_title, COL_TEXT, 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, L.title_x_offset, L.title_y);
    register_primary(lbl_title);

    make_usage_panel(usage_container, L.content_y, "5h",
                     &lbl_session_pct, &lbl_session_label,
                     &bar_session, &lbl_session_reset);
    make_usage_panel(usage_container,
                     L.content_y + L.usage_panel_h + L.usage_panel_gap, "Weekly",
                     &lbl_weekly_pct, &lbl_weekly_label,
                     &bar_weekly, &lbl_weekly_reset);

    lbl_anim = lv_label_create(usage_container);
    lv_label_set_text(lbl_anim, "");
    lv_obj_set_style_text_font(lbl_anim, L.spinner_font, 0);
    lv_obj_set_style_text_color(lbl_anim, COL_ACCENT, 0);
    lv_obj_align(lbl_anim, LV_ALIGN_BOTTOM_MID, 0, -L.spinner_bottom);
    register_accent(lbl_anim);

    make_settings_button(usage_container);
}

// ======== Bluetooth Screen ========

static void init_bluetooth_screen(lv_obj_t* scr) {
    const bool compact_display = (L.scr_h <= 340);

    ble_container = lv_obj_create(scr);
    lv_obj_set_size(ble_container, L.scr_w, L.scr_h);
    lv_obj_set_pos(ble_container, 0, 0);
    lv_obj_set_style_bg_opa(ble_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ble_container, 0, 0);
    lv_obj_set_style_pad_all(ble_container, 0, 0);
    lv_obj_clear_flag(ble_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* lbl_ble_title = lv_label_create(ble_container);
    lv_label_set_text(lbl_ble_title, "Bluetooth");
    const bool tiny_portrait = (L.scr_w <= 260 && L.scr_h <= 340);
    lv_obj_set_style_text_font(lbl_ble_title, tiny_portrait ? &font_styrene_24 : L.bt_title_font, 0);
    lv_obj_set_style_text_color(lbl_ble_title, COL_TEXT, 0);
    lv_obj_align(lbl_ble_title, LV_ALIGN_TOP_MID,
                 tiny_portrait ? -10 : L.title_x_offset,
                 tiny_portrait ? 20 : L.title_y);
    register_primary(lbl_ble_title);

    lv_obj_t* p_info = make_panel(ble_container, L.margin, L.content_y,
                                  L.content_w, L.bt_info_panel_h);

    static lv_image_dsc_t icon_bt_dsc;
    static lv_image_dsc_t icon_bt_small_dsc;
    init_icon_dsc_rgb565a8(&icon_bt_dsc, ICON_BLUETOOTH_W, ICON_BLUETOOTH_H, icon_bluetooth_data);
    init_icon_dsc_rgb565a8(&icon_bt_small_dsc, ICON_BLUETOOTH_SMALL_W, ICON_BLUETOOTH_SMALL_H, icon_bluetooth_small_data);

    img_ble_icon = lv_image_create(p_info);
    lv_image_set_src(img_ble_icon, compact_display ? &icon_bt_small_dsc : &icon_bt_dsc);
    lv_obj_set_pos(img_ble_icon, 0, 0);

    lbl_ble_status = lv_label_create(p_info);
    lv_label_set_text(lbl_ble_status, "Initializing...");
    lv_obj_set_style_text_font(lbl_ble_status, L.bt_status_font, 0);
    lv_obj_set_style_text_color(lbl_ble_status, COL_DIM, 0);
    lv_obj_set_pos(lbl_ble_status, compact_display ? 48 : 56, compact_display ? 7 : 2);

    lbl_ble_device = lv_label_create(p_info);
    lv_label_set_text(lbl_ble_device, "Device: ---");
    lv_obj_set_style_text_font(lbl_ble_device, L.bt_device_font, 0);
    lv_obj_set_style_text_color(lbl_ble_device, COL_DIM, 0);
    lv_obj_set_pos(lbl_ble_device, 0, (L.scr_h <= 260) ? 52 : ((L.scr_h <= 340) ? 58 : 64));
    register_dim(lbl_ble_device);

    lbl_ble_mac = lv_label_create(p_info);
    lv_label_set_text(lbl_ble_mac, "Address: ---");
    lv_obj_set_style_text_font(lbl_ble_mac, L.bt_device_font, 0);
    lv_obj_set_style_text_color(lbl_ble_mac, COL_DIM, 0);
    lv_obj_set_pos(lbl_ble_mac, 0, (L.scr_h <= 260) ? 72 : ((L.scr_h <= 340) ? 82 : 100));
    register_dim(lbl_ble_mac);

    int reset_y = L.content_y + L.bt_info_panel_h + 16;
    lv_obj_t* reset_zone = lv_obj_create(ble_container);
    lv_obj_set_pos(reset_zone, L.margin, reset_y);
    lv_obj_set_size(reset_zone, L.content_w, L.bt_reset_zone_h);
    lv_obj_set_style_bg_color(reset_zone, COL_PANEL, 0);
    lv_obj_set_style_bg_opa(reset_zone, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(reset_zone, panel_radius(), 0);
    lv_obj_set_style_border_width(reset_zone, 0, 0);
    lv_obj_set_style_pad_column(reset_zone, 14, 0);
    lv_obj_set_flex_flow(reset_zone, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(reset_zone, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(reset_zone, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(reset_zone, ble_reset_click_cb, LV_EVENT_CLICKED, NULL);
    register_panel(reset_zone);

    static lv_image_dsc_t icon_trash_dsc;
    static lv_image_dsc_t icon_trash_small_dsc;
    init_icon_dsc_rgb565a8(&icon_trash_dsc, ICON_TRASH2_W, ICON_TRASH2_H, icon_trash2_data);
    init_icon_dsc_rgb565a8(&icon_trash_small_dsc, ICON_TRASH2_SMALL_W, ICON_TRASH2_SMALL_H, icon_trash2_small_data);
    img_trash_icon = lv_image_create(reset_zone);
    lv_image_set_src(img_trash_icon, compact_display ? &icon_trash_small_dsc : &icon_trash_dsc);

    lv_obj_t* reset_lbl = lv_label_create(reset_zone);
    lv_label_set_text(reset_lbl, "Reset Bluetooth");
    lv_obj_set_style_text_font(reset_lbl, L.bt_device_font, 0);
    lv_obj_set_style_text_color(reset_lbl, COL_DIM, 0);
    register_dim(reset_lbl);

    if (L.scr_h > 260) {
        lv_obj_t* lbl_credit = lv_label_create(ble_container);
        lv_label_set_text(lbl_credit, "Clawdmeter-CYD");
        lv_obj_set_style_text_font(lbl_credit, L.bt_credit_1_font, 0);
        lv_obj_set_style_text_color(lbl_credit, COL_DIM, 0);
        lv_obj_align(lbl_credit, LV_ALIGN_BOTTOM_MID, 0, -46);
        register_dim(lbl_credit);

        lv_obj_t* lbl_credit2 = lv_label_create(ble_container);
        lv_label_set_text(lbl_credit2, "Built from Clawdmeter");
        lv_obj_set_style_text_font(lbl_credit2, L.bt_credit_2_font, 0);
        lv_obj_set_style_text_color(lbl_credit2, COL_DIM, 0);
        lv_obj_align(lbl_credit2, LV_ALIGN_BOTTOM_MID, 0, -20);
        register_dim(lbl_credit2);
    }

    make_settings_button(ble_container);

    lv_obj_add_flag(ble_container, LV_OBJ_FLAG_HIDDEN);
}

// ======== Settings Screen ========

static int settings_card_h(void) {
    if (L.scr_h <= 260) return 36;
    if (L.scr_h <= 340) return 48;
    return 70;
}

static int settings_card_gap(void) {
    return (L.scr_h <= 260) ? 6 : 8;
}

static lv_obj_t* make_settings_card(lv_obj_t* parent, int index, const char* name,
                                    lv_obj_t** out_value,
                                    lv_event_cb_t cb) {
    int h = settings_card_h();
    int y = L.content_y + index * (h + settings_card_gap());
    lv_obj_t* panel = make_panel(parent, L.margin, y, L.content_w, h);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_flag(panel, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(panel, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* lbl_name = lv_label_create(panel);
    lv_label_set_text(lbl_name, name);
    lv_obj_set_style_text_font(lbl_name, L.usage_reset_font, 0);
    lv_obj_set_style_text_color(lbl_name, COL_DIM, 0);
    lv_obj_set_width(lbl_name, (L.content_w - (2 * L.panel_pad)) / 2);
    lv_obj_align(lbl_name, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_long_mode(lbl_name, LV_LABEL_LONG_CLIP);
    register_dim(lbl_name);

    *out_value = lv_label_create(panel);
    lv_label_set_text(*out_value, "---");
    lv_obj_set_style_text_font(*out_value, L.usage_pill_font, 0);
    lv_obj_set_style_text_color(*out_value, COL_TEXT, 0);
    lv_obj_set_width(*out_value, (L.content_w - (2 * L.panel_pad)) / 2);
    lv_obj_set_style_text_align(*out_value, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_align(*out_value, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_label_set_long_mode(*out_value, LV_LABEL_LONG_CLIP);
    register_primary(*out_value);

    return panel;
}

static void init_settings_screen(lv_obj_t* scr) {
    settings_container = lv_obj_create(scr);
    lv_obj_set_size(settings_container, L.scr_w, L.scr_h);
    lv_obj_set_pos(settings_container, 0, 0);
    lv_obj_set_style_bg_opa(settings_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(settings_container, 0, 0);
    lv_obj_set_style_pad_all(settings_container, 0, 0);
    lv_obj_clear_flag(settings_container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(settings_container);
    lv_label_set_text(title, "Settings");
    const bool tiny_portrait = (L.scr_w <= 260 && L.scr_h <= 340);
    lv_obj_set_style_text_font(title, tiny_portrait ? &font_styrene_24 : L.title_font, 0);
    lv_obj_set_style_text_color(title, COL_TEXT, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID,
                 tiny_portrait ? 18 : L.title_x_offset,
                 tiny_portrait ? 20 : L.title_y);
    register_primary(title);
    make_back_button(settings_container);

    make_settings_card(settings_container, 0, "Display", &lbl_metric_value,
                       settings_metric_click_cb);
    make_settings_card(settings_container, 1, "Theme", &lbl_theme_value,
                       settings_theme_click_cb);
    make_settings_card(settings_container, 2, "Accent", &lbl_accent_value,
                       settings_accent_click_cb);
    make_settings_card(settings_container, 3, "Bluetooth", &lbl_bluetooth_value,
                       settings_bluetooth_click_cb);

    lbl_settings_note = lv_label_create(settings_container);
    lv_label_set_text(lbl_settings_note, "BACK exits");
    lv_obj_set_style_text_font(lbl_settings_note, L.usage_reset_font, 0);
    lv_obj_set_style_text_color(lbl_settings_note, COL_DIM, 0);
    lv_obj_set_width(lbl_settings_note, L.scr_w - 2 * L.margin);
    lv_obj_set_style_text_align(lbl_settings_note, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lbl_settings_note, LV_LABEL_LONG_CLIP);
    lv_obj_align(lbl_settings_note, LV_ALIGN_BOTTOM_MID, 0, -L.spinner_bottom);
    register_dim(lbl_settings_note);

    refresh_settings_labels();
    lv_obj_add_flag(settings_container, LV_OBJ_FLAG_HIDDEN);
}

static void apply_theme_styles(void) {
    lv_obj_t* scr = lv_screen_active();
    if (scr) {
        lv_obj_set_style_bg_color(scr, COL_BG, 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    }

    for (uint8_t i = 0; i < theme_panel_count; i++) {
        lv_obj_t* obj = theme_panels[i];
        lv_obj_set_style_bg_color(obj, COL_PANEL, 0);
        lv_obj_set_style_border_color(obj, COL_BAR_BG, 0);
    }

    for (uint8_t i = 0; i < theme_bar_count; i++) {
        lv_obj_t* obj = theme_bars[i];
        lv_obj_set_style_bg_color(obj, COL_BAR_BG, LV_PART_MAIN);
        lv_obj_set_style_bg_color(obj, COL_ACCENT, LV_PART_INDICATOR);
    }

    for (uint8_t i = 0; i < theme_pill_count; i++) {
        lv_obj_t* obj = theme_pills[i];
        lv_obj_set_style_text_color(obj, COL_TEXT, 0);
        lv_obj_set_style_bg_color(obj, COL_BAR_BG, 0);
    }

    for (uint8_t i = 0; i < theme_primary_count; i++) {
        lv_obj_set_style_text_color(theme_primary_labels[i], COL_TEXT, 0);
    }

    for (uint8_t i = 0; i < theme_dim_count; i++) {
        lv_obj_set_style_text_color(theme_dim_labels[i], COL_DIM, 0);
    }

    for (uint8_t i = 0; i < theme_accent_count; i++) {
        lv_obj_set_style_text_color(theme_accent_labels[i], COL_ACCENT, 0);
    }

    if (img_ble_icon) {
        lv_obj_set_style_image_recolor(img_ble_icon, COL_DIM, 0);
        lv_obj_set_style_image_recolor_opa(img_ble_icon, LV_OPA_COVER, 0);
    }
    if (img_trash_icon) {
        lv_obj_set_style_image_recolor(img_trash_icon, COL_DIM, 0);
        lv_obj_set_style_image_recolor_opa(img_trash_icon, LV_OPA_COVER, 0);
    }

    refresh_settings_labels();
    ui_update_ble_status(last_ui_ble_state, ble_get_device_name(), ble_get_mac_address());
    if (last_usage.valid) ui_update(&last_usage);
}

// ======== Public API ========

void ui_init(void) {
    compute_layout(board_caps());
    settings_init();

    lv_obj_t* scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, COL_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    init_icon_dsc_rgb565a8(&logo_dsc, LOGO_WIDTH, LOGO_HEIGHT, logo_data);
    init_icon_dsc_rgb565a8(&logo_badge_tiny_dsc, LOGO_BADGE_TINY_WIDTH, LOGO_BADGE_TINY_HEIGHT, logo_badge_tiny_data);
    init_battery_icons();

    init_usage_screen(scr);
    init_bluetooth_screen(scr);
    init_settings_screen(scr);
    splash_init(scr);

    if (splash_get_root()) {
        lv_obj_add_event_cb(splash_get_root(), global_click_cb, LV_EVENT_CLICKED, NULL);
    }

    if (L.show_logo) {
        logo_img = lv_image_create(scr);
        if (L.scr_h <= 340) {
            lv_image_set_src(logo_img, &logo_badge_tiny_dsc);
            lv_image_set_scale(logo_img, LV_SCALE_NONE);
            lv_obj_set_pos(logo_img, L.margin, L.content_y - LOGO_BADGE_TINY_HEIGHT);
        } else {
            lv_image_set_src(logo_img, &logo_dsc);
            lv_image_set_scale(logo_img, LV_SCALE_NONE);
            lv_obj_set_pos(logo_img, L.margin, L.title_y - 10);
        }
        lv_obj_add_flag(logo_img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_ext_click_area(logo_img, (L.scr_h <= 340) ? 12 : 8);
        lv_obj_add_event_cb(logo_img, logo_click_cb, LV_EVENT_CLICKED, NULL);
    }

    battery_img = lv_image_create(scr);
    lv_image_set_src(battery_img, &battery_dscs[0]);
    lv_obj_set_pos(battery_img, L.scr_w - 48 - L.margin, L.title_y);
    apply_theme_styles();
}

void ui_update(const UsageData* data) {
    if (!data->valid) return;

    last_usage = *data;

    float s_shown = display_pct_from_used(data->session_pct);
    int s_pct = (int)(s_shown + 0.5f);

    lv_label_set_text_fmt(lbl_session_pct, "%d%%", s_pct);
    lv_bar_set_value(bar_session, s_pct, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_session, display_pct_color(s_shown), LV_PART_INDICATOR);

    char buf[48];
    format_reset_time(data->session_reset_mins, buf, sizeof(buf));
    lv_label_set_text(lbl_session_reset, buf);

    float w_shown = display_pct_from_used(data->weekly_pct);
    int w_pct = (int)(w_shown + 0.5f);
    lv_label_set_text_fmt(lbl_weekly_pct, "%d%%", w_pct);
    lv_bar_set_value(bar_weekly, w_pct, LV_ANIM_ON);
    lv_obj_set_style_bg_color(bar_weekly, display_pct_color(w_shown), LV_PART_INDICATOR);

    format_reset_time(data->weekly_reset_mins, buf, sizeof(buf));
    lv_label_set_text(lbl_weekly_reset, buf);
}

void ui_tick_anim(void) {
    if (current_screen == SCREEN_SPLASH) {
        if (!splash_hold_requested &&
            (uint32_t)(lv_tick_get() - splash_started_ms) >= SPLASH_AUTO_USAGE_MS) {
            ui_show_screen(SCREEN_USAGE);
        }
        return;
    }

    if (current_screen != SCREEN_USAGE) return;

    uint32_t now = lv_tick_get();

    if (now - anim_msg_start >= ANIM_MSG_MS) {
        anim_msg_idx = (anim_msg_idx + 1) % ANIM_MSG_COUNT;
        anim_msg_start = now;
    }

    if (now - anim_last_ms >= spinner_ms[anim_spinner_idx]) {
        anim_last_ms = now;
        anim_phase = (anim_phase + 1) % SPINNER_PHASES;
        anim_spinner_idx = (anim_phase < SPINNER_COUNT) ? anim_phase
                                                        : (SPINNER_PHASES - anim_phase);

        static char buf[80];
        snprintf(buf, sizeof(buf), "%s %s\xE2\x80\xA6",
                 spinner_frames[anim_spinner_idx],
                 anim_messages[anim_msg_idx]);
        lv_label_set_text(lbl_anim, buf);
    }
}

static screen_t prev_non_splash_screen = SCREEN_USAGE;
static screen_t prev_non_settings_screen = SCREEN_USAGE;

static void show_settings_from_current(void) {
    if (current_screen != SCREEN_SETTINGS && current_screen != SCREEN_SPLASH) {
        prev_non_settings_screen = current_screen;
    }
    ui_show_screen(SCREEN_SETTINGS);
}

static void apply_battery_visibility(void) {
    if (!battery_img) return;
    if (!board_caps().has_battery || current_screen == SCREEN_SPLASH) {
        lv_obj_add_flag(battery_img, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(battery_img, LV_OBJ_FLAG_HIDDEN);
    }
}

static void global_click_cb(lv_event_t* e) {
    if (current_screen == SCREEN_SPLASH) {
        lv_indev_t* indev = lv_event_get_indev(e);
        if (indev) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            if (splash_point_hits_art(p.x, p.y)) {
                splash_hold_requested = true;
                return;
            }
        }
        ui_show_screen(SCREEN_USAGE);
        return;
    }
    show_settings_from_current();
}

static void logo_click_cb(lv_event_t* e) {
    (void)e;
    if (current_screen == SCREEN_USAGE || current_screen == SCREEN_BLUETOOTH) {
        ui_show_screen(SCREEN_SPLASH);
    }
}

static void ble_reset_click_cb(lv_event_t* e) {
    (void)e;
    ble_clear_bonds();
}

static void settings_metric_click_cb(lv_event_t* e) {
    (void)e;
    settings_toggle_display_metric();
    refresh_settings_labels();
    if (last_usage.valid) ui_update(&last_usage);
}

static void settings_theme_click_cb(lv_event_t* e) {
    (void)e;
    settings_toggle_display_theme();
    apply_theme_styles();
}

static void settings_accent_click_cb(lv_event_t* e) {
    (void)e;
    settings_toggle_accent_theme();
    apply_theme_styles();
}

static void settings_bluetooth_click_cb(lv_event_t* e) {
    (void)e;
    ui_show_screen(SCREEN_BLUETOOTH);
}

static void settings_button_click_cb(lv_event_t* e) {
    (void)e;
    show_settings_from_current();
}

static void settings_back_click_cb(lv_event_t* e) {
    (void)e;
    ui_show_screen(prev_non_settings_screen);
}

void ui_show_screen(screen_t screen) {
    lv_obj_add_flag(usage_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ble_container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(settings_container, LV_OBJ_FLAG_HIDDEN);
    splash_hide();

    lv_obj_t* scr = lv_screen_active();
    if (scr) {
        lv_obj_set_style_bg_color(scr, screen == SCREEN_SPLASH ? lv_color_hex(0x000000) : COL_BG, 0);
        lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    }

    switch (screen) {
    case SCREEN_SPLASH:
        splash_started_ms = lv_tick_get();
        splash_hold_requested = false;
        splash_show();
        break;
    case SCREEN_USAGE:      lv_obj_clear_flag(usage_container, LV_OBJ_FLAG_HIDDEN); break;
    case SCREEN_BLUETOOTH:  lv_obj_clear_flag(ble_container, LV_OBJ_FLAG_HIDDEN); break;
    case SCREEN_SETTINGS:   lv_obj_clear_flag(settings_container, LV_OBJ_FLAG_HIDDEN); break;
    default: break;
    }

    if (logo_img) {
        if (screen == SCREEN_USAGE || screen == SCREEN_BLUETOOTH) lv_obj_clear_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
        else                                                       lv_obj_add_flag(logo_img, LV_OBJ_FLAG_HIDDEN);
    }

    if (screen != SCREEN_SPLASH) prev_non_splash_screen = screen;
    current_screen = screen;
    apply_battery_visibility();
}

void ui_cycle_screen(void) {
    screen_t next;
    switch (current_screen) {
    case SCREEN_USAGE:     next = SCREEN_BLUETOOTH; break;
    case SCREEN_BLUETOOTH: next = SCREEN_SETTINGS;  break;
    case SCREEN_SETTINGS:  next = SCREEN_SPLASH;    break;
    case SCREEN_SPLASH:    next = SCREEN_USAGE;     break;
    default:               next = SCREEN_USAGE;     break;
    }
    ui_show_screen(next);
}

void ui_toggle_splash(void) {
    if (current_screen == SCREEN_SPLASH) ui_show_screen(prev_non_splash_screen);
    else                                  ui_show_screen(SCREEN_SPLASH);
}

screen_t ui_get_current_screen(void) {
    return current_screen;
}

bool ui_splash_is_held(void) {
    return current_screen == SCREEN_SPLASH && splash_hold_requested;
}

void ui_update_ble_status(ble_state_t state, const char* name, const char* mac) {
    last_ui_ble_state = state;
    switch (state) {
    case BLE_STATE_CONNECTED:
        lv_label_set_text(lbl_ble_status, "Connected");
        lv_obj_set_style_text_color(lbl_ble_status, COL_GREEN, 0);
        break;
    case BLE_STATE_ADVERTISING:
        lv_label_set_text(lbl_ble_status, "Advertising...");
        lv_obj_set_style_text_color(lbl_ble_status, COL_AMBER, 0);
        break;
    case BLE_STATE_DISCONNECTED:
        lv_label_set_text(lbl_ble_status, "Disconnected");
        lv_obj_set_style_text_color(lbl_ble_status, COL_RED, 0);
        break;
    default:
        lv_label_set_text(lbl_ble_status, "Initializing...");
        lv_obj_set_style_text_color(lbl_ble_status, COL_DIM, 0);
        break;
    }

    if (name) {
        static char nbuf[48];
        snprintf(nbuf, sizeof(nbuf), "Device: %s", name);
        lv_label_set_text(lbl_ble_device, nbuf);
    }
    if (mac) {
        static char mbuf[48];
        snprintf(mbuf, sizeof(mbuf), "Address: %s", mac);
        lv_label_set_text(lbl_ble_mac, mbuf);
    }
}

void ui_update_battery(int percent, bool charging) {
    int idx;
    if (charging) {
        idx = 4;
    } else if (percent < 0) {
        idx = 0;
    } else if (percent <= 10) {
        idx = 0;
    } else if (percent <= 35) {
        idx = 1;
    } else if (percent <= 75) {
        idx = 2;
    } else {
        idx = 3;
    }
    lv_image_set_src(battery_img, &battery_dscs[idx]);
    apply_battery_visibility();
}

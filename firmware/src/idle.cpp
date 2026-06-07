#include <Arduino.h>
#include "idle.h"
#include "idle_cfg.h"
#include "hal/display_hal.h"
#include "hal/power_hal.h"

enum IdleState {
    STATE_AWAKE,
    STATE_FADING_OUT,
    STATE_ASLEEP,
    STATE_FADING_IN,
};

static IdleState state = STATE_AWAKE;
static uint32_t last_activity_ms = 0;
static uint32_t fade_started_ms  = 0;
static uint32_t fade_last_step_ms = 0;
static uint8_t  fade_from = DISPLAY_DEFAULT_BRIGHTNESS;
static uint8_t  fade_to   = 0;
static bool     forced_sleep = false;
static uint32_t forced_wake_ms = 5UL * 60UL * 1000UL;
static uint32_t forced_wake_until_ms = 0;

static void apply_brightness(uint8_t b) {
    display_hal_set_brightness(b);
}

static void begin_fade(uint8_t to, uint32_t now) {
    fade_from = (to == 0) ? DISPLAY_DEFAULT_BRIGHTNESS : 0;
    fade_to   = to;
    fade_started_ms = now;
    fade_last_step_ms = now;
}

static bool time_reached(uint32_t now, uint32_t target) {
    return (int32_t)(now - target) >= 0;
}

static bool forced_sleep_should_apply(uint32_t now) {
    if (!forced_sleep) return false;
    if (forced_wake_until_ms == 0) return true;
    return time_reached(now, forced_wake_until_ms);
}

void idle_init(void) {
    state = STATE_AWAKE;
    last_activity_ms = millis();
    apply_brightness(DISPLAY_DEFAULT_BRIGHTNESS);
}

void idle_note_activity(void) {
    last_activity_ms = millis();
    if (state == STATE_FADING_IN) return;
    if (state == STATE_AWAKE) return;
    // Asleep/fading-out shouldn't reach here in normal flow (callers gate via
    // idle_consume_wake_press first), but if it does: trigger a wake.
    begin_fade(DISPLAY_DEFAULT_BRIGHTNESS, last_activity_ms);
    state = STATE_FADING_IN;
}

void idle_set_forced_sleep(bool forced, uint32_t wake_ms) {
    if (wake_ms < 1000UL) wake_ms = 1000UL;
    if (wake_ms > 60UL * 60UL * 1000UL) wake_ms = 60UL * 60UL * 1000UL;
    forced_wake_ms = wake_ms;

    if (forced) {
        forced_sleep = true;
        return;
    }

    bool was_forced = forced_sleep;
    forced_sleep = false;
    forced_wake_until_ms = 0;
    if (was_forced && (state == STATE_ASLEEP || state == STATE_FADING_OUT)) {
        uint32_t now = millis();
        last_activity_ms = now;
        begin_fade(DISPLAY_DEFAULT_BRIGHTNESS, now);
        state = STATE_FADING_IN;
    }
}

bool idle_consume_wake_press(void) {
    if (state == STATE_ASLEEP || state == STATE_FADING_OUT) {
        uint32_t now = millis();
        last_activity_ms = now;
        if (forced_sleep) {
            forced_wake_until_ms = now + forced_wake_ms;
        }
        begin_fade(DISPLAY_DEFAULT_BRIGHTNESS, now);
        state = STATE_FADING_IN;
        return true;
    }
    if (state == STATE_FADING_IN) {
        // Mid-wake: still swallow this press; the user shouldn't get a
        // half-wake half-action surprise.
        last_activity_ms = millis();
        return true;
    }
    last_activity_ms = millis();
    return false;
}

bool idle_is_asleep(void) {
    return state == STATE_ASLEEP || state == STATE_FADING_OUT;
}

bool idle_forced_sleep_active(void) {
    return forced_sleep_should_apply(millis());
}

void idle_tick(void) {
    uint32_t now = millis();
    bool forced_now = forced_sleep_should_apply(now);

    if (forced_now) {
        if (state == STATE_AWAKE || state == STATE_FADING_IN) {
            begin_fade(0, now);
            state = STATE_FADING_OUT;
        }
    }

    // While on USB power (if configured), don't sleep, and wake from sleep
    // when power comes back. Treats USB-in as continuous activity.
    if (!forced_now && !IDLE_SLEEP_WHEN_CHARGING && power_hal_is_vbus_in()) {
        last_activity_ms = now;
        if (state == STATE_ASLEEP || state == STATE_FADING_OUT) {
            begin_fade(DISPLAY_DEFAULT_BRIGHTNESS, now);
            state = STATE_FADING_IN;
        }
    }

    switch (state) {
    case STATE_AWAKE:
        if (!forced_now && now - last_activity_ms >= IDLE_TIMEOUT_MS) {
            begin_fade(0, now);
            state = STATE_FADING_OUT;
        }
        break;

    case STATE_FADING_OUT:
    case STATE_FADING_IN: {
        if (now - fade_last_step_ms < IDLE_FADE_STEP_MS) break;
        fade_last_step_ms = now;
        uint32_t dur = (state == STATE_FADING_OUT) ? IDLE_FADE_OUT_MS : IDLE_FADE_IN_MS;
        uint32_t elapsed = now - fade_started_ms;
        if (elapsed >= dur) {
            apply_brightness(fade_to);
            state = (state == STATE_FADING_OUT) ? STATE_ASLEEP : STATE_AWAKE;
        } else {
            // Linear interpolation fade_from -> fade_to over dur ms.
            int32_t span = (int32_t)fade_to - (int32_t)fade_from;
            int32_t b = (int32_t)fade_from + (span * (int32_t)elapsed) / (int32_t)dur;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            apply_brightness((uint8_t)b);
        }
        break;
    }

    case STATE_ASLEEP:
        break;
    }
}

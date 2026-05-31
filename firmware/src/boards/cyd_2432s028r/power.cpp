#include "../../hal/power_hal.h"
#include "board.h"
#include <Arduino.h>

#define BOOT_POLL_MS 50

static bool     pwr_pressed_flag = false;
static bool     last_boot_state  = false;
static uint32_t last_poll_ms     = 0;

void power_hal_init(void) {
    pinMode(BTN_BOOT_GPIO, INPUT_PULLUP);
}

void power_hal_tick(void) {
    uint32_t now = millis();
    if (now - last_poll_ms < BOOT_POLL_MS) return;
    last_poll_ms = now;

    bool boot_now = digitalRead(BTN_BOOT_GPIO) == LOW;
    if (boot_now && !last_boot_state) {
        pwr_pressed_flag = true;
    }
    last_boot_state = boot_now;
}

int  power_hal_battery_pct(void) { return -1; }
bool power_hal_is_charging(void) { return false; }
bool power_hal_is_vbus_in(void)  { return true; }

bool power_hal_pwr_pressed(void) {
    if (!pwr_pressed_flag) return false;
    pwr_pressed_flag = false;
    return true;
}

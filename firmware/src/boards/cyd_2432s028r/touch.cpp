#include "../../hal/touch_hal.h"
#include "board.h"
#include <Arduino.h>
#include <SPI.h>

static SPIClass touch_spi(VSPI);
static uint16_t touch_x = 0;
static uint16_t touch_y = 0;
static bool touch_pressed = false;

static uint16_t read12(uint8_t cmd) {
    touch_spi.transfer(cmd);
    uint16_t hi = touch_spi.transfer(0x00);
    uint16_t lo = touch_spi.transfer(0x00);
    return (uint16_t)(((hi << 8) | lo) >> 3);
}

static int map_clamped(uint16_t raw, int in_min, int in_max, int out_max) {
    if (in_max == in_min) return 0;
    long v = ((long)raw - in_min) * out_max / (in_max - in_min);
    if (v < 0) v = 0;
    if (v > out_max) v = out_max;
    return (int)v;
}

static bool read_raw(uint16_t* raw_x, uint16_t* raw_y) {
    if (digitalRead(TP_IRQ) == HIGH) return false;

    uint32_t sx = 0;
    uint32_t sy = 0;
    const uint8_t samples = 4;

    touch_spi.beginTransaction(SPISettings(2500000, MSBFIRST, SPI_MODE0));
    digitalWrite(TP_CS, LOW);
    delayMicroseconds(2);
    for (uint8_t i = 0; i < samples; i++) {
        sx += read12(0xD0);  // X position
        sy += read12(0x90);  // Y position
    }
    digitalWrite(TP_CS, HIGH);
    touch_spi.endTransaction();

    *raw_x = (uint16_t)(sx / samples);
    *raw_y = (uint16_t)(sy / samples);
    return true;
}

void touch_hal_init(void) {
    pinMode(TP_IRQ, INPUT);
    pinMode(TP_CS, OUTPUT);
    digitalWrite(TP_CS, HIGH);
    touch_spi.begin(TP_CLK, TP_MISO, TP_MOSI, TP_CS);
}

void touch_hal_read(uint16_t* x, uint16_t* y, bool* pressed) {
    uint16_t raw_x = 0;
    uint16_t raw_y = 0;
    if (read_raw(&raw_x, &raw_y)) {
        int px = map_clamped(raw_x, TOUCH_RAW_X_MIN, TOUCH_RAW_X_MAX, LCD_NATIVE_WIDTH - 1);
        int py = map_clamped(raw_y, TOUCH_RAW_Y_MIN, TOUCH_RAW_Y_MAX, LCD_NATIVE_HEIGHT - 1);

#if TOUCH_INVERT_X
        px = (LCD_NATIVE_WIDTH - 1) - px;
#endif
#if TOUCH_INVERT_Y
        py = (LCD_NATIVE_HEIGHT - 1) - py;
#endif
#if TOUCH_SWAP_XY
        int tmp = px;
        px = py;
        py = tmp;
#endif

        int tx = px;
        int ty = py;

#if TOUCH_ROTATION == 1
        tx = py;
        ty = (LCD_NATIVE_WIDTH - 1) - px;
#elif TOUCH_ROTATION == 2
        tx = (LCD_NATIVE_WIDTH - 1) - px;
        ty = (LCD_NATIVE_HEIGHT - 1) - py;
#elif TOUCH_ROTATION == 3
        tx = (LCD_NATIVE_HEIGHT - 1) - py;
        ty = px;
#endif

        if (tx < 0) tx = 0;
        if (ty < 0) ty = 0;
        if (tx >= LCD_WIDTH) tx = LCD_WIDTH - 1;
        if (ty >= LCD_HEIGHT) ty = LCD_HEIGHT - 1;

        touch_x = (uint16_t)tx;
        touch_y = (uint16_t)ty;
        touch_pressed = true;
    } else {
        touch_pressed = false;
    }

    *x = touch_x;
    *y = touch_y;
    *pressed = touch_pressed;
}

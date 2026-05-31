#include "../../hal/display_hal.h"
#include "board.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>

static Arduino_DataBus* bus = nullptr;
static Arduino_ILI9341* gfx = nullptr;

static void write_backlight(uint8_t level) {
    static bool pwm_attached = false;
    if (!pwm_attached) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcAttach(LCD_BL, 5000, 8);
#else
        ledcSetup(0, 5000, 8);
        ledcAttachPin(LCD_BL, 0);
#endif
        pwm_attached = true;
    }

    uint8_t duty = (LCD_BL_ON == HIGH) ? level : (uint8_t)(255 - level);
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWrite(LCD_BL, duty);
#else
    ledcWrite(0, duty);
#endif
}

void display_hal_init(void) {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCLK, LCD_MOSI, LCD_MISO);
    gfx = new Arduino_ILI9341(bus, LCD_RESET, LCD_ROTATION);
}

void display_hal_begin(void) {
    if (!gfx) return;
    gfx->begin();
    gfx->invertDisplay(LCD_INVERT_COLORS);
    gfx->fillScreen(0x0000);
    write_backlight(200);
}

void display_hal_set_brightness(uint8_t level) {
    write_backlight(level);
}

void display_hal_fill_screen(uint16_t color) {
    if (gfx) gfx->fillScreen(color);
}

void display_hal_draw_bitmap(int32_t x, int32_t y, int32_t w, int32_t h,
                             const uint16_t* pixels) {
    if (!gfx) return;

#if LCD_SWAP_RED_BLUE
    static uint16_t line_buf[LCD_WIDTH];
    for (int32_t row = 0; row < h; row++) {
        const uint16_t* src = pixels + (row * w);
        for (int32_t col = 0; col < w && col < LCD_WIDTH; col++) {
            uint16_t c = src[col];
            line_buf[col] = (uint16_t)((c & 0x07E0) |
                                       ((c & 0x001F) << 11) |
                                       ((c & 0xF800) >> 11));
        }
        gfx->draw16bitRGBBitmap(x, y + row, line_buf, w, 1);
    }
#else
    gfx->draw16bitRGBBitmap(x, y, (uint16_t*)pixels, w, h);
#endif
}

void display_hal_tick(void) {}

void display_hal_round_area(int32_t* x1, int32_t* y1, int32_t* x2, int32_t* y2) {
    (void)x1;
    (void)y1;
    (void)x2;
    (void)y2;
}

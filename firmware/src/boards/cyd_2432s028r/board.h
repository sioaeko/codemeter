#pragma once

// Sunton / "Cheap Yellow Display" ESP32-2432S028R.
// 2.8" 240x320 ILI9341 SPI TFT + XPT2046 resistive touch, no PSRAM.

#if defined(BOARD_CYD_2432S028R_LANDSCAPE)
#define BOARD_NAME           "CYD ESP32-2432S028R Landscape"
#define LCD_WIDTH            320
#define LCD_HEIGHT           240
#define LCD_ROTATION         1
#define TOUCH_ROTATION       1
#else
#define BOARD_NAME           "CYD ESP32-2432S028R Portrait"
#define LCD_WIDTH            240
#define LCD_HEIGHT           320
#define LCD_ROTATION         0
#define TOUCH_ROTATION       0
#endif

// Native panel coordinates before display rotation is applied.
#define LCD_NATIVE_WIDTH     240
#define LCD_NATIVE_HEIGHT    320

// Many CYD / ILI9341 variants need inversion for normal-looking colors.
// If a specific board revision looks worse, override this in platformio.ini.
#ifndef LCD_INVERT_COLORS
#define LCD_INVERT_COLORS    1
#endif

// Secondary fallback for panels with red/blue swapped.
#ifndef LCD_SWAP_RED_BLUE
#define LCD_SWAP_RED_BLUE    0
#endif

// ---- TFT SPI pins (ILI9341) ----
#define LCD_MISO             12
#define LCD_MOSI             13
#define LCD_SCLK             14
#define LCD_CS               15
#define LCD_DC               2
#define LCD_RESET            GFX_NOT_DEFINED
#define LCD_BL               21
#define LCD_BL_ON            HIGH

// ---- Touch SPI pins (XPT2046) ----
#define TP_IRQ               36
#define TP_MISO              39
#define TP_MOSI              32
#define TP_CLK               25
#define TP_CS                33

// Default CYD calibration. If touch is mirrored on a board revision, adjust
// these four values first; then flip TOUCH_INVERT_* if needed.
#define TOUCH_RAW_X_MIN      200
#define TOUCH_RAW_X_MAX      3700
#define TOUCH_RAW_Y_MIN      240
#define TOUCH_RAW_Y_MAX      3800

#if defined(BOARD_CYD_2432S028R_LANDSCAPE)
// After landscape rotation, the screen Y axis comes from native touch X.
#define TOUCH_INVERT_X       1
#define TOUCH_INVERT_Y       0
#else
#define TOUCH_INVERT_X       0
#define TOUCH_INVERT_Y       1
#endif
#define TOUCH_SWAP_XY        0

// ---- Buttons ----
// The CYD has BOOT and RESET only. BOOT is used as the screen-cycle button
// through power_hal; no HID shortcut buttons are exposed on this port.
#define BTN_BOOT_GPIO        0

// ---- Capability flags ----
#define BOARD_HAS_SECONDARY_BUTTON 0
#define BOARD_HAS_ROTATION         0
#define BOARD_HAS_IMU              0
#define BOARD_HAS_BATTERY          0
#define BOARD_HAS_IO_EXPANDER      0

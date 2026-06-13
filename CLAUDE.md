# Project Context

CodeMeter is ESP32 firmware for a desk-side Codex or Claude Code usage display. The current distribution keeps the generic usage/settings UI and removes bundled third-party mascot animations, official-looking brand strings, and proprietary font-derived firmware assets.

## Current Product Surface

- BLE device name: `CodeMeter`
- Supported CYD envs: `cyd_2432s028r`, `cyd_2432s028r_landscape`
- Main screens: Usage, Bluetooth, Settings, plus a startup status screen
- Settings pages:
  - Page 1: Display (`Used`/`Left`), Theme, Accent (`Warm`/`Green`), Bluetooth
  - Page 2: Night, Start, End
- Startup status UI is a generic abstract meter animation implemented in `firmware/src/splash.cpp`.
- UI fonts use LVGL built-in Montserrat sizes enabled in `firmware/platformio.ini`.

Compatibility references to Codex and Claude Code are descriptive only. Do not add official logos, mascots, proprietary fonts, copied product chrome, or official-looking manufacturer strings.

## Architecture

```text
firmware/src/
  hal/                      board-agnostic interfaces
  boards/<name>/            per-board display/touch/input/power/imu implementations
  main.cpp                  setup + loop, HAL calls only
  ui.{h,cpp}                usage/bluetooth/settings UI
  splash.{h,cpp}            generic startup status animation
  ble.{h,cpp}               NimBLE peripheral: custom data service + HID keyboard
  settings.{h,cpp}          NVS-backed display/theme/accent/night settings
  usage_rate.{h,cpp}        usage activity grouping for status accent
  data.h                    UsageData struct
  icons.h / icons_small.h   RGB565A8 icon arrays
```

Each board folder contains `board.h`, `board_init.cpp`, `display.cpp`, `touch.cpp`, `input.cpp`, `power.cpp`, `imu.cpp`, and `caps.cpp`. PlatformIO's `build_src_filter` includes shared code plus one board folder per env.

## CYD Hardware Notes

Default target is the common ESP32-2432S028R:

- TFT: ILI9341, SPI pins MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: XPT2046, SPI pins IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0 cycles screens

If colors or touch axes are wrong, start with `firmware/src/boards/cyd_2432s028r/board.h`.

## Build / Flash

```bash
pio run -d firmware -e cyd_2432s028r
pio run -d firmware -e cyd_2432s028r_landscape
./tools/update_web_flasher.sh
```

CLI flash helpers:

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

Local web flasher:

```bash
python3 -m http.server 8787 --directory web-flasher
```

## Daemon / Host Side

Daemons discover the BLE peripheral by name (`CodeMeter`) and write the same compact JSON payload to the RX characteristic:

```json
{ "s": 6, "sr": 285, "w": 1, "wr": 10065, "now": 765, "st": "allowed", "ok": true }
```

Run only one daemon per display at a time:

- `daemon/codex_usage_daemon.py`
- `daemon/claude_usage_daemon.py`
- `daemon/claude-usage-daemon.sh`

## GATT Characteristics

Custom service: `434f4445-582d-4d65-7465-720000000001`

- RX `...0002`: host writes usage JSON
- TX `...0003`: firmware notifies ack/nack
- REQ `...0004`: firmware requests first refresh after subscribe

## Preview Assets

Regenerate SVG previews with:

```bash
node tools/render_cyd_previews.js
```

The renderer produces generic SVG previews in both `screenshots/cyd_2432s028r/` and `web-flasher/previews/cyd_2432s028r/`.

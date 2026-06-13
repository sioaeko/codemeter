# CodeMeter Port Notes

This fork turns the CYD / `ESP32-2432S028R` board into a BLE usage display for Codex or Claude Code.

## What Changed

- Added PlatformIO envs `cyd_2432s028r` and `cyd_2432s028r_landscape`.
- Added a CYD HAL port for the ILI9341 SPI display and XPT2046 SPI touch controller.
- Renamed the BLE peripheral to `CodeMeter`.
- Kept the compact `240x320` and `320x240` usage/settings UI.
- Added on-device settings for `Used` vs `Left`, theme, accent, Bluetooth status, and Night Mode.
- Replaced third-party mascot/proprietary font splash assets with a generic startup status UI.
- Added `daemon/codex_usage_daemon.py`, which reads `~/.codex/auth.json` and sends usage over BLE.
- Kept the Claude Code daemon as a compatibility option using the same BLE payload.

Both daemons push the same compact payload shape:

```json
{ "s": 6, "sr": 285, "w": 1, "wr": 10065, "now": 765, "st": "allowed", "ok": true }
```

## Hardware Target

Default pinout is for the common ESP32-2432S028R:

- TFT: ILI9341, SPI pins MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: XPT2046, SPI pins IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0 cycles Usage/Bluetooth/Settings screens after startup

The Settings screen lets you switch the main meter between `Used` and `Left`, change theme/accent, open Bluetooth status, and configure Night Mode. The selected mode is stored in ESP32 NVS and survives reboot.

## Night Mode

This firmware does not start a Wi-Fi access point or web portal. Night Mode is configured on page 2 of the board's `Settings` screen. Use `NEXT` / `PREV` to switch between pages.

- `Night`: toggle the schedule on/off
- `Start`: advance the start time by one hour
- `End`: advance the end time by one hour

Night Mode turns the screen backlight fully off between the configured start and end times. Touching the screen wakes it temporarily. The schedule follows the local wall-clock minute sent by the desktop daemon in the `now` payload field, so no Wi-Fi or NTP setup is required.

## Flash

The easiest install path is the static ESP Web Tools page:

[https://sioaeko.github.io/codemeter/web-flasher/](https://sioaeko.github.io/codemeter/web-flasher/)

Test locally:

```bash
python3 -m http.server 8787 --directory web-flasher
```

To refresh bundled firmware after a new build:

```bash
./tools/update_web_flasher.sh
```

CLI:

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

## Daemons

Run either the Codex daemon or the Claude Code daemon. Running both at once makes them race to update the same `CodeMeter` display.

Codex:

```bash
codex login
./install-codex-mac.sh
```

Claude Code:

```bash
./install-mac.sh
```

## Distribution Cleanup

This port keeps the generic usage/settings behavior and removes bundled third-party mascot animations, proprietary font-derived firmware fonts, and official-looking manufacturer/device strings. Compatibility references to Codex and Claude Code are descriptive only; this project is not affiliated with OpenAI, Anthropic, or the upstream project.

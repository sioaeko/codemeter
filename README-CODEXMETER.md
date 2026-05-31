# Codexmeter CYD Port

This fork turns Clawdmeter into a Codex usage display for the Cheap Yellow
Display / ESP32-2432S028R board.

## What Changed

- Added PlatformIO env `cyd_2432s028r`.
- Added a CYD HAL port for the 240x320 ILI9341 SPI display and XPT2046 SPI
  touch controller.
- Renamed the BLE peripheral to `Codex Meter` and moved the custom service to
  Codexmeter UUIDs.
- Added a compact 240x320 UI layout.
- Added `daemon/codex_usage_daemon.py`, which reads `~/.codex/auth.json`,
  polls `https://chatgpt.com/backend-api/wham/usage`, and pushes:

```json
{ "s": 6, "sr": 285, "w": 1, "wr": 10065, "st": "allowed", "ok": true }
```

## Hardware Target

Default pinout is for the common ESP32-2432S028R:

- TFT: ILI9341, SPI pins MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: XPT2046, SPI pins IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Button: BOOT/GPIO0 cycles screens

There are CYD board revisions with different routing. If display or touch is
mirrored, start with `firmware/src/boards/cyd_2432s028r/board.h`.

## Flash

### Web Flasher

The easiest install path is the static ESP Web Tools page in `web-flasher/`.
Host that folder on GitHub Pages, or test it locally:

```bash
python3 -m http.server 8787 --directory web-flasher
```

Then open `http://localhost:8787/` in Chrome or Edge and click **Install**.

To refresh the bundled web-flasher firmware after a new build:

```bash
./tools/update_web_flasher.sh
```

### CLI

```bash
./flash-mac.sh cyd_2432s028r
```

Pass a serial device as the second argument if auto-detect picks the wrong port:

```bash
./flash-mac.sh cyd_2432s028r /dev/cu.usbserial-110
```

## Install The Codex Daemon On macOS

Sign in first:

```bash
codex login
```

Then install:

```bash
./install-codex-mac.sh
```

Useful logs:

```bash
tail -F ~/Library/Logs/codex-usage-daemon.out.log
```

## Notes

Codex usage fields come from the current Codex CLI backend-client behavior:
ChatGPT backend URLs normalize to `/backend-api`, and usage is fetched from
`/wham/usage`. The daemon includes the same OAuth refresh client id used by
Codex CLI so it can keep `~/.codex/auth.json` fresh.

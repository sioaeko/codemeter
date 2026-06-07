# Clawdmeter-CYD Port

This fork turns Clawdmeter into a Claude Code and Codex usage display for the
Cheap Yellow Display / ESP32-2432S028R board.

## What Changed

- Added PlatformIO env `cyd_2432s028r`.
- Added a CYD HAL port for the 240x320 ILI9341 SPI display and XPT2046 SPI
  touch controller.
- Kept the upstream-compatible BLE peripheral name `Claude Controller` and
  moved the custom service to Clawdmeter-CYD UUIDs.
- Added compact 240x320 portrait and 320x240 landscape UI layouts.
- Added on-device settings for `Used` vs remaining quota mode, theme, accent,
  and Night Mode.
- Added CYD LCD color correction hooks for inversion and red/blue swap.
- Added `daemon/codex_usage_daemon.py`, which reads `~/.codex/auth.json`,
  polls `https://chatgpt.com/backend-api/wham/usage`.
- Kept the Claude Code daemon path, updated to the same BLE
  service used by the CYD firmware.

Both daemons push the same compact payload shape:

```json
{ "s": 6, "sr": 285, "w": 1, "wr": 10065, "now": 765, "st": "allowed", "ok": true }
```

## Hardware Target

Default pinout is for the common ESP32-2432S028R:

- TFT: ILI9341, SPI pins MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, BL 21
- Touch: XPT2046, SPI pins IRQ 36, MISO 39, MOSI 32, CLK 25, CS 33
- Touch: tap the top-left Claude icon on Usage/Bluetooth to show Splash;
  tap Splash to return to Usage
- Button: BOOT/GPIO0 also cycles Usage/Bluetooth/Settings/Splash screens

The Settings screen lets you switch the main meter between `Used` and `Left`,
change theme/accent, and configure Night Mode. `Used` shows consumed quota,
while `Left` shows remaining available quota. The selected mode is stored in
ESP32 NVS and survives reboot.

## Night Mode

This firmware does not start a Wi-Fi access point or web portal. Night Mode is
configured on the board's `Settings` screen:

- `Night`: toggle the schedule on/off
- `Start`: advance the start time by one hour
- `End`: advance the end time by one hour

Night Mode turns the screen backlight fully off between the configured start
and end times. Touching the screen wakes it temporarily; the default wake
window is 5 minutes. The schedule follows the local wall-clock minute sent by
the desktop daemon in the `now` payload field, so no Wi-Fi or NTP setup is
required. The schedule is inactive until the board receives its first BLE
payload after boot.

There are CYD board revisions with different routing. If display or touch is
mirrored, start with `firmware/src/boards/cyd_2432s028r/board.h`.

## Flash

### Web Flasher

The easiest install path is the static ESP Web Tools page in `web-flasher/`.

**Live installer:** [https://sioaeko.github.io/clawdmeter-cyd/web-flasher/](https://sioaeko.github.io/clawdmeter-cyd/web-flasher/).

Test locally:

```bash
python3 -m http.server 8787 --directory web-flasher
```

Then open `http://localhost:8787/` in a browser with Web Serial support and
choose **Portrait** or **Landscape**.

To refresh the bundled web-flasher firmware after a new build:

```bash
./tools/update_web_flasher.sh
```

### CLI

```bash
./flash-mac.sh cyd_2432s028r
./flash-mac.sh cyd_2432s028r_landscape
```

Pass a serial device as the second argument if auto-detect picks the wrong port:

```bash
./flash-mac.sh cyd_2432s028r /dev/cu.usbserial-110
./flash-mac.sh cyd_2432s028r_landscape /dev/cu.usbserial-110
```

## Install A Daemon On macOS

Run either the Codex daemon or the Claude Code daemon. Running both at once
will make them race to update the same `Claude Controller` display.

### Codex

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

### Claude Code

Sign in to Claude Code first, then install:

```bash
./install-mac.sh
```

Useful logs:

```bash
tail -F ~/Library/Logs/claude-usage-daemon.out.log
```

## Notes

Codex usage fields come from the current Codex CLI backend-client behavior:
ChatGPT backend URLs normalize to `/backend-api`, and usage is fetched from
`/wham/usage`. The daemon includes the same OAuth refresh client id used by
Codex CLI so it can keep `~/.codex/auth.json` fresh.

# CodeMeter Installer

Static ESP Web Tools installer for the CYD / `ESP32-2432S028R` build.

## Local Test

```bash
python3 -m http.server 8787 --directory web-flasher
```

Then open:

```text
http://localhost:8787/
```

Published page:

```text
https://sioaeko.github.io/clawdmeter-cyd/web-flasher/
```

Use Chrome, Edge, or another browser with Web Serial support. The page offers both portrait (`240x320`) and landscape (`320x240`) firmware.

## Update Firmware

After rebuilding both CYD firmware variants, refresh the binaries used by the web flasher:

```bash
pio run -d firmware -e cyd_2432s028r
pio run -d firmware -e cyd_2432s028r_landscape
./tools/update_web_flasher.sh
```

The manifests flash these factory images at offset `0x0`:

```text
firmware/clawdmeter-cyd_2432s028r.factory.bin
firmware/clawdmeter-cyd_2432s028r_landscape.factory.bin
```

Each factory image is the combined PlatformIO output containing bootloader, partition table, boot app, and application firmware.

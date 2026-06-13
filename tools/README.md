# Tools

## Web Flasher Firmware Update

After rebuilding the CYD firmware variants, refresh the factory binaries served by the web flasher:

```bash
pio run -d firmware -e cyd_2432s028r
pio run -d firmware -e cyd_2432s028r_landscape
./tools/update_web_flasher.sh
```

The script copies PlatformIO factory images into:

```text
web-flasher/firmware/
```

## CYD Preview Rendering

Regenerate the SVG screenshots used by the README and web flasher:

```bash
node tools/render_cyd_previews.js
```

The preview renderer draws generic usage, settings, and Bluetooth screens. It does not depend on third-party mascot sprites or proprietary fonts.

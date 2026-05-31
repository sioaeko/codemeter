# Codexmeter Web Flasher

Static ESP Web Tools installer for the CYD / ESP32-2432S028R build.

The page UI follows the [Claude design system](DESIGN.md) from `npx getdesign add claude`
(Cormorant Garamond + Inter, cream canvas, coral CTAs, dark product mockups).

## Local Test

```bash
python3 -m http.server 8787 --directory web-flasher
```

Then open:

```text
http://localhost:8787/
```

**GitHub Pages (published site):**

```text
https://sioaeko.github.io/codexmeter-cyd/web-flasher/
```

GitHub Pages is published from the repo root. Open `/web-flasher/` for the
installer page.

Use Chrome, Edge, or another browser with Web Serial support.

## Update Firmware

After rebuilding the CYD firmware, refresh the binary used by the web flasher:

```bash
./tools/update_web_flasher.sh
```

The manifest flashes `firmware/codexmeter-cyd_2432s028r.factory.bin` at offset
`0x0`. That factory image is the combined PlatformIO output containing
bootloader, partition table, boot app, and application firmware.

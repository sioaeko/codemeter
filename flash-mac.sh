#!/bin/bash
# Build and flash CYD Usage Meter firmware on macOS.
# Usage:
#   ./flash-mac.sh <board>                       # auto-detect common USB serial ports
#   ./flash-mac.sh <board> /dev/cu.usbserial-110 # explicit USB serial port
#
# <board> is the PlatformIO env name, e.g. cyd_2432s028r or waveshare_amoled_18.
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOARD="${1:-}"
PORT="${2:-}"

if [ -z "$BOARD" ]; then
    echo "Error: board env name is required."
    echo "Usage: $0 <board> [port]"
    echo "Available boards:"
    grep -E '^\[env:' "$SCRIPT_DIR/firmware/platformio.ini" | sed 's/\[env:/  /;s/\]//'
    exit 1
fi

if [ -z "$PORT" ]; then
    for pattern in /dev/cu.usbserial* /dev/cu.wchusbserial* /dev/cu.SLAB_USBtoUART* /dev/cu.usbmodem*; do
        PORT=$(ls $pattern 2>/dev/null | head -1)
        if [ -n "$PORT" ]; then
            break
        fi
    done
    if [ -z "$PORT" ]; then
        echo "Error: no USB serial device found. Plug in the board or pass a port explicitly."
        exit 1
    fi
fi

if command -v pio >/dev/null; then
    PIO="pio"
elif [ -x "$SCRIPT_DIR/.pio-venv/bin/pio" ]; then
    PIO="$SCRIPT_DIR/.pio-venv/bin/pio"
else
    echo "PlatformIO not found; creating local .pio-venv ..."
    python3 -m venv "$SCRIPT_DIR/.pio-venv"
    "$SCRIPT_DIR/.pio-venv/bin/pip" install --quiet --upgrade pip
    "$SCRIPT_DIR/.pio-venv/bin/pip" install --quiet platformio
    PIO="$SCRIPT_DIR/.pio-venv/bin/pio"
fi

export PLATFORMIO_CORE_DIR="${PLATFORMIO_CORE_DIR:-$SCRIPT_DIR/.platformio}"

echo "=== Flashing CYD Usage Meter ==="
echo "Board: $BOARD"
echo "Port:  $PORT"
echo ""

cd "$SCRIPT_DIR/firmware"
"$PIO" run -e "$BOARD" -t upload --upload-port "$PORT"

echo ""
echo "=== Done ==="
echo "Monitor with: $PIO device monitor -p $PORT -b 115200"

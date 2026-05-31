#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
SRC="${1:-$ROOT/firmware/.pio/build/cyd_2432s028r/firmware.factory.bin}"
DST="$ROOT/web-flasher/firmware/codexmeter-cyd_2432s028r.factory.bin"

if [ ! -f "$SRC" ]; then
    echo "Missing firmware image: $SRC"
    echo "Build it first:"
    echo "  env PLATFORMIO_CORE_DIR=$ROOT/.platformio $ROOT/.pio-venv/bin/pio run -d firmware -e cyd_2432s028r"
    exit 1
fi

mkdir -p "$(dirname "$DST")"
cp "$SRC" "$DST"

SIZE=$(wc -c < "$DST" | tr -d ' ')
SHA=$(shasum -a 256 "$DST" | awk '{print $1}')

echo "Updated: $DST"
echo "Size:    $SIZE bytes"
echo "SHA256:  $SHA"

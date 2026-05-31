#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"

copy_image() {
    env_name="$1"
    dst_name="$2"
    src="$ROOT/firmware/.pio/build/$env_name/firmware.factory.bin"
    dst="$ROOT/web-flasher/firmware/$dst_name"

    if [ ! -f "$src" ]; then
        echo "Missing firmware image: $src"
        echo "Build it first:"
        echo "  env PLATFORMIO_CORE_DIR=$ROOT/.platformio $ROOT/.pio-venv/bin/pio run -d firmware -e $env_name"
        exit 1
    fi

    mkdir -p "$(dirname "$dst")"
    cp "$src" "$dst"

    size=$(wc -c < "$dst" | tr -d ' ')
    if command -v shasum >/dev/null 2>&1; then
        sha=$(shasum -a 256 "$dst" | awk '{print $1}')
    elif command -v sha256sum >/dev/null 2>&1; then
        sha=$(sha256sum "$dst" | awk '{print $1}')
    else
        sha="unavailable"
    fi

    echo "Updated: $dst"
    echo "Size:    $size bytes"
    echo "SHA256:  $sha"
}

if [ "$#" -eq 1 ]; then
    dst="$ROOT/web-flasher/firmware/clawdmeter-cyd_2432s028r.factory.bin"
    mkdir -p "$(dirname "$dst")"
    cp "$1" "$dst"
    echo "Updated: $dst"
else
    copy_image cyd_2432s028r clawdmeter-cyd_2432s028r.factory.bin
    copy_image cyd_2432s028r_landscape clawdmeter-cyd_2432s028r_landscape.factory.bin
fi

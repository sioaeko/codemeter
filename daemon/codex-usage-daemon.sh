#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
VENV="$SCRIPT_DIR/.venv"

if [ ! -x "$VENV/bin/python" ]; then
    python3 -m venv "$VENV"
    "$VENV/bin/pip" install --quiet --upgrade pip
    "$VENV/bin/pip" install --quiet "bleak>=0.22" "httpx>=0.27"
fi

exec "$VENV/bin/python" "$SCRIPT_DIR/codex_usage_daemon.py"

#!/bin/bash
# macOS installer for Codexmeter daemon (Python + bleak + launchd).
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVICE_LABEL="com.user.codex-usage-daemon"
PLIST_SRC="$SCRIPT_DIR/daemon/$SERVICE_LABEL.plist"
PLIST_DST="$HOME/Library/LaunchAgents/$SERVICE_LABEL.plist"
VENV_DIR="$SCRIPT_DIR/daemon/.venv"
DAEMON_PY="$SCRIPT_DIR/daemon/codex_usage_daemon.py"
APP_DIR="$SCRIPT_DIR/daemon/CodexmeterDaemon.app"
APP_EXEC="$APP_DIR/Contents/MacOS/CodexmeterDaemon"
LOG_DIR="$HOME/Library/Logs"
LOG_OUT="$LOG_DIR/codex-usage-daemon.out.log"
LOG_ERR="$LOG_DIR/codex-usage-daemon.err.log"

echo "=== Codexmeter macOS install ==="
echo ""

echo "[1/6] Checking prerequisites..."
for cmd in python3 curl; do
    command -v "$cmd" >/dev/null || { echo "Error: $cmd is required"; exit 1; }
done
if [ ! -f "$HOME/.codex/auth.json" ]; then
    echo "Warning: ~/.codex/auth.json not found."
    echo "  Run 'codex login' first, then re-run this installer."
    echo "  Continuing anyway; the daemon will retry on each poll."
fi
echo "  OK"
echo ""

echo "[2/6] Creating Python virtualenv at daemon/.venv ..."
if [ ! -d "$VENV_DIR" ]; then
    python3 -m venv "$VENV_DIR"
fi
"$VENV_DIR/bin/pip" install --quiet --upgrade pip
"$VENV_DIR/bin/pip" install --quiet "bleak>=0.22" "httpx>=0.27"
PYTHON_BIN="$VENV_DIR/bin/python"
SITE_PACKAGES="$("$PYTHON_BIN" -c 'import site; print(site.getsitepackages()[0])')"
PYTHON_HOME="$("$PYTHON_BIN" -c 'import sys; print(sys.base_prefix)')"
echo "  OK ($PYTHON_BIN)"
echo ""

echo "[3/6] Building macOS Bluetooth helper app..."
PY_REAL="$("$PYTHON_BIN" -c 'import os, sys; print(os.path.realpath(getattr(sys, "_base_executable", sys.executable)))')"
PY_ROOT="$(dirname "$(dirname "$PY_REAL")")"
PY_LIB="$PY_ROOT/Python3"
PY_APP="$PY_ROOT/Resources/Python.app"
if [ ! -f "$PY_REAL" ] || [ ! -f "$PY_LIB" ] || [ ! -d "$PY_APP" ]; then
    echo "Error: could not locate the Python framework executable for app bundling."
    echo "  Python: $PY_REAL"
    echo "  Library: $PY_LIB"
    echo "  App: $PY_APP"
    exit 1
fi

mkdir -p "$APP_DIR/Contents/MacOS" "$APP_DIR/Contents/Resources"
cp "$PY_REAL" "$APP_EXEC"
cp "$PY_LIB" "$APP_DIR/Contents/Python3"
rm -rf "$APP_DIR/Contents/Resources/Python.app"
cp -R "$PY_APP" "$APP_DIR/Contents/Resources/Python.app"
chmod +x "$APP_EXEC"
cat > "$APP_DIR/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>CodexmeterDaemon</string>
    <key>CFBundleIdentifier</key>
    <string>com.user.codexmeter.daemon</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Codexmeter Daemon</string>
    <key>CFBundleDisplayName</key>
    <string>Codexmeter Daemon</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSUIElement</key>
    <true/>
    <key>LSEnvironment</key>
    <dict>
        <key>PYTHONHOME</key>
        <string>$PYTHON_HOME</string>
        <key>PYTHONPATH</key>
        <string>$SITE_PACKAGES</string>
        <key>PYTHONUNBUFFERED</key>
        <string>1</string>
    </dict>
    <key>NSBluetoothAlwaysUsageDescription</key>
    <string>Codexmeter sends Codex usage percentages to your paired ESP32 display over Bluetooth.</string>
    <key>NSBluetoothPeripheralUsageDescription</key>
    <string>Codexmeter sends Codex usage percentages to your paired ESP32 display over Bluetooth.</string>
</dict>
</plist>
PLIST
cat > "$APP_DIR/Contents/Resources/Python.app/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>Python</string>
    <key>CFBundleIdentifier</key>
    <string>com.user.codexmeter.daemon.python</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Codexmeter Daemon</string>
    <key>CFBundleDisplayName</key>
    <string>Codexmeter Daemon</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSUIElement</key>
    <true/>
    <key>LSEnvironment</key>
    <dict>
        <key>PYTHONHOME</key>
        <string>$PYTHON_HOME</string>
        <key>PYTHONPATH</key>
        <string>$SITE_PACKAGES</string>
        <key>PYTHONUNBUFFERED</key>
        <string>1</string>
    </dict>
    <key>NSBluetoothAlwaysUsageDescription</key>
    <string>Codexmeter sends Codex usage percentages to your paired ESP32 display over Bluetooth.</string>
    <key>NSBluetoothPeripheralUsageDescription</key>
    <string>Codexmeter sends Codex usage percentages to your paired ESP32 display over Bluetooth.</string>
</dict>
</plist>
PLIST
if command -v codesign >/dev/null; then
    codesign --force --deep --sign - "$APP_DIR" >/dev/null 2>&1 || true
fi
echo "  OK ($APP_DIR)"
echo ""

echo "[4/6] Rendering launchd plist..."
mkdir -p "$HOME/Library/LaunchAgents" "$LOG_DIR"
sed \
    -e "s|__APP_EXEC__|${APP_EXEC}|g" \
    -e "s|__DAEMON_PATH__|${DAEMON_PY}|g" \
    -e "s|__REPO_DIR__|${SCRIPT_DIR}|g" \
    -e "s|__LOG_OUT__|${LOG_OUT}|g" \
    -e "s|__LOG_ERR__|${LOG_ERR}|g" \
    -e "s|__HOME__|${HOME}|g" \
    -e "s|__PYTHON_HOME__|${PYTHON_HOME}|g" \
    -e "s|__SITE_PACKAGES__|${SITE_PACKAGES}|g" \
    "$PLIST_SRC" > "$PLIST_DST"
echo "  Installed: $PLIST_DST"
echo ""

echo "[5/6] Bluetooth permission check..."
echo "  On first run the daemon may trigger a Bluetooth permission prompt."
echo "  If macOS asks, allow Bluetooth for 'Codexmeter Daemon'."
echo ""
read -r -p "Run a permission-priming scan now? [Y/n] " ans
if [[ ! "$ans" =~ ^[Nn]$ ]]; then
    PYTHONHOME="$PYTHON_HOME" PYTHONPATH="$SITE_PACKAGES" PYTHONUNBUFFERED=1 "$APP_EXEC" "$DAEMON_PY" --prime-scan || true
fi
echo ""

echo "[6/6] Loading launchd service..."
launchctl unload "$PLIST_DST" 2>/dev/null || true
launchctl load -w "$PLIST_DST"
echo "  Loaded."
echo ""

echo "=== Done ==="
echo ""
echo "First-time Bluetooth pairing after firmware is flashed:"
echo "  1. Power on the device."
echo "  2. Open System Settings > Bluetooth."
echo "  3. Click 'Connect' next to 'clawdmeter'."
echo "  4. The daemon will discover it within about 30 seconds."
echo ""
echo "Useful commands:"
echo "  launchctl list | grep codex-usage"
echo "  tail -F $LOG_OUT"
echo "  launchctl unload $PLIST_DST"
echo "  launchctl load -w $PLIST_DST"

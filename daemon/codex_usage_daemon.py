#!/usr/bin/env python3
"""Codex usage daemon for Codexmeter.

Reads Codex CLI ChatGPT credentials from ~/.codex/auth.json, polls the
ChatGPT backend usage endpoint, and writes a compact JSON payload to the
ESP32 over BLE.
"""

from __future__ import annotations

import asyncio
import base64
import json
import signal
import sys
import time
from pathlib import Path
from typing import Any

import httpx
from bleak import BleakClient, BleakScanner
from bleak.exc import BleakError

DEVICE_NAME = "Claude Controller"
SERVICE_UUID = "434f4445-582d-4d65-7465-720000000001"
RX_CHAR_UUID = "434f4445-582d-4d65-7465-720000000002"
REQ_CHAR_UUID = "434f4445-582d-4d65-7465-720000000004"

POLL_INTERVAL = 60
TICK = 5
SCAN_TIMEOUT = 8.0

AUTH_PATH = Path.home() / ".codex" / "auth.json"
SAVED_ADDR_FILE = Path.home() / ".config" / "codex-usage-monitor" / "ble-address"

USAGE_URL = "https://chatgpt.com/backend-api/wham/usage"
REFRESH_URL = "https://auth.openai.com/oauth/token"
CLIENT_ID = "app_EMoamEEZ73f0CkXaXp7hrann"


def log(msg: str) -> None:
    print(f"[{time.strftime('%H:%M:%S')}] {msg}", flush=True)


def read_auth() -> dict[str, Any] | None:
    try:
        return json.loads(AUTH_PATH.read_text())
    except OSError as e:
        log(f"Error reading {AUTH_PATH}: {e}")
    except json.JSONDecodeError as e:
        log(f"Error parsing {AUTH_PATH}: {e}")
    return None


def save_auth(auth: dict[str, Any]) -> None:
    tmp = AUTH_PATH.with_suffix(".json.tmp")
    tmp.write_text(json.dumps(auth, indent=2) + "\n")
    tmp.replace(AUTH_PATH)


def _jwt_payload(token: str) -> dict[str, Any] | None:
    parts = token.split(".")
    if len(parts) < 2:
        return None
    payload = parts[1] + "=" * (-len(parts[1]) % 4)
    try:
        return json.loads(base64.urlsafe_b64decode(payload.encode()))
    except Exception:
        return None


def token_expires_soon(token: str, skew_seconds: int = 300) -> bool:
    payload = _jwt_payload(token)
    exp = payload.get("exp") if payload else None
    if not isinstance(exp, (int, float)):
        return False
    return time.time() >= float(exp) - skew_seconds


async def refresh_token(auth: dict[str, Any]) -> bool:
    tokens = auth.get("tokens") or {}
    refresh = tokens.get("refresh_token")
    if not refresh:
        log("No Codex refresh token available; run `codex login` again")
        return False

    body = {
        "client_id": CLIENT_ID,
        "grant_type": "refresh_token",
        "refresh_token": refresh,
    }
    try:
        async with httpx.AsyncClient(timeout=20) as client:
            resp = await client.post(REFRESH_URL, json=body)
    except httpx.HTTPError as e:
        log(f"Token refresh failed: {e}")
        return False

    if resp.status_code >= 400:
        log(f"Token refresh failed: HTTP {resp.status_code}")
        return False

    data = resp.json()
    if data.get("access_token"):
        tokens["access_token"] = data["access_token"]
    if data.get("refresh_token"):
        tokens["refresh_token"] = data["refresh_token"]
    if data.get("id_token"):
        tokens["id_token"] = data["id_token"]
    auth["tokens"] = tokens
    auth["last_refresh"] = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    save_auth(auth)
    log("Refreshed Codex access token")
    return True


def reset_minutes(window: dict[str, Any] | None) -> int:
    if not window:
        return -1
    after = window.get("reset_after_seconds")
    if isinstance(after, (int, float)):
        return max(0, int((float(after) + 59) // 60))
    reset_at = window.get("reset_at")
    if isinstance(reset_at, (int, float)):
        return max(0, int(((float(reset_at) - time.time()) + 59) // 60))
    return -1


def used_percent(window: dict[str, Any] | None) -> float:
    if not window:
        return 0.0
    value = window.get("used_percent", 0)
    if isinstance(value, (int, float)):
        return max(0.0, min(100.0, float(value)))
    return 0.0


async def fetch_usage() -> dict[str, Any]:
    auth = read_auth()
    if not auth:
        return {"s": 0, "sr": -1, "w": 0, "wr": -1, "st": "auth", "ok": False}

    tokens = auth.get("tokens") or {}
    access = tokens.get("access_token")
    if not access:
        return {"s": 0, "sr": -1, "w": 0, "wr": -1, "st": "auth", "ok": False}

    if token_expires_soon(access):
        if await refresh_token(auth):
            auth = read_auth() or auth
            tokens = auth.get("tokens") or {}
            access = tokens.get("access_token", access)

    account_id = tokens.get("account_id")
    headers = {
        "Authorization": f"Bearer {access}",
        "User-Agent": "codexmeter",
    }
    if account_id:
        headers["ChatGPT-Account-Id"] = account_id

    async with httpx.AsyncClient(timeout=20) as client:
        resp = await client.get(USAGE_URL, headers=headers)
        if resp.status_code == 401 and await refresh_token(auth):
            auth = read_auth() or auth
            tokens = auth.get("tokens") or {}
            headers["Authorization"] = f"Bearer {tokens.get('access_token', access)}"
            if tokens.get("account_id"):
                headers["ChatGPT-Account-Id"] = tokens["account_id"]
            resp = await client.get(USAGE_URL, headers=headers)

    if resp.status_code >= 400:
        log(f"Usage poll failed: HTTP {resp.status_code}")
        return {"s": 0, "sr": -1, "w": 0, "wr": -1, "st": "error", "ok": False}

    data = resp.json()
    rate = data.get("rate_limit") or {}
    primary = rate.get("primary_window") or {}
    secondary = rate.get("secondary_window") or {}
    allowed = bool(rate.get("allowed", True)) and not bool(rate.get("limit_reached", False))

    return {
        "s": used_percent(primary),
        "sr": reset_minutes(primary),
        "w": used_percent(secondary),
        "wr": reset_minutes(secondary),
        "st": "allowed" if allowed else "limited",
        "ok": True,
    }


def load_cached_address() -> str | None:
    if not SAVED_ADDR_FILE.exists():
        return None
    addr = SAVED_ADDR_FILE.read_text().strip()
    if addr:
        return addr
    return None


def save_address(addr: str) -> None:
    SAVED_ADDR_FILE.parent.mkdir(parents=True, exist_ok=True)
    SAVED_ADDR_FILE.write_text(addr)


async def scan_for_device() -> str | None:
    log(f"Scanning for '{DEVICE_NAME}' ({SCAN_TIMEOUT}s)...")
    try:
        devices = await BleakScanner.discover(timeout=SCAN_TIMEOUT)
    except BleakError as e:
        log(f"Bluetooth scan unavailable: {e}")
        return None
    for device in devices:
        if device.name == DEVICE_NAME:
            log(f"Found: {device.address}")
            return device.address
    return None


_cb_manager = None


async def _get_cb_manager():
    global _cb_manager
    if _cb_manager is None:
        from bleak.backends.corebluetooth.CentralManagerDelegate import (
            CentralManagerDelegate,
        )

        manager = CentralManagerDelegate.alloc().init()
        _cb_manager = manager
    return _cb_manager


async def retrieve_connected_macos(skip_addr: str | None = None):
    from CoreBluetooth import CBUUID
    from bleak.backends.device import BLEDevice

    try:
        manager = await _get_cb_manager()
    except Exception as e:
        log(f"CoreBluetooth unavailable: {e}")
        return None

    cm = manager.central_manager

    def wrap(peripheral):
        addr = peripheral.identifier().UUIDString()
        log(f"Found system-connected peripheral: {peripheral.name()!r} [{addr}]")
        return BLEDevice(addr, peripheral.name(), (peripheral, manager))

    def ok(peripheral) -> bool:
        return not (skip_addr and peripheral.identifier().UUIDString() == skip_addr)

    custom = cm.retrieveConnectedPeripheralsWithServices_(
        [CBUUID.UUIDWithString_(SERVICE_UUID)]
    )
    for peripheral in custom or []:
        if ok(peripheral):
            return wrap(peripheral)

    hid = cm.retrieveConnectedPeripheralsWithServices_([CBUUID.UUIDWithString_("1812")])
    for peripheral in hid or []:
        if ok(peripheral) and peripheral.name() == DEVICE_NAME:
            return wrap(peripheral)

    return None


async def discover_target(skip_addr: str | None = None):
    if sys.platform == "darwin":
        dev = await retrieve_connected_macos(skip_addr=skip_addr)
        if dev is not None:
            return dev
        log(f"Not held by OS; scanning for '{DEVICE_NAME}' ({SCAN_TIMEOUT}s)...")
        try:
            devices = await BleakScanner.discover(timeout=SCAN_TIMEOUT)
        except BleakError as e:
            log(f"Bluetooth scan unavailable: {e}")
            return None
        for device in devices:
            if device.name == DEVICE_NAME:
                save_address(device.address)
                return device
        return None

    cached = load_cached_address()
    if cached and cached != skip_addr:
        return cached
    addr = await scan_for_device()
    if addr:
        save_address(addr)
    return addr


async def wait_or_stop(stop: asyncio.Event, timeout: float) -> None:
    try:
        await asyncio.wait_for(stop.wait(), timeout=timeout)
    except asyncio.TimeoutError:
        pass


async def push_loop(stop: asyncio.Event) -> None:
    skip_addr = None
    while not stop.is_set():
        target = await discover_target(skip_addr=skip_addr)
        if not target:
            log("Device not found; retrying")
            if not stop.is_set():
                await wait_or_stop(stop, 30)
            continue

        addr = getattr(target, "address", target)
        skip_addr = None
        try:
            log(f"Connecting to {addr}...")
            async with BleakClient(target, timeout=20) as client:
                log("Connected")
                refresh_now = asyncio.Event()

                def on_refresh(_sender, _data):
                    refresh_now.set()

                try:
                    await client.start_notify(REQ_CHAR_UUID, on_refresh)
                except Exception as e:
                    log(f"Refresh notify unavailable: {e}")

                while not stop.is_set() and client.is_connected:
                    payload = await fetch_usage()
                    raw = json.dumps(payload, separators=(",", ":")).encode()
                    await client.write_gatt_char(RX_CHAR_UUID, raw, response=False)
                    log(
                        "Sent usage: "
                        f"5h={payload['s']:.0f}% weekly={payload['w']:.0f}% "
                        f"status={payload['st']}"
                    )

                    refresh_now.clear()
                    deadline = time.monotonic() + POLL_INTERVAL
                    while not stop.is_set() and time.monotonic() < deadline:
                        timeout = min(TICK, deadline - time.monotonic())
                        try:
                            await asyncio.wait_for(refresh_now.wait(), timeout=timeout)
                            break
                        except asyncio.TimeoutError:
                            pass
        except (BleakError, OSError, asyncio.TimeoutError) as e:
            log(f"BLE error: {e}")
            skip_addr = addr if sys.platform == "darwin" else None
            if not stop.is_set():
                await wait_or_stop(stop, 10)


async def main() -> None:
    stop = asyncio.Event()
    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, stop.set)
        except NotImplementedError:
            pass
    await push_loop(stop)


async def prime_scan() -> None:
    target = await discover_target()
    if target:
        log(f"Bluetooth scan OK: {getattr(target, 'address', target)}")
    else:
        log(f"Bluetooth scan OK; {DEVICE_NAME} not found yet")


if __name__ == "__main__":
    if "--prime-scan" in sys.argv:
        asyncio.run(prime_scan())
    else:
        asyncio.run(main())

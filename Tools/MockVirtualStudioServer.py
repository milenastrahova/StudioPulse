#!/usr/bin/env python3
"""
Local mock backend for StudioPulse and VirtualStudioCore.

HTTP:
    http://127.0.0.1:8090/snapshot

WebSocket:
    ws://127.0.0.1:8091

The HTTP endpoint uses only Python's standard library.
The WebSocket endpoint requires:
    py -m pip install websockets
"""

from __future__ import annotations

import argparse
import asyncio
import json
import math
import threading
import time
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any


START_TIME = time.monotonic()
SEQUENCE = 0
SEQUENCE_LOCK = threading.Lock()


def build_payload() -> dict[str, Any]:
    global SEQUENCE

    with SEQUENCE_LOCK:
        SEQUENCE += 1
        sequence = SEQUENCE

    elapsed = max(0.0, time.monotonic() - START_TIME)
    alert_count = 1 if sequence % 9 == 0 else 0

    return {
        "operator_count": 8 + sequence % 5,
        "latency_ms": round(42.0 + 18.0 * math.sin(sequence * 0.35), 2),
        "uptime_seconds": round(elapsed, 2),
        "alert_count": alert_count,
        "status": "Warning" if alert_count else "Operational",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
    }


class SnapshotHandler(BaseHTTPRequestHandler):
    server_version = "VirtualStudioMock/1.0"

    def do_GET(self) -> None:
        if self.path != "/snapshot":
            self.send_error(404, "Use /snapshot")
            return

        body = json.dumps(build_payload()).encode("utf-8")

        self.send_response(200)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def log_message(self, format_string: str, *args: object) -> None:
        print(f"[HTTP] {self.address_string()} - {format_string % args}")


def run_http(host: str, port: int) -> None:
    server = ThreadingHTTPServer((host, port), SnapshotHandler)
    print(f"[HTTP] Snapshot endpoint: http://{host}:{port}/snapshot")
    server.serve_forever()


async def run_websocket(host: str, port: int, interval: float) -> None:
    try:
        import websockets
    except ImportError:
        print(
            "[WebSocket] Disabled. Install dependency with:\n"
            "py -m pip install websockets"
        )
        return

    async def handler(websocket: Any) -> None:
        print("[WebSocket] Client connected")

        try:
            while True:
                await websocket.send(json.dumps(build_payload()))
                await asyncio.sleep(interval)
        except websockets.ConnectionClosed:
            print("[WebSocket] Client disconnected")

    async with websockets.serve(handler, host, port):
        print(f"[WebSocket] Live endpoint: ws://{host}:{port}")
        await asyncio.Future()


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--http-port", type=int, default=8090)
    parser.add_argument("--ws-port", type=int, default=8091)
    parser.add_argument("--interval", type=float, default=1.0)
    args = parser.parse_args()

    http_thread = threading.Thread(
        target=run_http,
        args=(args.host, args.http_port),
        daemon=True,
    )
    http_thread.start()

    asyncio.run(
        run_websocket(
            args.host,
            args.ws_port,
            max(0.1, args.interval),
        )
    )


if __name__ == "__main__":
    main()

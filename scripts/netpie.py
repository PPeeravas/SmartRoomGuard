#!/usr/bin/env -S uv run --script
"""NETPIE test tool: fake the NodeMCU (publish) or fake the STM32 side (listen).

Setup:
    cp .env.example .env   # then fill in the values

Usage:
    uv run scripts/netpie.py publish --state ARMED --door 0 --dist 132
    uv run scripts/netpie.py demo [--delay 3]
    uv run scripts/netpie.py listen

All commands use the NODEMCU_* credentials from .env. Run one at a time:
NETPIE allows one connection per Client ID.
"""

import argparse
import json
import os
import sys
import threading
import time
from pathlib import Path

import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion

HOST = "mqtt.netpie.io"
PORT = 1883
SHADOW_TOPIC = "@shadow/data/update"
CMD_TOPIC_FILTER = "@msg/#"
STATES = ["DISARMED", "ARMING", "ARMED", "ALARM", "SILENCED"]

# (state, door, dist) walk through every state, door open/closed, and no-reading.
DEMO = [
    ("DISARMED", 0, 132),
    ("ARMING", 0, 132),
    ("ARMED", 0, 132),
    ("ALARM", 1, 40),
    ("SILENCED", 1, 40),
    ("DISARMED", 1, -1),
    ("DISARMED", 0, 130),
]


def load_env():
    """Minimal .env loader (KEY=VALUE per line); real env vars win."""
    path = Path(__file__).resolve().parent.parent / ".env"
    if not path.exists():
        return
    for line in path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, _, value = line.partition("=")
        os.environ.setdefault(key.strip(), value.strip().strip("\"'"))


def connect() -> mqtt.Client:
    names = ["NODEMCU_CLIENT_ID", "NODEMCU_TOKEN", "NODEMCU_SECRET"]
    missing = [n for n in names if not os.environ.get(n)]
    if missing:
        sys.exit(f"Missing in .env: {', '.join(missing)}")
    client_id, token, secret = (os.environ[n] for n in names)

    client = mqtt.Client(CallbackAPIVersion.VERSION2, client_id=client_id)
    client.username_pw_set(token, secret)
    connected = threading.Event()
    results = []

    def on_connect(_client, _userdata, _flags, reason_code, _properties):
        results.append(reason_code)
        connected.set()

    client.on_connect = on_connect
    client.connect(HOST, PORT, keepalive=30)
    client.loop_start()
    if not connected.wait(10):
        sys.exit(f"Timed out connecting to {HOST}:{PORT}")
    if results[0].is_failure:
        sys.exit(f"Connect refused: {results[0]} (check Client ID / Token / Secret)")
    return client


def disconnect(client: mqtt.Client):
    client.loop_stop()
    client.disconnect()


def publish_status(client: mqtt.Client, state: str, door: int, dist: int):
    payload = json.dumps({"data": {"state": state, "door": door, "dist": dist}})
    client.publish(SHADOW_TOPIC, payload, qos=1).wait_for_publish(10)
    print(f"-> {SHADOW_TOPIC} {payload}")


def cmd_publish(args):
    client = connect()
    publish_status(client, args.state, args.door, args.dist)
    disconnect(client)


def cmd_demo(args):
    client = connect()
    try:
        for state, door, dist in DEMO:
            publish_status(client, state, door, dist)
            time.sleep(args.delay)
    finally:
        disconnect(client)


def cmd_listen(_args):
    client = connect()

    def on_message(_client, _userdata, msg):
        print(f"<- {msg.topic} {msg.payload.decode(errors='replace')!r}")

    client.on_message = on_message
    client.subscribe(CMD_TOPIC_FILTER, qos=1)
    print(f"Listening on {CMD_TOPIC_FILTER} (Ctrl+C to stop). Press dashboard buttons now.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        disconnect(client)


def main():
    load_env()
    parser = argparse.ArgumentParser(description=(__doc__ or "").split("\n")[0])
    sub = parser.add_subparsers(required=True)

    p = sub.add_parser("publish", help="publish one status to the shadow")
    p.add_argument("--state", choices=STATES, default="ARMED")
    p.add_argument("--door", type=int, choices=[0, 1], default=0)
    p.add_argument("--dist", type=int, default=132, help="cm, -1 = no reading")
    p.set_defaults(func=cmd_publish)

    d = sub.add_parser("demo", help="cycle through all states")
    d.add_argument("--delay", type=float, default=3.0, help="seconds between updates")
    d.set_defaults(func=cmd_demo)

    l = sub.add_parser("listen", help="print everything arriving on @msg/#")
    l.set_defaults(func=cmd_listen)

    args = parser.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()

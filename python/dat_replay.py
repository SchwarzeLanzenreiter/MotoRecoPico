#!/usr/bin/env python3
"""Replay a MotoReco .dat log onto a CAN bus through a CANable v2.0.

A .dat file is a sequence of fixed-length 16-byte records (no header, little-endian):
    uint32 second / uint16 mirisecond / uint16 CAN ID (11bit) / uint8[8] data
The DLC is not recorded, so every frame is sent as 8 bytes.
0x7FF / 0x7FE / 0x7FD are virtual frames carrying GPS, so they are not sent by default
(use --include-gps to send them).

Pick the interface to match the firmware on the CANable v2.0:
    slcan (as shipped)  : --interface slcan  --channel COM5
    candleLight (gs_usb): --interface gs_usb --channel 0
"""

import argparse
import struct
import sys
import time

RECORD = struct.Struct("<IHH8s")
GPS_IDS = (0x7FF, 0x7FE, 0x7FD)


def read_records(path):
    """Yield (elapsed seconds as float, CAN ID, data bytes) in order."""
    with open(path, "rb") as f:
        raw = f.read()
    if len(raw) % RECORD.size:
        print(f"warning: the last {len(raw) % RECORD.size} bytes are an incomplete record, ignoring them",
              file=sys.stderr)
    for second, msec, can_id, data in RECORD.iter_unpack(raw[: len(raw) - len(raw) % RECORD.size]):
        yield second + msec / 1000.0, can_id & 0x7FF, data


def parse_ids(text):
    return {int(x, 16) for x in text.split(",") if x.strip()}


def open_bus(args):
    try:
        import can
    except ImportError:
        sys.exit("python-can not found: pip install -r requirements.txt")
    kwargs = {"interface": args.interface, "channel": args.channel, "bitrate": args.bitrate}
    if args.interface == "slcan":
        kwargs["tty_baudrate"] = 115200  # USB CDC, so the actual value is irrelevant
    try:
        return can, can.Bus(**kwargs)
    except Exception as e:  # noqa: BLE001 - show the reason the connection failed as-is
        sys.exit(f"cannot open the CAN interface ({args.interface} {args.channel}): {e}")


def main():
    p = argparse.ArgumentParser(description="Replay a MotoReco .dat onto a CAN bus through a CANable v2.0")
    p.add_argument("datfile", help="the .dat file to replay")
    p.add_argument("-i", "--interface", default="slcan",
                   help="python-can interface name (slcan / gs_usb / virtual etc. default: slcan)")
    p.add_argument("-c", "--channel", default=None,
                   help="the COM port for slcan (e.g. COM5), or the device number for gs_usb (e.g. 0)")
    p.add_argument("-b", "--bitrate", type=int, default=500000, help="CAN bitrate (default: 500000)")
    p.add_argument("-s", "--speed", type=float, default=1.0,
                   help="playback speed multiplier (2.0 for double speed, 0 to send as fast as possible "
                        "without waiting. default: 1.0)")
    p.add_argument("--start", type=float, default=0.0, help="start sending from this elapsed time in seconds")
    p.add_argument("--end", type=float, default=None, help="stop sending at this elapsed time in seconds")
    p.add_argument("--loop", action="store_true", help="start over from the beginning after the last frame")
    p.add_argument("--include-gps", action="store_true", help="also send the virtual GPS frames 0x7FF/0x7FE/0x7FD")
    p.add_argument("--only", type=parse_ids, default=None,
                   help="send only these IDs (hex, comma separated. e.g. 10c,130)")
    p.add_argument("--exclude", type=parse_ids, default=set(), help="IDs not to send (hex, comma separated)")
    p.add_argument("--dry-run", action="store_true", help="print the contents instead of sending to CAN")
    p.add_argument("-v", "--verbose", action="store_true", help="print one line per frame sent")
    args = p.parse_args()

    if args.speed < 0:
        p.error("--speed must be 0 or more")
    if not args.dry_run and args.channel is None and args.interface in ("slcan", "gs_usb"):
        p.error(f"--interface {args.interface} needs --channel "
                f"(e.g. {'COM5' if args.interface == 'slcan' else '0'})")

    def wanted(t, can_id):
        if t < args.start or (args.end is not None and t > args.end):
            return False
        if can_id in GPS_IDS and not args.include_gps:
            return False
        if args.only is not None and can_id not in args.only:
            return False
        return can_id not in args.exclude

    try:
        frames = [r for r in read_records(args.datfile) if wanted(r[0], r[1])]
    except OSError as e:
        sys.exit(f"cannot read {args.datfile}: {e}")
    if not frames:
        sys.exit("there are no frames to send (check the filter options)")
    print(f"{len(frames)} frames, {frames[0][0]:.3f}s - {frames[-1][0]:.3f}s "
          f"({frames[-1][0] - frames[0][0]:.1f} seconds' worth)", file=sys.stderr)

    if args.dry_run:
        can, bus = None, None
    else:
        can, bus = open_bus(args)
        print(f"connected to {args.interface} {args.channel} @ {args.bitrate} bps", file=sys.stderr)

    sent = errors = 0
    try:
        while True:
            t0_log = frames[0][0]
            t0_wall = time.perf_counter()
            for t, can_id, data in frames:
                if args.speed > 0:
                    delay = t0_wall + (t - t0_log) / args.speed - time.perf_counter()
                    if delay > 0:
                        time.sleep(delay)
                if args.dry_run or args.verbose:
                    print(f"{t:10.3f}  {can_id:03X}  {data.hex(' ').upper()}")
                if bus is not None:
                    msg = can.Message(arbitration_id=can_id, data=data, is_extended_id=False)
                    try:
                        bus.send(msg, timeout=0.1)
                        sent += 1
                    except can.CanError as e:
                        errors += 1
                        if errors <= 10 or args.verbose:
                            print(f"send failed at {t:.3f}s ID {can_id:03X}: {e}", file=sys.stderr)
            if not args.loop:
                break
            print("starting over from the beginning", file=sys.stderr)
    except KeyboardInterrupt:
        print("interrupted", file=sys.stderr)
    finally:
        if bus is not None:
            # closing an slcan channel makes the CANable drop frames it has not sent yet,
            # so wait for it to finish
            time.sleep(0.5)
            bus.shutdown()
            print(f"sent {sent} / failed {errors}", file=sys.stderr)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""MotoReco の .dat ログを CANable v2.0 経由で CAN バスへ再送信する。

.dat は 16 バイト固定長レコードの連続 (ヘッダ無し・リトルエンディアン):
    uint32 second / uint16 mirisecond / uint16 CAN ID (11bit) / uint8[8] data
DLC は記録されていないので常に 8 バイトで送る。
0x7FF / 0x7FE は GPS を載せた仮想フレームなので、既定では送らない (--include-gps で送る)。

CANable v2.0 のファームウェアに合わせてインタフェースを選ぶ:
    slcan (出荷時)      : --interface slcan  --channel COM5
    candleLight (gs_usb): --interface gs_usb --channel 0
"""

import argparse
import struct
import sys
import time

RECORD = struct.Struct("<IHH8s")
GPS_IDS = (0x7FF, 0x7FE)


def read_records(path):
    """(経過秒 float, CAN ID, data bytes) を順に返す。"""
    with open(path, "rb") as f:
        raw = f.read()
    if len(raw) % RECORD.size:
        print(f"警告: ファイル末尾の {len(raw) % RECORD.size} バイトは不完全なレコードなので無視します",
              file=sys.stderr)
    for second, msec, can_id, data in RECORD.iter_unpack(raw[: len(raw) - len(raw) % RECORD.size]):
        yield second + msec / 1000.0, can_id & 0x7FF, data


def parse_ids(text):
    return {int(x, 16) for x in text.split(",") if x.strip()}


def open_bus(args):
    try:
        import can
    except ImportError:
        sys.exit("python-can が見つかりません: pip install -r requirements.txt")
    kwargs = {"interface": args.interface, "channel": args.channel, "bitrate": args.bitrate}
    if args.interface == "slcan":
        kwargs["tty_baudrate"] = 115200  # USB CDC なので実際の値は無関係
    try:
        return can, can.Bus(**kwargs)
    except Exception as e:  # noqa: BLE001 - 接続失敗の理由をそのまま見せる
        sys.exit(f"CAN インタフェースを開けません ({args.interface} {args.channel}): {e}")


def main():
    p = argparse.ArgumentParser(description="MotoReco の .dat を CANable v2.0 で CAN バスへ再送信する")
    p.add_argument("datfile", help="再送信する .dat ファイル")
    p.add_argument("-i", "--interface", default="slcan",
                   help="python-can のインタフェース名 (slcan / gs_usb / virtual など。既定: slcan)")
    p.add_argument("-c", "--channel", default=None,
                   help="slcan なら COM ポート (例: COM5)、gs_usb ならデバイス番号 (例: 0)")
    p.add_argument("-b", "--bitrate", type=int, default=500000, help="CAN ビットレート (既定: 500000)")
    p.add_argument("-s", "--speed", type=float, default=1.0,
                   help="再生速度倍率 (2.0 で 2 倍速。0 で待たずに最速送信。既定: 1.0)")
    p.add_argument("--start", type=float, default=0.0, help="この経過秒から送信を始める")
    p.add_argument("--end", type=float, default=None, help="この経過秒で送信を終える")
    p.add_argument("--loop", action="store_true", help="最後まで送ったら先頭から繰り返す")
    p.add_argument("--include-gps", action="store_true", help="GPS 仮想フレーム 0x7FF/0x7FE も送信する")
    p.add_argument("--only", type=parse_ids, default=None, help="送る ID を限定する (16進、カンマ区切り。例: 10c,130)")
    p.add_argument("--exclude", type=parse_ids, default=set(), help="送らない ID (16進、カンマ区切り)")
    p.add_argument("--dry-run", action="store_true", help="CAN に送らず内容を表示するだけ")
    p.add_argument("-v", "--verbose", action="store_true", help="送信したフレームを 1 行ずつ表示する")
    args = p.parse_args()

    if args.speed < 0:
        p.error("--speed は 0 以上")
    if not args.dry_run and args.channel is None and args.interface in ("slcan", "gs_usb"):
        p.error(f"--interface {args.interface} には --channel が必要 (例: {'COM5' if args.interface == 'slcan' else '0'})")

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
        sys.exit(f"{args.datfile} を読めません: {e}")
    if not frames:
        sys.exit("送信するフレームがありません (フィルタ条件を確認してください)")
    print(f"{len(frames)} フレーム, {frames[0][0]:.3f}s - {frames[-1][0]:.3f}s "
          f"({frames[-1][0] - frames[0][0]:.1f} 秒分)", file=sys.stderr)

    if args.dry_run:
        can, bus = None, None
    else:
        can, bus = open_bus(args)
        print(f"{args.interface} {args.channel} @ {args.bitrate} bps に接続", file=sys.stderr)

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
                            print(f"送信失敗 {t:.3f}s ID {can_id:03X}: {e}", file=sys.stderr)
            if not args.loop:
                break
            print("先頭に戻ります", file=sys.stderr)
    except KeyboardInterrupt:
        print("中断しました", file=sys.stderr)
    finally:
        if bus is not None:
            # slcan はチャンネルを閉じると CANable 内でまだ送っていないフレームを捨てるので、送り切るまで待つ
            time.sleep(0.5)
            bus.shutdown()
            print(f"送信 {sent} / 失敗 {errors}", file=sys.stderr)


if __name__ == "__main__":
    main()

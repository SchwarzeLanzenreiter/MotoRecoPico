# MotoRecoPico

二輪車に積む CAN + GPS データロガー。**Raspberry Pi Pico 2 (RP2350)** を載せた自作基板で、
車両の CAN バスと GPS を microSD に記録する。記録したファイルは
[MotoRecoViewer](https://github.com/SchwarzeLanzenreiter/MotoRecoViewer) でそのまま開ける。

Raspberry Pi + HAT で動かしていた [MotoRecoLogger](https://github.com/SchwarzeLanzenreiter/MotoRecoLogger)
を、基板1枚に置き換えたもの。記録フォーマットは互換なので、過去のログも新しいログも同じツールで読める。

## リポジトリ構成

| ディレクトリ | 内容 |
|---|---|
| [`firmware/`](firmware/) | Pico 2 用ファームウェア（C / pico-sdk）。詳細は [firmware/README.md](firmware/README.md) |
| `cad/kicad/` | KiCad 10 プロジェクト、製造データ、生成・検査スクリプト |
| `cad/electronics/` | 旧 EAGLE 版（参考。現役は KiCad 側）|
| [`docs/`](docs/) | 設計ドキュメント。**[netlist.md](docs/netlist.md) が回路の正典** |

## ハードウェア

| 項目 | 内容 |
|---|---|
| 基板 | 4層、21 × 61 mm、部品 41点。リード部品は J1・Y1・Pico ヘッダのみ |
| MCU | Raspberry Pi Pico 2 (RP2350)。2×20 ソケットで交換可能。v4.2 で裏面実装 |
| CAN | Microchip MCP25625（コントローラ＋トランシーバ一体）を SPI1 で駆動 |
| GPS | YIC GT-502MGG-N。UART0 / 9600bps で NMEA を受ける |
| 保存 | microSD（Hirose DM3AT-SF-PEJM5）を SPI0 で駆動 |
| 車両I/F | TE MQS .63 8極。常時12V / IG / CAN High / CAN Low / GND の5本だけ使う |
| 電源 | 12V → AP63205 バックコンバータ → 5V。3.3V 系は Pico の 3V3(OUT) から |

**キーOFF 後も約 0.55 秒（実測）電源を保持する。** C13 が高側スイッチのゲート電荷を保つ仕組みで、
この猶予にファームウェアがログを閉じる。契約は [netlist.md](docs/netlist.md) §4.7。

設計の経緯（なぜ4層なのか、なぜこの配置なのか、各リビジョンで何を直したのか）は
[netlist.md](docs/netlist.md) と [hardware-design.md](docs/hardware-design.md) に残してある。
部品の購入先は [parts-sourcing.md](docs/parts-sourcing.md)。

## ファームウェア

- **単一ループ**で動く。割込みは CAN 受信と GPS UART の2つだけ
- CAN 受信は割込みでリングバッファへ退避。SD の書き込みが数百 ms 止まっても、
  コントローラの2段 FIFO を溢れさせない
- NMEA は自前でパース（Pi 版の gpsd の代替）。RTC が無いので GPS 時刻を単調時計に紐付ける
- FatFs R0.16 を同梱。SD の SPI ブロックドライバは自作
- ログの確定は **IG OFF 検出のみ**。車種依存を持ち込まないため、CAN 回転数は使わない
- タイムゾーンは microSD の隠しファイル `MOTORECO.CFG` で設定（再ビルド不要）
- ハードウェアに依存しない部分（NMEA・符号化・設定・IG 判定・時計）は
  **素の Pico 2 でも PC でもテストを実行できる**

ビルド方法・ログの読み方・トラブルシュートは [firmware/README.md](firmware/README.md) に。

## 記録フォーマット（`.dat`）

16バイト固定長レコードの連続。ヘッダも区切りも無く、すべてリトルエンディアン。

| オフセット | 型 | 内容 |
|---|---|---|
| 0 | `uint32` | ログ開始からの経過秒 |
| 4 | `uint16` | 同ミリ秒 |
| 6 | `uint16` | CAN ID（標準 11bit）|
| 8 | `uint8[8]` | ペイロード（DLC が 8 未満ならゼロ埋め）|

GPS は実在しない CAN ID に固定小数点で載せる。**`0x7FF`** が経度・緯度、
**`0x7FE`** が標高・速度。こうすると CAN と GPS が同じ1本の時系列に収まる。

> この形式は MotoRecoLogger（Pi 版）と MotoRecoViewer と**共有している契約**で、
> 変更すると3つのリポジトリすべてに影響する。

ファイル名は `YYYYMMDD_HHMMSS.dat`（キーOFF 時刻、ローカル時間）。GPS が測位するまでは
日付が分からないため `TMPnnnnn.DAT` で記録を始め、測位後に開始時刻の名前へ変える。

## 状態

- **実車で CAN・GPS ともに記録を確認済み**（2026-09）
- 基板は v4.2。実動作を確認したのは v4.1 の実基板（v4.2 は部品・ネット・GPIO 割当とも不変）
- 未確認: 記録した `.dat` を MotoRecoViewer で開く通しの確認

## 関連リポジトリ

- [MotoRecoLogger](https://github.com/SchwarzeLanzenreiter/MotoRecoLogger) — Raspberry Pi 版のロガーデーモン。本ファームウェアの移植元
- [MotoRecoViewer](https://github.com/SchwarzeLanzenreiter/MotoRecoViewer) — 記録した `.dat` を閲覧する Windows アプリ

## ライセンス

MIT License（[LICENSE](LICENSE)）。

同梱している ChaN FatFs は独自のライセンスに従う
（[firmware/third_party/fatfs/LICENSE.txt](firmware/third_party/fatfs/LICENSE.txt)）。

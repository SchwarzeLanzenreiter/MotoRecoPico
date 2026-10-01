# MotoRecoPico ファームウェア

Raspberry Pi 版ロガー（`MotoRecoLogger/mrlogger.c`）の Pico 2 (RP2350) 移植。
**`.dat` は Pi 版とバイナリ互換**で、MotoRecoViewer がそのまま開ける。

ピン割当・回路は [`../docs/netlist.md`](../docs/netlist.md) が正典。本書はソフト側だけを扱う。

## ビルド

pico-sdk と arm-none-eabi-gcc が必要。FatFs（R0.16）は
[`third_party/fatfs/`](third_party/fatfs/README.md) に同梱済み。

VS Code の Pico 拡張が入っている環境では、SDK もツールチェーンも `~/.pico-sdk` にある。
**`picotool_DIR` を渡すこと**（渡さないと SDK が picotool をソースから作ろうとし、
ホスト用 C コンパイラが無い環境では失敗する）:

```bash
cmake -B build -G Ninja -DPICO_BOARD=pico2 -Dpicotool_DIR=$HOME/.pico-sdk/picotool/2.2.0-a4/picotool
cmake --build build
```

`build/motorecopico.uf2` を BOOTSEL 起動した Pico にコピーする。

CAN バスなしでドライバを確かめたいときは `-DCAN_SELF_TEST=ON`。
MCP25625 をループバックにして、送信したフレームが戻るかを起動時に1回試す。

## テスト

ハードウェアに触らない部分（NMEA パーサ・レコード符号化・設定ファイル・IG 判定・壁時計）を検証する。
**実機（素の Pico 2 でよい。MotoRecoPico 基板は不要）でも PC でも走る。**

実機で走らせる場合 — `build/motorecopico_tests.uf2` を書き込み、USB シリアルを開く:

```
MotoRecoPico tests
checksum
GGA
...
<n> checks, 0 failures
ALL TESTS PASSED (0 failures).
```

**このテストはまだ一度も実行されていない**（開発機にホスト用 C コンパイラが無く、
Pico も接続されていないため）。コンパイルは両ビルドで通っている。最初に実機へ書き込んだ際、
失敗が出たらそれはテスト側の想定値の誤りかもしれない — 実装と突き合わせて判断すること。

実機で走らせる意味は結果だけではない。**`struct CANData` が arm で本当に16バイトに並ぶか**は
PC 上のテストでは確かめられない（`motoreco.h` の `static_assert` が実機ビルドで効く）。

PC で走らせる場合は `cd test && ./build.ps1`（MSVC）か `make`（gcc/clang）。

### 実機で確かめた数字

| 項目 | 実測 | 出所 |
|---|---|---|
| IG OFF 後の保持時間 | **約 0.55〜0.6秒** | 2026-09-04 の走行。`power still up 550ms` が最後の行で 600ms は書けなかった。§4.7 の「保証0.4秒・典型0.5秒」を裏付ける |
| 全 CAN のレコード数 | 約 1160/秒（約 18.5KB/秒）、33 ID | 実走ログ 1611秒の解析 |

シャットダウン処理（フラッシュ→sync→Close→リネーム）はこの 0.55秒に収める必要がある。
デバッグログ1行の書き込みは実測で数 ms 未満だった（生存ログの行がきっちり 50ms 刻みで並んだ）。

## 構成

| ファイル | 役割 |
|---|---|
| `src/main.c` | 初期化と単一ループ。CAN と GPS UART だけが割込み駆動 |
| `src/config.h` | ピン割当と全定数。設定ファイルの既定値もここ |
| `src/settings.c` | 設定ファイルのパース（純粋。PC でテスト可能）|
| `src/settings_file.c` | 同・FatFs 側（読み込み／既定値での生成／隠し属性）|
| `src/motoreco.h` | `struct CANData`。Pi 版と同一レイアウト（16バイト固定・LE）|
| `src/mcp25625.c` | CAN コントローラ。受信は割込みでリングバッファへ |
| `src/sd_spi.c` | SD の SPI ブロックドライバ |
| `src/diskio.c` / `src/ffconf.h` | FatFs との接続。`get_fattime()` は GPS 時刻を返す |
| `src/nmea.c` | GGA / RMC パーサ。gpsd の代わり |
| `src/wallclock.c` | GPS 時刻を単調時計に紐付けた壁時計 |
| `src/record.c` | `.dat` レコードの生成と GPS の固定小数点符号化 |
| `src/logger.c` | ファイルのライフサイクル、`f_sync`、デバッグログ |
| `src/led.c` | 緑LED の点滅パターン |
| `src/power.c` | IG OFF 判定の状態機械（純粋。PC でテスト可能）|

## Pi 版との違い

| | Raspberry Pi 版 | Pico 版 |
|---|---|---|
| CAN | SocketCAN (`can0`) | MCP25625 を SPI1 で直接。受信は割込み |
| GPS | gpsd | UART0 の NMEA を自前でパース |
| 保存 | ext4 上の `fopen`/`fwrite` | FatFs + 自作 SD SPI ドライバ |
| 時刻 | NTP 同期済みの system clock | GPS 時刻（RTC もバックアップも無い）|
| キーON/OFF | GPIO 27 を3回デバウンス | **GP28 の IG 検出**。0.55秒の猶予で確定する |
| 他プロセス連携 | System V 共有メモリ | 無し（Pico に相当物が無い）|
| デバッグログ | `canlogger.log` | `motoreco.log` ＋ USB シリアル |

## ログファイルの扱い

```
電源ON（＝IG ON）      TMPnnnnn.DAT を作って即記録開始
最初の GPS 測位        閉じて「開始時刻」の名前にリネームし、追記で開き直す
IG OFF（キーOFF）      フラッシュ → sync → Close → 「終了時刻」にリネーム → アイドル
異常断電               直前の f_sync までが確実に残る（最大1秒分の損失）
```

## ログの確定は IG OFF ただ1つ

**トリガは IG OFF だけ。** 以前は CAN の回転数低下でも確定していたが、CAN ID と
デコード式が車種ごとに違い、閾値も1台の実走データ由来だった。**イグニッション線は
どのバイクでも同じことを言う**ので、車種依存を持ち込まずに済むこちらへ一本化した。

デバッグログを `trigger:` で grep すれば確定の記録が出る:

```
[20260904 154302.233] trigger: ignition off (65mV). log closed and renamed as '20260904_154302.dat'
```

結果は成否だけでなく**何が起きたか**を出す（`logger_finalize_text()`）:

| 結果 | 意味 |
|---|---|
| `log closed and renamed as '...'` | 同期してリネームまで完了 |
| `log synced but not renamed, no gps time yet` | データは安全。GPS 未測位で名前が付けられない |
| `no log was open` | カード無し等 |
| `the card refused` | 書き込み失敗 |

**キーONのままエンジンが止まってもログは閉じない**（信号待ちのエンスト、休憩でエンジンだけ
切る等）。データ保全は毎秒の `f_sync` が担保し、確定は IG OFF で行われる。
回転数そのものは、これまでどおり CAN フレームとして `.dat` に記録される。

## IG OFF 検出（GP28）

**基板は IGN OFF 後 0.4秒（typ 0.5秒）だけ生き続ける**（C13 が Q2 のゲート電荷を保持し、
R1 経由でゆっくり放電するため）。この猶予でログを確定させるのがファームウェアの責務で、
契約は [`docs/netlist.md`](../docs/netlist.md) §4.7 に定義されている。

| 契約 | 実装 |
|---|---|
| GP28 を 50ms 以下の周期でポーリング | **10ms 周期のタイマ割込み**（`IG_POLL_INTERVAL_MS`）|
| 0.35V を下回ったら フラッシュ→Close→リネーム→アイドル | `logger_shutdown()`。**sync を最優先**し、リネームは後 |
| 猶予は保証 0.4秒（SD ストール最悪 250ms 込み）| 検出に使うのは 20ms（`IG_OFF_SAMPLES` 2回一致）|

**メインループではなくタイマ割込みで読む理由**は、SD の書き込みが数百 ms メインループを
止めることがあるためです。そこで検出まで遅れると猶予を食い潰します。
割込みは検出だけを行い、実際のファイル操作はメインループが担当します（FatFs は再入不可）。

### GP28 は電圧計ではない（v3 での変更）

**この基板では電池電圧は測れません。** v3 で R15(10k) が Q1 のベースノードに接続され、
そのノードは Q1 のベース・エミッタ接合によって**約0.65V にクランプ**されます。
つまり読めるのは **IG ON（≈650mV）/ OFF（0V）だけ**です。
0.65V はデジタル入力の VIH（約2V）未満なので、**ADC で読む必要があります**。

| 状態 | GP28 |
|---|---|
| IG ON | 約 650mV |
| IG OFF | 0mV |
| 閾値 | `IG_OFF_MV` 350mV / 復帰 `IG_ON_MV` 500mV |

status 行の `ig:` はこの**生のピン電圧**で、電池電圧ではありません:

```
status: rx:412033 records:398211 ig:651mV(on) | gps fix:1 ...
```

### 保持時間の実測（診断用・既定 OFF）

IG OFF 後、電源が尽きるまで 50ms ごとにログを書き続ける機能がある。
**カードに残った最後の行が実測値**で、2026-09-04 の走行で測った結果が以下:

```
[20260904 154302.233] trigger: ignition off (65mV). log closed and renamed as '20260904_154302.dat'
[20260904 154302.264] power still up 50ms after ignition off
[20260904 154302.314] power still up 100ms after ignition off
...
[20260904 154302.764] power still up 550ms after ignition off   ← ここで途切れた
```

**保持時間は約 0.55〜0.6秒**で、§4.7 の「保証0.4秒」に対し余裕がある。測定は済んだので
`config.h` の `IG_OFF_SURVIVAL_LOG` は **既定 0**。C13 や負荷を変えて測り直したくなったら
1 に戻す。電源が落ちかけている最中にデバッグログへ書き続けるため、その1ファイルが
chkdsk を要する形で壊れる可能性がある（**トリップログはこの時点で既に閉じて同期済み**）。

### 動作

- **一度 IG ON を観測するまで発火しない**（`armed`）。USB 給電のベンチで GP28 が 0V のまま
  でも誤ってシャットダウンしない
- **IG が戻ったら新しいログを開く**（USB 給電中にキーを戻した場合など）。閉じたファイルは
  閉じたまま、次のトリップは別ファイルになる
- アイドル中は **LED 消灯**（`LED_IDLE`）。「カード異常＝高速点滅」と紛らわしくないように
- アイドル中は 5秒ごとのカード再マウントも止める（せっかく閉じたログを開き直さないため）

## 設定ファイル `MOTORECO.CFG`

SDカードのルートに置く**隠しファイル**。無ければ起動時に既定値で自動生成する。
`#` で始まる行はコメント。テキストエディタで編集できる。

```ini
# MotoRecoPico settings
#
# Lines starting with '#' are comments. Edit with any text editor.
# This file is hidden; enable "show hidden files" to see it.
#
# timezone: offset from UTC, used for the .dat file names and the debug log.
#           whole hours (+8, -5) or with minutes (+5:30).
timezone = +9
```

| キー | 内容 | 既定 |
|---|---|---|
| `timezone` | UTC からのオフセット。`+9` / `-5` / `+5:30` / `+5.5` を受け付ける。`.dat` のファイル名とデバッグログの時刻に効く | `+9`（JST）|

- `key = value` でも `key value` でも通る。キーは**大文字小文字を区別しない**
- 読めない行は**その行だけ**無視してデバッグログに残す。ファイル全体を捨てたりはしない
- 正常時のログ: `settings: timezone +09:00`
- カードを差し替えると、その**カードの設定ファイルが読み直される**（マウントのたびに読む）

### 廃止された `vehicle` キー

以前はエンジン回転数のデコード規則（車種）を選ぶ `vehicle` キーがあった。
回転数によるログ確定をやめたので不要になっている。**古いファームで書かれたカードには
この行が残っている**ため、パーサは `vehicle` を「廃止済み」として黙って読み飛ばす。
手で消さなくても、起動のたびに警告が出ることはない。

## ピン割当（基板 v4.1）

正典は [`docs/netlist.md`](../docs/netlist.md) 3.7節。`src/config.h` がこれに追従する。

| 機能 | GPIO | 備考 |
|---|---|---|
| microSD | GP2/3/4/5 | **SPI0** SCK/MOSI/MISO/CS |
| SD カード検出 | GP7 | 外部プルアップ R13 |
| GPS UART0 | GP0（TX）/ GP1（RX）| GP1 にモジュールの TXD（オレンジ）|
| GPS PPS | GP6 | 未使用（入力のまま）|
| LED 緑 | GP8 | Low で点灯 |
| CAN MCP25625 | GP10/11/12/13 | **SPI1** SCK/MOSI/MISO/CS |
| CAN INT / STBY / RESET | GP9 / GP20 / GP21 | |
| IG 検出 | GP28 (ADC2) | 回転で変わらず。**ON/OFF のみ**で電池電圧は測れない |

**GP14〜GP19・GP22・GP26・GP27 は基板上で GND に落ちている。絶対に出力にしないこと。**

v4 からの差分は Pico の 180° 回転によるもので、**SPI インスタンスも入れ替わっている**
（SD が SPI1→**SPI0**、CAN が SPI0→**SPI1**）。ピンだけ直して SPI を直し忘れる事故を防ぐため、
`sd_spi.c` と `mcp25625.c` に「そのピンが本当にそのインスタンスの SCK/TX/RX か」を
`_Static_assert` で検査してある（RP2350 は 8ピンごとにインスタンス、4ピンごとに機能が巡回する）。

**stdio を UART に出してはいけない。** GP0/GP1 は SDK の stdio UART の既定ピンでもあるため、
有効にすると printf が GPS モジュールの受信端子に流れ込む。デバッグ出力は USB CDC のみ。

## LED（緑・GP8、Low で点灯）

| 表示 | 意味 |
|---|---|
| 点灯 | 記録中 |
| 1Hz 点滅 | 記録中だが GPS 未測位 |
| 高速点滅 | カードが使えない（応答なし／マウント失敗／書き込みエラー）。5秒ごとに再試行中 |
| 消灯 | IG OFF を検出してログを閉じた後（通常は直後に電源が落ちる）|

## GPS の状態ログ

測位に時間がかかるとき、原因は「配線・ボーレートが違って一切受信していない」「アンテナが空を見ていない」
「単にコールドスタート中」のどれかで、**どれも「未測位」としか見えない**。区別できるよう、
受信機自身の見え方をログに出す。

未測位の間は `GPS_REPORT_INTERVAL_MS`（既定10秒）ごとに:

```
gps searching after 30s: quality:0 mode:1D sats used:0 view:9 tracked:4 best snr:28 hdop:0.0 sentences:301 bad:0
```

読み方:

| 症状 | 出力 | 意味 |
|---|---|---|
| 何も受信していない | `gps silent: nothing received on uart0 in 30s...` | 配線・電源。モジュールは測位前から喋るので、無音は受信の問題ではない |
| 受信しているが壊れている | `sentences:0 bad:250` が増え続ける | ボーレート違い（文字化けでチェックサム不一致）|
| `view:0 tracked:0` のまま | 衛星が1個も見えない | アンテナ面が空を向いていない、金属ケース内、モジュール不良 |
| `view:9 tracked:1 best snr:15` | 見えてはいるが信号が弱い | 遮蔽（ビル・屋内・タンク下）。SNR 30以上が数個ないと測位しない |
| `view:9 tracked:5 snr:30+` で待ち | 正常な取得中 | あと数十秒待てば入る（下記のコールドスタート）|

測位した瞬間と失った瞬間にも1行ずつ出る（`gps fixed after 42s: ...` / `gps fix lost after ...`）。
最初の1文を受信した時点でも `gps talking: first sentence '$GNGGA' at 9600 baud` を出すので、
**配線とボーレートが合っているかは起動直後に確定できる**。

### 実機で確定した配線（2026-08-14）

**ボーレートは 9600bps**（`GPS_UART_BAUD` の既定値どおり）。線色と J3 パッドの対応:

| J3 パッド | シルク | 繋ぐ線 | 色 |
|---|---|---|---|
| 1 | V | VCC | 赤 |
| 2 | G | GND | 黒 |
| 3 | T | **RXD** | **緑** |
| 4 | R | **TXD** | **オレンジ** |
| 5 | P | PPS | 茶 |

**シルクの T / R は基板側から見た名前で、モジュールのピン名とは逆になる。**
UART は交差するので、`T`（基板が送信）にはモジュールの **RXD（緑）** が付く。
「TXD（オレンジ）を T に」と読んで配線すると出力どうしが衝突し、1バイトも受信できない
（実際にこれが起きた）。正典は [`docs/netlist.md`](../docs/netlist.md) 3.5節。

### 測位が遅いこと自体の原因

**J3 はバックアップ電源を持たない**（`docs/netlist.md` の J3 は VCC / GND / TXD / RXD / PPS の5線のみ）。
つまり**キーOFF のたびに軌道暦を失い、毎回コールドスタートになる**。
一般的な受信機のコールドスタートは晴天・見晴らしの良い場所で 30〜45秒、
条件が悪ければ数分かかる。ホットスタートなら1〜2秒で入るので、この差は大きい。

改善するならハードウェア側の変更になる:
- GT-502MGG-N にバックアップ端子があるなら、常時12V系からそこへ給電する（データシート 2.1節を要確認）
- あるいは J3.VCC を常時電源にする（受信機が待機電流を食い続けるためバッテリー上がりとの兼ね合い）

**現状のログはこの判断材料を出すためのもの**で、`gps fixed after Ns` の N が毎回30〜45秒なら
コールドスタートそのもの、毎回数分なら遮蔽かアンテナの問題、という切り分けができる。

## カード検出（GP7）の扱い

**カード検出ピン（GP7）は判断材料であって判断者ではない。** カードの有無は「SPI で応答するか」で決める
（[`sd_spi.c`](src/sd_spi.c) の `sd_init()` は検出線を見ずに必ず試す）。理由は3つ:

- 検出スイッチはソケットで**最も小さいパッド2枚（9番と10番）**に載っており、はんだ不良の確率が高い
- 接点が「挿入で閉」なのか「挿入で開」なのかは **DM3AT の図面に明記がない**（`SD_CD_INSERTED_LEVEL` は仮定）
- 二輪の振動でチャタれば、正常なカードを切り離してしまう

`SD_CD_INSERTED_LEVEL` を間違えていても**記録は普通に動く**。失うのは「走行中の抜去を早期に検出する」機能だけで、
それも実際に抜かれれば次の書き込み失敗で検出される。

走行中の抜去チェックは、**一度でも「検出線が挿入と言っている状態で実際にカードが応答した」場合にのみ**有効になる
（`sd_detect_reliable()`）。食い違ったまま動いた場合は起動時に一度だけ警告を出す:

```
note: card detect (GP7) reads empty with a working card. ignoring it.
      check pad 9/10 solder, or flip SD_CD_INSERTED_LEVEL
```

## データ量の目安（同じ実走ログより）

全 CAN で **約 1160 レコード/秒（約 18.5 KB/秒）**、33 種類の ID。
27分の走行で 30MB。受信リング 4096 レコードはこのレートで **約 3.5 秒分**にあたり、
SD カードの書き込みが数百 ms 詰まっても溢れない。

## 実機で最初に確認すること

1. USB シリアルに `MotoRecoPico starting` → `logging to '...'` → `can controller ready` が出るか
2. CAN が1フレームも来ない場合: `config.h` の `CAN_BITRATE`（既定 500kbps）と、
   終端抵抗 R8（`docs/netlist.md` の「CAN 終端は常時有効」）を疑う
3. カードが使えない場合、USB シリアルのメッセージで切り分ける:
   - `no card: the slot reads empty and nothing answered on the card spi` → SPI 配線（MISO/MOSI/CS）、
     カードの挿し込み不足、3V3 の供給
   - `a card is in the slot but no log file could be opened...` → **まず exFAT を疑う**。
     32GB超のカードは工場出荷時 exFAT で、`FF_FS_EXFAT 0` のため弾かれる。FAT32 で再フォーマット
   - `note: card detect (GP7) reads empty with a working card` → 記録は動いている。
     パッド 9/10 のはんだと `SD_CD_INSERTED_LEVEL` の極性を後で確認すればよい
4. GPS のボーレート（`GPS_UART_BAUD`、既定 9600）
5. キーOFF で `trigger: ignition off (...)` が出て、ファイル名が終了時刻になっているか
6. 記録した `.dat` を MotoRecoViewer で開き、Pi 版のログと同じように描けるか

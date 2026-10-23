# MotoRecoPico firmware

A port of the Raspberry Pi logger (`MotoRecoLogger/mrlogger.c`) to the Pico 2 (RP2350).
**The `.dat` files are binary-compatible with the Pi version**, so MotoRecoViewer opens them as-is.

Pin assignment and the circuit are defined canonically in
[`../docs/netlist.md`](../docs/netlist.md). This document covers only the software side.

## Build

You need pico-sdk and arm-none-eabi-gcc. FatFs (R0.16) is already bundled in
[`third_party/fatfs/`](third_party/fatfs/README.md).

On a machine with the VS Code Pico extension installed, both the SDK and the toolchain live in
`~/.pico-sdk`. **Pass `picotool_DIR`** — without it the SDK tries to build picotool from source,
which fails on a machine with no host C compiler:

```bash
cmake -B build -G Ninja -DPICO_BOARD=pico2 -Dpicotool_DIR=$HOME/.pico-sdk/picotool/2.2.0-a4/picotool
cmake --build build
```

Copy `build/motorecopico.uf2` onto a Pico booted in BOOTSEL mode.

To exercise the driver without a CAN bus, build with `-DCAN_SELF_TEST=ON`. It puts the MCP25625
into loopback and checks once at startup that a transmitted frame comes back.

The controller otherwise runs in **listen-only** mode and never drives the bus, not even with an
ACK. On the bike the ECUs acknowledge each other. On a bench with only a CANable and the Pico,
nobody acknowledges, so `python/dat_replay.py` switches the CANable's automatic retransmission off
(`A0`). The CANable then counts an ACK error per frame; for up to the first 16 frames, while it is
still error-active, its error flag corrupts each frame and the Pico drops it. From then on it is
error-passive, its error flag is recessive, and the Pico receives the frames.

One more catch with `A0`: a frame that fails for want of an ACK takes the frames waiting behind it
in the CANable's 3-deep transmit FIFO down with it. Replaying back-to-back frames lost 49%
(`20261023_200818.dat`: only the first frame of each same-millisecond burst arrived, and the loss
repeated every four frames), so `dat_replay.py` hands frames over at least `--min-gap` (0.5 ms)
apart.

## Tests

The tests cover everything that does not touch hardware: the NMEA parser, record encoding, the
settings file, the IG decision, and the wall clock. **They run on real hardware (a bare Pico 2 is
enough — the MotoRecoPico board is not needed) and on a PC.**

To run on hardware, flash `build/motorecopico_tests.uf2` and open the USB serial port:

```
MotoRecoPico tests
checksum
GGA
...
<n> checks, 0 failures
ALL TESTS PASSED (0 failures).
```

**These tests have never actually been run** (the development machine has no host C compiler and no
Pico attached). Both builds do compile. The first time they are flashed to hardware, a failure may
well be a wrong expected value in the test rather than a bug — check it against the implementation
before concluding anything.

Running on hardware matters for more than the result. **Whether `struct CANData` really lays out as
16 bytes on arm** cannot be checked by a PC test (the `static_assert` in `motoreco.h` only takes
effect in the hardware build).

To run on a PC: `cd test && ./build.ps1` (MSVC) or `make` (gcc/clang).

### Numbers measured on real hardware

| Item | Measured | Source |
|---|---|---|
| Hold-up time after IG OFF | **about 0.55–0.6 s** | Ride of 2026-09-04. `power still up 550ms` was the last line written; 600 ms never made it. Confirms the "0.4 s guaranteed, 0.5 s typical" of §4.7 |
| Record rate, all CAN | about 1160/s (about 18.5 KB/s), 33 IDs | Analysis of a 1611-second real-world log |
| One trip, measured | **8.1 M records / 123.7 MB / 1 h 55 min / 1177 rec/s** | 2026-10-17. 146 km ridden. 31 CAN IDs |
| Actual GPS rate (while riding) | **9.92 Hz, 99.7% of all intervals at 100 ms** | Same trip. While parked, fixes are thinned out by the duplicate check; while riding every fix is recorded |
| Time going backwards | **0 out of 8.1 M** | Same trip. `clamped:0`. The time-ordered merge is doing its job |
| Missing records | **largest gap 7 ms, zero gaps over 500 ms** | Same trip. `can rx overflow` was also 0 for the whole day |
| Main loop stalls | up to **1285 ms** (waiting on the SD card) | Same day. Measured GPS ring peak 3536/8192. The old 2048 overflowed (`uart_lost:455`) |
| Records out of time order | **4 out of 27,846, at most 2 ms** | 2026-10-13, GPS at 10 Hz plus CAN replay. Zero at 1 Hz. Resolved by the clamp described below |
| Actual GPS throughput at 10 Hz | **1440 B/s** (20 sentences/s, 72 B each) = 12% of 115200 | 2026-10-13. `bad:0 uart_lost:0`. It does not fit in the 960 B/s of 9600 baud |
| Cold-start time to fix | about 43 s (first fix at `sats used:5`) | Same day. J3 has no backup supply, so every start is a cold start |

The shutdown sequence (flush → sync → close → rename) has to fit inside that 0.55 s. Writing one
line of debug log measured at well under a few milliseconds (the survival-log lines came out at
exactly 50 ms spacing).

## Layout

| File | Role |
|---|---|
| `src/main.c` | Initialization and the single loop. Only CAN and the GPS UART are interrupt-driven |
| `src/config.h` | Pin assignment and every constant, including the settings-file defaults |
| `src/settings.c` | Settings-file parsing (pure; testable on a PC) |
| `src/settings_file.c` | The FatFs side of the same (read, create with defaults, hidden attribute) |
| `src/motoreco.h` | `struct CANData`. Identical layout to the Pi version (fixed 16 bytes, LE) |
| `src/mcp25625.c` | The CAN controller. Reception goes into a ring buffer from the interrupt |
| `src/sd_spi.c` | The SPI block driver for the SD card |
| `src/diskio.c` / `src/ffconf.h` | Glue to FatFs. `get_fattime()` returns GPS time |
| `src/nmea.c` | GGA / RMC parser. Replaces gpsd |
| `src/wallclock.c` | A wall clock made by pinning GPS time to the monotonic clock |
| `src/record.c` | Building `.dat` records and the fixed-point encoding of GPS |
| `src/logger.c` | File lifecycle, `f_sync`, the debug log |
| `src/led.c` | Blink patterns for the green LED |
| `src/power.c` | The state machine for the IG OFF decision (pure; testable on a PC) |

## Differences from the Pi version

| | Raspberry Pi version | Pico version |
|---|---|---|
| CAN | SocketCAN (`can0`) | MCP25625 directly over SPI1. Interrupt-driven reception. **Listen-only**: sends no ACK and no error frames |
| GPS | gpsd | NMEA on UART0, parsed in-house |
| Storage | `fopen`/`fwrite` on ext4 | FatFs plus our own SD SPI driver |
| Time | NTP-synchronized system clock | GPS time (there is no RTC and no backup) |
| Key on/off | GPIO 27, debounced three times | **IG detection on GP28**. Finalized within the 0.55 s grace period |
| Talking to other processes | System V shared memory | None (the Pico has no equivalent) |
| Debug log | `canlogger.log` | `motoreco.log` plus USB serial |

## How log files are handled

```
Power on (= IG ON)        create TMPnnnnn.DAT and start recording immediately
First GPS fix             close it, rename it to the "start time", reopen for append
IG OFF (key off)          flush -> sync -> close -> rename to the "end time" -> idle
Unexpected power loss     everything up to the last f_sync is safe (losing at most
                          1 second plus 31 records)
```

On every mount (at startup and when a card is swapped), **zero-byte `TMP*.DAT` files are deleted**.
Those are logs that lost power before writing a single record; the automatic reboot right after
flashing firmware always left one behind. A `TMP*.DAT` with content (a ride that ended before GPS
ever supplied a date) is the only copy of that ride, so it is kept. When something is deleted,
`motoreco.log` gets a `deleted N empty TMP*.DAT ...` line.

### Record timestamps are guaranteed to be monotonic

Both kinds of `.dat` record carry **the time at which the interrupt saw the data arrive**.

| Record | Where the timestamp comes from |
|---|---|
| CAN frame | **The time the interrupt captured it** (`frame.timestamp_us`). This feeds CAN timing analysis, so we do not want to lose precision |
| GPS record | **The time the UART interrupt received that sentence's `$`**. Whichever sentence (GGA or RMC) the record was written from, it is that sentence's arrival time |

GPS records used to carry **the time they were written**, which caused two problems:

- They came out of order with respect to CAN. If the interrupt pushed new frames while the main
  loop was draining CAN, a GPS record written afterwards could carry an older timestamp.
  **4 out of 27,846 records in a 10 Hz hardware log, at most 2 ms**
- When the SD card stalled the loop, every fix that arrived during the stall got written in a burst
  afterwards, **all with the same timestamp** (in a 2026-10-13 bench run, after a 0.93-second stall,
  4 fixes landed within 1 ms, up to about 0.8 s off)

The write path now has three stages:

1. **Capture the arrival time in the interrupt.** When `on_uart_rx` puts a `$` into the ring, it
   also puts the timestamp into a timestamp ring (256 entries). If the timestamp cannot be stored,
   the `$` is not stored either, so the two rings cannot drift apart
2. **Parse GPS first and hold the record back.** `handle_gps` does not write to the SD card; it only
   pushes the record and its arrival time onto a pending queue (128 entries). If the queue is full,
   the rest stays in the ring for the next pass
3. **Write it merged with CAN in time order.** `handle_can_frames` looks at the timestamp of the
   next CAN frame (without consuming it, via `mcp25625_peek_timestamp`) and writes any pending
   record that arrived before it. Furthermore, if a `$` arrived before that frame but **has not been
   parsed yet**, it stops writing so the next pass of the loop parses it first
   (`gps_unparsed_before`). Finally, if the CAN ring is empty it writes all pending records,
   otherwise only those older than the next frame

   The first version, which did not wait for unparsed sentences, produced `clamped:82` on the bench.
   It overtook a sentence that arrived mid-drain and wrote newer CAN first, so that sentence's record
   got clamped afterwards. During an 895 ms stall, the 3 fixes that had been waiting all landed in
   the same millisecond. A sentence that stops arriving mid-assembly is given up on after
   `GPS_SENTENCE_MAX_MS` (200 ms), so CAN does not stall if the receiver goes quiet in the middle of
   a sentence

On top of that, `stamp_record()` **never writes a timestamp older than the previous record**
(a monotonicity clamp). This is not a fix but a **guarantee**: if the merge is correct it almost
never fires. The number of records it shifted appears as `clamped:` on the `status:` line, so if
that keeps climbing, the merge is broken.

Because the timestamp is taken at the start of the sentence, it is **a few milliseconds earlier**
than before (when it was the write time) — by the transmission time of one sentence plus the loop
delay, which puts it closer to the moment of the fix. The `.dat` format is unchanged.

The clamp **does not change a single bit of a healthy log** (verified against 26,718 records of a
real 1 Hz log). It only touches the record that is out of order, moving it forward by exactly the
amount of the rewind (at most 2 ms).

> The same comparison **also prevents unsigned underflow**. The first record stamped sets the
> reference time, so if that was "a CAN frame captured after the top of the loop", the subtraction
> for the following GPS record could wrap and put enormous garbage into `second`
> (`18446744073709551615` in the old code). This was a reachable path whenever CAN traffic was
> already flowing at startup, which is the normal case on the bike.

### The wall clock (debug log timestamps and file names)

There is no RTC on the board, so the date and time are handed to `wallclock.c` by **a fixed RMC**.
It is separate from the `.dat` timestamps and is only used for debug-log times and file names.

- The RMC time is read **down to the fractional seconds** (`hhmmss.sss`). The arrival time of that
  sentence's `$`, minus the fractional part, is what the whole second is pinned to. Previously the
  fractional part was discarded and the time was pinned to the top of the loop, so at 10 Hz the
  clock was pulled back by up to 0.9 s on every fix and log timestamps ran backwards (a 16 ms
  rewind was observed on 2026-10-13)
- The clock is only re-synced when the running clock and the fix **disagree by more than 50 ms**
  (`WALLCLOCK_RESYNC_MS`, in `wallclock_sync`), so that a few milliseconds of jitter in arrival
  times does not keep nudging the clock back and forth. The Pico's crystal drifts far more slowly
  than that, so an occasional re-sync is enough even on a long ride
- The clock lags slightly by the receiver's fixed output delay (tens of milliseconds). That is
  plenty good enough for logs and file names

## IG OFF is the one and only finalization trigger

**The only trigger is IG OFF.** A drop in CAN engine speed used to finalize a log as well, but the
CAN ID and the decoding formula differ per model, and the threshold came from the real-world data of
a single bike. **The ignition line says the same thing on any motorcycle**, so everything was moved
onto the one that carries no vehicle-specific knowledge.

Grep the debug log for `trigger:` to find the finalization record:

```
[20260904 154302.233] trigger: ignition off (65mV). log closed and renamed as '20260904_154302.dat'
```

The result reports not just success or failure but **what actually happened**
(`logger_finalize_text()`):

| Result | Meaning |
|---|---|
| `log closed and renamed as '...'` | Synced and renamed; fully complete |
| `log synced but not renamed, no gps time yet` | The data is safe. No GPS fix yet, so no name can be given |
| `no log was open` | No card, or similar |
| `the card refused` | The write failed |

**A log is not closed when the engine stops with the key still on** (stalling at a light, switching
the engine off during a break, and so on). Data safety is handled by the per-second `f_sync`;
finalization happens on IG OFF. Engine speed itself continues to be recorded into the `.dat` as CAN
frames, as before.

## IG OFF detection (GP28)

**The board stays alive for 0.4 s after IGN OFF (typ. 0.5 s)** because C13 holds Q2's gate charge
and discharges slowly through R1. Using that grace period to finalize the log is the firmware's
responsibility; the contract is defined in [`docs/netlist.md`](../docs/netlist.md) §4.7.

| Contract | Implementation |
|---|---|
| Poll GP28 at 50 ms or faster | **A 10 ms timer interrupt** (`IG_POLL_INTERVAL_MS`) |
| Below 0.35 V: flush → close → rename → idle | `logger_shutdown()`. **The sync comes first**; the rename is last |
| The grace period is 0.4 s guaranteed (including a worst-case 250 ms SD stall) | Detection uses 20 ms of it (`IG_OFF_SAMPLES`, two consecutive readings) |

**The reason it is read from a timer interrupt and not the main loop** is that an SD write can stall
the main loop for hundreds of milliseconds. A detection delayed that long would eat the grace
period. The interrupt only detects; the actual file operations are done by the main loop (FatFs is
not reentrant).

### GP28 is not a voltmeter (changed in v3)

**This board cannot measure battery voltage.** In v3, R15 (10k) was connected to Q1's base node, and
that node is **clamped to about 0.65 V** by Q1's base-emitter junction. So all that can be read is
**IG ON (≈650 mV) or OFF (0 V)**. Since 0.65 V is below the VIH of a digital input (about 2 V), it
**has to be read with the ADC**.

| State | GP28 |
|---|---|
| IG ON | about 650 mV |
| IG OFF | 0 mV |
| Thresholds | `IG_OFF_MV` 350 mV, recovery `IG_ON_MV` 500 mV |

The `ig:` field on the status line is this **raw pin voltage**, not the battery voltage:

```
status: rx:412033 records:398211 ig:651mV(on) | gps fix:1 ...
```

### Measuring the hold-up time (diagnostic, off by default)

There is a feature that keeps writing a log line every 50 ms after IG OFF until the power runs out.
**The last line left on the card is the measured value.** Here is what the ride of 2026-09-04
produced:

```
[20260904 154302.233] trigger: ignition off (65mV). log closed and renamed as '20260904_154302.dat'
[20260904 154302.264] power still up 50ms after ignition off
[20260904 154302.314] power still up 100ms after ignition off
...
[20260904 154302.764] power still up 550ms after ignition off   <- it stopped here
```

**The hold-up time is about 0.55–0.6 s**, comfortably above the "0.4 s guaranteed" of §4.7. The
measurement is done, so `IG_OFF_SURVIVAL_LOG` in `config.h` **defaults to 0**. Set it back to 1 if
C13 or the load changes and it needs re-measuring. Because it keeps writing to the debug log while
the power is collapsing, that one file may end up corrupted enough to need chkdsk (**the trip log is
already closed and synced by that point**).

### Behavior

- **It never fires until IG ON has been observed at least once** (`armed`), so a USB-powered bench
  session with GP28 at 0 V does not shut down by mistake
- **If IG comes back, a new log is opened** (for example turning the key back on while USB-powered).
  Files already closed stay closed; the next trip becomes a separate file
- While idle the **LED is off** (`LED_IDLE`), so it cannot be confused with "card error = fast
  blink"
- While idle the 5-second card remount is also suspended, so a log that was just closed does not get
  reopened

## GPS fix interval (PMTK)

The receiver is MediaTek-based, so **PMTK commands can change the fix interval, the output
sentences, and the baud rate** (canonically documented in
[`docs/MT33xx_GNSS_PMTK_Users_Guide`](../docs/MT33xx_GNSS_PMTK_Users_Guide)). The TX line
(GP0 → the module's RXD), unused until now, carries them, **once at startup**. `PMTK251`, `PMTK314`
and `PMTK220` all have `Keep status after reboot -> Yes`, so they persist in the module.

| Command | Purpose |
|---|---|
| `PMTK251` | NMEA baud rate. **This has to come first to get 10 Hz in.** 4800–921600 |
| `PMTK314` | Which NMEA sentences to output and how often (22 fields). Each value is a **divider of the fix rate**: 0 = off, 1 = every fix, 2–5 = every n-th fix |
| `PMTK220` | Fix interval in ms. Range **100–10000** (above 1000 ms it is rounded to whole seconds) |
| `PMTK400` | **Query** the current fix interval. The answer comes back as `$PMTK500,...` |

### The receiver refuses what it cannot do

This is the key thing learned on hardware. **Sending `gps_rate = 100` (10 Hz) at 9600 bps got
refused:**

```
[+0.721s] gps: PMTK220 rejected, flag 2
```

By the guide's definition, flag 2 in `$PMTK001,<cmd>,2` means **"Valid command / packet, but action
failed"** — the command arrived and was understood, but **was not carried out**. Against the 960 B/s
effective capacity of 9600 8N1, emitting GGA+RMC (measured at about 144 bytes per fix) at 10 Hz
needs about 1,530 B/s, so **the module itself rejected an interval it could not output at**. The
measurement agreed: `2.0 sentences/s` (= 1 Hz); the interval had never changed at all.

> Note that the guide at hand only documents this "fails when the baud rate is insufficient"
> mechanism **under PMTK251** (*"If the baudrate is less than the max size of all NMEA sentences, it
> will return error with command PMTK001,251,2"*). For the fix interval, the same statement appears
> in the MT3333 NMEA specification instead.

So **the only way to get 10 Hz is to raise the baud rate**. The startup sequence has to be in this
order:

1. **Raise the baud rate with `PMTK251`** — try the interval first and it will find the ceiling of
   the link you are about to widen, and settle there
2. Narrow the sentence set to GGA+RMC with `PMTK314`
3. Set the fix interval with `PMTK220`. **If it is refused, automatically climb to the next
   candidate and retry** (ladder: 100 → 125 → 200 → 250 → 500 → 1000 ms)
4. **Read the effective value back with `PMTK400`** — do not take the ACK as evidence

### Result: 10 Hz went through at 115200 (measured 2026-10-13)

```
gps: nothing at 9600 baud, listening at 115200
gps: module is at 115200 baud
gps: fix interval accepted (100ms, 10.0Hz)
gps: module holds 100ms fixes (10.0Hz), as asked
gps rate: 20.0 sentences/s 1440 B/s (12% of 115200 baud) avg 72 B/sentence bad:0 uart_lost:0 (want 100ms, module 100ms)
```

**Accepted on the first try, without a single backoff step.** `bad:0 uart_lost:0` means nothing was
dropped either. The estimate of 144 bytes per fix (72 B × 2 sentences) matches the measurement,
confirming that **it never could have fit in the 960 B/s of 9600** — the module's refusal was
correct.

Note that the log above is from **a second or later boot**. `PMTK251` persists in the module, so
once it has been raised, auto-detection finds it at 115200 and does not even need to send `PMTK251`
(`gps_cfg_begin` decides it is "already at the target baud" and skips it). This is why detection is
described below as the real mechanism rather than a nicety.

The log looks like this on a first boot, coming up from 9600:

```
gps: module is at 9600 baud
gps: raising the link from 9600 to 115200 baud
gps: PMTK251 applied, the link is now 115200 baud
gps: configuring for 100ms fixes (10.0Hz), GGA+RMC every fix, GSA+GSV every 5 (2.0Hz)
gps: sentence set applied (GGA+RMC every fix, GSA+GSV every 5)
gps: fix interval accepted (100ms, 10.0Hz)
gps: module holds 100ms fixes (10.0Hz), as asked
```

And like this when it does not fit (**one line tells you which interval it settled for**):

```
gps: PMTK251,115200 did not take, staying at 9600 baud
gps: PMTK220,100 refused (flag 2), backing off to 125ms
gps: PMTK220,125 refused (flag 2), backing off to 200ms
gps: fix interval accepted (200ms, 5.0Hz)
gps: module holds 200ms fixes (5.0Hz), as asked
```

**If `module holds ...` and `as asked` do not agree, it did not go through.** If the read-back says
`gps: module holds 1000ms fixes, we asked for 100ms. PMTK220 did not take`, then the ACK was lying
about itself.

### The baud rate is not knowable in advance (auto-detection)

The `PMTK251` setting survives a reboot, but **a full cold start or a transition to standby resets
it to the default**. On top of that, settings from a previously used card may still be in place. In
other words, **the baud rate the module is talking at when power comes up is genuinely unknown**.

So **we listen before sending anything**. Listen at 9600 for `GPS_PROBE_MS` (1.5 s); if no sentence
with a valid checksum arrives, switch to the other candidate and listen again (up to four times).
This is not decoration but the real mechanism: an implementation that assumes one of the two kills
GPS on a certain fraction of boots.

```
gps: nothing at 9600 baud, listening at 115200
gps: module is at 115200 baud
```

Setting `gps_baud = 9600` **puts it back**. Detection always tries both 9600 and 115200 regardless
of the configured value, so a module left at 115200 is still found and can be brought back.

### Measured throughput

So that nothing rests on guesswork, **the measured value is printed every 10 seconds**:

```
gps rate: 20.0 sentences/s 1440 B/s (12% of 115200 baud) avg 72 B/sentence bad:0 uart_lost:0 (want 100ms, module 100ms)
```

(The numbers above are measured on 2026-10-13, in 10 Hz steady state **with GSA / GSV turned off**.
Now that GSA / GSV are back at 2 Hz, the expected values are about 38 sentences/s instead of about
20, about 2600 B/s, and `% of 115200 baud` in the low twenties.)

- The trailing `(want Nms, module Nms)` is the **self-check**. If those disagree, the interval did
  not go through
- If `% of ... baud` goes over 100%, it is overflowing. `bad:` (checksum mismatch) and `uart_lost:`
  (UART ring overflow) will climb at the same time
- `ring peak:N/8192` is the fullest the UART ring got during those 10 seconds. If it approaches
  8192, the next SD stall will produce `uart_lost:`
- `avg B/sentence` is the measured length of one sentence

### SD stalls and ring sizes

For both CAN and GPS, the interrupt pushes received data into a ring and **the main loop writes it
out to the SD card**. The loop does not come back while the card is busy with a write, so both rings
have to survive that long.

In the 2026-10-13 bench run (CAN replay plus GPS at 10 Hz), **the loop stalled for about 0.93 s**
and GPS reported `uart_lost:455`. The GPS ring was 2048 bytes then, which the roughly 2.7 KB/s of
10 Hz fills in 0.77 s. The CAN ring (4096 entries) held up and not a single CAN frame was lost. So
the GPS ring was raised to **8192 bytes (about 3 seconds' worth)**.

- The SD driver gives up on a single busy-wait after at most 500 ms, but `f_sync` writes several
  sectors, so the total can exceed that. No error is reported unless it actually times out
- GPS sentences that arrived during the stall get processed in a batch once the loop resumes, but
  since the timestamp is the arrival time, **they are recorded at their true spacing** (during the
  0.93 s stall above they carried the write time, and 4 fixes landed within 1 ms — see the section
  "Record timestamps are guaranteed to be monotonic")
- To reproduce a stall on the bench, build with `-DSTALL_TEST_MS=1000`. The loop then halts for that
  long every 10 seconds. **Do not put it in a build for the bike**

Any pass of the loop taking `LOOP_STALL_WARN_MS` (200 ms) or more is logged (the following is a
format example, not a measured value):

```
main loop stalled 934ms (records 0, gps 0, sync 934, other 0 ms). gps ring peak:2520/8192. stalls so far:1 longest:934ms
```

- The parenthesized part says where the time went. `records` is writing CAN and GPS records (the
  `f_write` every 16 KB), `gps` is parsing GPS sentences (and any debug logging in between), `sync`
  is the per-second `f_sync`, and `other` is everything else, such as other debug-log writes
- This line is itself a write to the card, so it appears **at most once per 10 seconds**. The count
  and the longest value keep showing up as `stalls:` / `longest:` on the `status:` line
- Time spent with no card (while retrying the mount) is not counted

### The write unit (16 KB, on sector boundaries)

The two stalls measured on the bench (895 ms and 375 ms) both happened inside `records`, which at
the time meant **a single 512-byte write (CMD24)**. So the way writing works was changed
(`logger.c`):

- **Accumulate 16 KB (1024 records, 32 sectors) and write it in one go.** When the file position is
  on a sector boundary, FatFs bypasses its own sector buffer and calls `disk_write` for multiple
  sectors, which in the driver becomes ACMD23 (pre-erase hint) plus a CMD25 multi-block write. About
  36 single writes per second become about one
- **Keep the file position on a sector boundary at all times.** The per-second `logger_sync()` and
  the rename on the first fix write **only whole sectors**, leaving the remainder (fewer than 32
  records) in the buffer. Previously the per-second sync wrote a partial sector, which made the next
  write rewrite the same sector
- `logger_shutdown()` on IG OFF, and unmounting, write everything including the remainder (it is the
  end, so no boundary is needed)

In exchange, an unexpected power loss now costs "everything since the last `f_sync` (up to about one
second)" plus **the remaining 31 records (tens of milliseconds)**.

Stalls also happen for reasons internal to the card, so this does not take them to zero. Compare the
effect with `stalls:` / `longest:`. When comparing, repeat the same replay several times — stalls
are sporadic, 0 to 1 per 30 seconds of replay.

### Per-sentence rates (GSA / GSV at 2 Hz)

The log itself only needs GGA and RMC, so **only those two run on every fix**. GSA and GSV are
diagnostic, so they are dropped to **every fifth fix** (100 ms × 5 = **2 Hz**). GLL and VTG
duplicate what GGA / RMC already carry, so they are off.

**2 Hz is not a compromise but the limit.** A PMTK314 value is a "divider of the fix rate" and the
guide defines **5 as the maximum** (`5 - Output once every five position fixes`). With fixes at
10 Hz, **nothing slower than 2 Hz (divide by 5) can be requested**. 2 Hz is more than enough to know
how many satellites are visible.

Bandwidth is not a concern now that the link is at 115200:

| Sentences | Rate | Measured / estimated |
|---|---|---|
| GGA + RMC | 10 Hz | **1440 B/s** (measured) |
| GSA + GSV | 2 Hz | about 1200 B/s (estimated) |
| Total | | about 2600 B/s = **about 23% of 115200** |

With GSA / GSV back, the status line's **`sats used` / `view` / `tracked` / `best snr` / `mode:3D`
are populated again** (they stayed 0 while those sentences were off). Diagnosing a slow fix is
impossible without them, so bringing them back is the right call if the bandwidth allows.

> Setting `gps_rate` to 1000 (1 Hz) drops GSA / GSV to **0.2 Hz** at a divider of 5. The divider is
> relative to the fix rate, so for slow intervals it is worth setting the 4th and 5th fields of
> `GPS_SENTENCES_WANTED` (in `config.h`) back to 1.


## The settings file `MOTORECO.CFG`

A **hidden file** in the root of the SD card. If it is missing, it is created with defaults at
startup. Lines starting with `#` are comments. It can be edited in any text editor.

```ini
# MotoRecoPico settings
#
# Lines starting with '#' are comments. Edit with any text editor.
# This file is hidden; enable "show hidden files" to see it.
#
# timezone: offset from UTC, used for the .dat file names and the debug log.
#           whole hours (+8, -5) or with minutes (+5:30).
timezone = +9
#
# gps_rate: position fix interval in ms, 100 to 10000 (100 = 10Hz, 1000 = 1Hz).
#           the receiver refuses an interval it cannot output at the current
#           baud, and the firmware then climbs to the next one it accepts, so
#           this is the fastest rate to TRY. 0 leaves the receiver as it is.
gps_rate = 100
#
# gps_baud: NMEA baud to raise the receiver to, which is what makes 10Hz fit.
#           9600 carries about 6 fixes/s at most. one of 4800, 9600, 14400,
#           19200, 38400, 57600, 115200, 230400, 460800, 921600, or 0 to leave
#           it alone. the firmware finds the receiver at either baud on its own,
#           so 9600 here is also the way back if a faster rate misbehaves.
gps_baud = 115200
```

> That block is exactly what `settings_default_file_text()`
> ([`src/settings.c`](src/settings.c)) writes.

| Key | Meaning | Default |
|---|---|---|
| `timezone` | Offset from UTC. Accepts `+9`, `-5`, `+5:30`, `+5.5`. Affects `.dat` file names and debug-log timestamps | `+9` (JST) |
| `gps_rate` | Fix interval in ms, 100–10000. Refusals climb automatically, so this is **the fastest value to try**. `0` sends nothing to the receiver | `100` (10 Hz) |
| `gps_baud` | NMEA baud rate. Only the values in the guide's table. `0` leaves it unchanged. Write `9600` to go back | `115200` |

> **Existing cards need two lines added.** A `MOTORECO.CFG` written by older firmware has
> `gps_rate = 1000` and no `gps_baud` line, so such a card stays **at 1 Hz / 9600**. Either delete
> the file and let it be regenerated, or add the two lines above by hand.

- Both `key = value` and `key value` are accepted. Keys are **case-insensitive**
- An unparseable line is ignored **on its own** and recorded in the debug log. The whole file is
  never discarded
- A healthy log line looks like: `settings: timezone +09:00`
- Swapping cards **re-reads that card's settings file** (it is read on every mount)

### The retired `vehicle` key

There used to be a `vehicle` key that selected the engine-speed decoding rule (the model of bike).
It became unnecessary when finalization by engine speed was dropped. **Cards written by older
firmware still have that line**, so the parser silently skips `vehicle` as "retired". There is no
need to delete it by hand, and no warning on every boot.

## Pin assignment (board v4.1)

Canonically defined in [`../docs/netlist.md`](../docs/netlist.md) §3.7. `src/config.h` follows it.

| Function | GPIO | Notes |
|---|---|---|
| microSD | GP2/3/4/5 | **SPI0** SCK/MOSI/MISO/CS |
| SD card detect | GP7 | External pull-up R13 |
| GPS UART0 | GP0 (TX) / GP1 (RX) | The module's TXD (orange) goes to GP1 |
| GPS PPS | GP6 | Unused (left as an input) |
| LED, green | GP8 | Lit when driven low |
| CAN MCP25625 | GP10/11/12/13 | **SPI1** SCK/MOSI/MISO/CS |
| CAN INT / STBY / RESET | GP9 / GP20 / GP21 | |
| IG detect | GP28 (ADC2) | Does not change with engine speed. **ON/OFF only** — battery voltage cannot be measured |

**GP14–GP19, GP22, GP26 and GP27 are tied to GND on the board. Never make them outputs.**

The differences from v4 come from rotating the Pico 180°, and **the SPI instances swapped with it**
(SD went SPI1 → **SPI0**, CAN went SPI0 → **SPI1**). To prevent the accident of fixing the pins and
forgetting the instances, `sd_spi.c` and `mcp25625.c` carry `_Static_assert` checks that each pin
really is that instance's SCK/TX/RX (on the RP2350 the instance cycles every 8 pins and the function
every 4).

**stdio must not go to the UART.** GP0/GP1 are also the SDK's default stdio UART pins, so enabling
it would pour printf into the GPS module's receive pin. Debug output is USB CDC only.

## LED (green, GP8, lit when low)

| Indication | Meaning |
|---|---|
| Solid | Recording |
| 1 Hz blink | Recording but no GPS fix yet |
| Fast blink | The card is unusable (no response, mount failure, or write error). Retrying every 5 seconds |
| Off | IG OFF was detected and the log is closed (power normally drops right after) |

## GPS status logging

When a fix takes a long time, the cause is one of "nothing is being received at all because the
wiring or baud rate is wrong", "the antenna is not looking at the sky", or "it is simply still cold
starting" — and **all three look identical from the outside, as "no fix"**. To tell them apart, the
receiver's own view of the world is written to the log.

While there is no fix, every `GPS_REPORT_INTERVAL_MS` (10 s by default):

```
gps searching after 30s: quality:0 mode:1D sats used:0 view:9 tracked:4 best snr:28 hdop:0.0 sentences:301 bad:0
```

How to read it:

| Symptom | Output | Meaning |
|---|---|---|
| Nothing received at all | `gps silent: nothing received on uart0 in 30s...` | Wiring or power. The module talks before it has a fix, so silence is not a fix problem |
| Receiving, but corrupted | `sentences:0 bad:250` keeps climbing | Wrong baud rate (garbled characters fail the checksum) |
| `view:0 tracked:0` persists | Not a single satellite visible | The antenna face is not pointing at the sky, it is inside a metal case, or the module is faulty |
| `view:9 tracked:1 best snr:15` | Visible, but the signal is weak | Obstruction (buildings, indoors, under the tank). It will not fix without several satellites above SNR 30 |
| `view:9 tracked:5 snr:30+` and waiting | A healthy acquisition | Another few tens of seconds and it will fix (see the cold start below) |
| `module holds` disagrees with `want` | `gps rate: ... (want 100ms, module 1000ms)` | `PMTK220` did not go through. Review the baud rate or the number of sentences |
| A strange date before the fix | `gps: the receiver reports 2080-01-06 ... with no fix` | Normal behavior. That line is us **discarding** the receiver's pre-fix guess |

There is also a line at the moment a fix is acquired and the moment it is lost
(`gps fixed after 42s: ...` / `gps fix lost after ...`).

> **`hdop` is not printed with `%.1f`.** The SDK's `printf` cannot be trusted here, so it is
> assembled from two integers (`hdop_tenths()`). When the rounded fractional part lands exactly on
> .5, `_ftoa()` in `pico_printf` takes a branch that **lacks** the `frac >= pow10[prec]` carry guard
> its neighbour has. `frac` becomes 10, the fractional loop runs one extra time, the `unsigned`
> counter wraps to `UINT_MAX`, and the 32-byte buffer is filled end to end with `'0'`. Neither the
> decimal point nor the integer part is ever written.
>
> It showed up four times on a real ride as `hdop:00000000000000000000000000000010` (the rest of the
> line perfectly normal). The trigger is **hdop being exactly 0.95 or 1.95** — ordinary values for a
> good fix, and in the range 0 to 100 the only two that land exactly on a half in double arithmetic.
> The Hz display (`%.1f`) is computed from the fix interval and every reachable value there is safe,
> so it is left alone.

`sats used` / `view` / `tracked` / `best snr` / `mode` come from GSA and GSV, so **turning those two
sentences off zeroes all of them** (they are currently emitted at 2 Hz, so they are populated — see
"Per-sentence rates" above). There is also a `gps talking: first sentence '$GNGGA' at 9600 baud` as
soon as the very first sentence arrives, so **whether the wiring and baud rate are right is settled
immediately after startup**.

### The wiring, confirmed on hardware (2026-08-14)

**The baud rate is 9600 bps** (exactly the default of `GPS_UART_BAUD`). Wire colors to J3 pads:

| J3 pad | Silkscreen | Wire | Color |
|---|---|---|---|
| 1 | V | VCC | Red |
| 2 | G | GND | Black |
| 3 | T | **RXD** | **Green** |
| 4 | R | **TXD** | **Orange** |
| 5 | P | PPS | Brown |

**The T / R on the silkscreen are named from the board's point of view, which is the opposite of the
module's pin names.** A UART crosses over, so `T` (the board transmits) takes the module's
**RXD (green)**. Reading it as "orange TXD goes to T" shorts two outputs together and not a single
byte is received (which is exactly what happened). Canonically defined in
[`../docs/netlist.md`](../docs/netlist.md) §3.5.

### Why the fix itself is slow

**J3 has no backup supply** (J3 in `docs/netlist.md` is just the five wires VCC / GND / TXD / RXD /
PPS). That means **the ephemeris is lost on every key-off and every start is a cold start**. A cold
start on a typical receiver takes 30–45 s under clear sky with a good view, and minutes in poor
conditions. A hot start fixes in one or two seconds, so the difference matters.

Improving it means a hardware change:

- If the GT-502MGG-N has a backup pin, feed it from the permanent 12V rail (check section 2.1 of the
  datasheet)
- Or make J3.VCC a permanent supply (weighed against a flat battery, since the receiver keeps
  drawing standby current)

**The current logging exists to provide the evidence for that decision**: if the N in
`gps fixed after Ns` is 30–45 s every time, it is the cold start itself; if it is minutes every
time, it is obstruction or an antenna problem.

## How card detect (GP7) is treated

**The card detect pin (GP7) is evidence, not the judge.** Whether a card is present is decided by
"does it answer on SPI" (`sd_init()` in [`sd_spi.c`](src/sd_spi.c) always tries, without looking at
the detect line). Three reasons:

- The detect switch sits on **the two smallest pads of the socket (9 and 10)**, where a bad solder
  joint is most likely
- Whether the contact closes or opens on insertion is **not stated in the DM3AT drawing**
  (`SD_CD_INSERTED_LEVEL` is an assumption)
- If it chatters under the vibration of a motorcycle, it would disconnect a perfectly good card

**Recording works normally even with `SD_CD_INSERTED_LEVEL` wrong.** All that is lost is early
detection of a card pulled out while riding, and even that gets caught by the next failed write.

The removal check while riding only becomes active once **the detect line has said "inserted" while
a card actually answered, at least once** (`sd_detect_reliable()`). If they disagree and it works
anyway, a single warning is issued at startup:

```
note: card detect (GP7) reads empty with a working card. ignoring it.
      check pad 9/10 solder, or flip SD_CD_INSERTED_LEVEL
```

## Data volume (from the same real-world log)

All CAN comes to **about 1160 records/s (about 18.5 KB/s)** across 33 distinct IDs. A 27-minute ride
is 30 MB. At that rate the 4096-record receive ring is **about 3.5 seconds' worth**, so it does not
overflow even when the SD card blocks for hundreds of milliseconds.

GPS is **an order of magnitude away** from that. At 1 Hz it is 2 records per second (position plus
altitude/speed), 32 B/s; even at 10 Hz it is 20 records per second, **320 B/s**, under 2% of CAN's
18.5 KB/s. GSA / GSV are never written to the `.dat` at all (they are purely diagnostic), so they
are **completely free as far as the card is concerned**. **Raising the fix rate will never make the
card the bottleneck.**

What does change is that there are ten times as many GPS points inside the `.dat`, so if the viewer
has any processing that assumes about 1 Hz (smoothing, differentiating speed), it is worth a look at
the first high-rate log.

## What to check first on hardware

1. Does USB serial show `MotoRecoPico starting` → `logging to '...'` → `can controller ready`?
2. If not a single CAN frame arrives: suspect `CAN_BITRATE` in `config.h` (500 kbps by default) and
   the termination resistor R8 (see "CAN termination is always on" in `docs/netlist.md`)
3. If the card is unusable, the USB serial message tells you which case it is:
   - `no card: the slot reads empty and nothing answered on the card spi` → the SPI wiring
     (MISO/MOSI/CS), the card not fully seated, or the 3V3 supply
   - `a card is in the slot but no log file could be opened...` → **suspect exFAT first**. Cards
     over 32 GB ship formatted as exFAT, which is rejected because of `FF_FS_EXFAT 0`. Reformat as
     FAT32
   - `note: card detect (GP7) reads empty with a working card` → recording is working. The solder on
     pads 9/10 and the polarity of `SD_CD_INSERTED_LEVEL` can be checked later
4. For GPS, does it get as far as `gps: module is at ... baud` → `gps: module holds ...ms fixes ...
   as asked`? If `module holds` disagrees with `want`, the interval did not go through
5. Does turning the key off produce `trigger: ignition off (...)`, with the file named after the end
   time?
6. Does the recorded `.dat` open in MotoRecoViewer and draw the same way as a log from the Pi
   version?

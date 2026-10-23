# MotoRecoPico

A CAN + GPS data logger for motorcycles. A home-made board carrying a **Raspberry Pi Pico 2
(RP2350)** records the vehicle's CAN bus and GPS to a microSD card. The resulting files open
directly in [MotoRecoViewer](https://github.com/SchwarzeLanzenreiter/MotoRecoViewer).

This is [MotoRecoLogger](https://github.com/SchwarzeLanzenreiter/MotoRecoLogger) — which ran on a
Raspberry Pi plus a HAT — collapsed onto a single board. The recording format is compatible, so old
and new logs read with the same tools.

## Repository layout

| Directory | Contents |
|---|---|
| [`firmware/`](firmware/) | Firmware for the Pico 2 (C / pico-sdk). See [firmware/README.md](firmware/README.md) |
| `cad/kicad/` | KiCad 10 project, fabrication data, generation and inspection scripts |
| `cad/electronics/` | The old EAGLE version (reference only; KiCad is current) |
| [`docs/`](docs/) | Design documents. **[netlist.md](docs/netlist.md) is the canonical description of the circuit** |

## Hardware

| Item | Details |
|---|---|
| Board | 4 layers, 21 × 61 mm, 41 components. The only leaded parts are J1, Y1 and the Pico header |
| MCU | Raspberry Pi Pico 2 (RP2350). Replaceable, in a 2×20 socket. Moved to the back side in v4.2 |
| CAN | Microchip MCP25625 (controller and transceiver in one), driven over SPI1 |
| GPS | YIC GT-502MGG-N. NMEA over UART0 at 9600 bps |
| Storage | microSD (Hirose DM3AT-SF-PEJM5) over SPI0 |
| Vehicle I/F | TE MQS .63, 8 way. Only 5 of them are used: permanent 12V, IG, CAN High, CAN Low, GND |
| Power | 12V → AP63205 buck converter → 5V. The 3.3V rail comes from the Pico's 3V3(OUT) |

**Power is held up for about 0.55 s (measured) after the key goes off.** C13 holds the gate charge
of the high-side switch, and the firmware uses that grace period to close the log. The contract is
in [netlist.md](docs/netlist.md) §4.7.

How the design got here — why four layers, why this placement, what each revision fixed — is
recorded in [netlist.md](docs/netlist.md) and [hardware-design.md](docs/hardware-design.md).
Where to buy the parts is in [parts-sourcing.md](docs/parts-sourcing.md).

## Firmware

- Runs as a **single loop**. There are only two interrupts: CAN reception and the GPS UART
- CAN reception is moved into a ring buffer by the interrupt, so the controller's two-stage FIFO
  does not overflow even when an SD write blocks for hundreds of milliseconds
- NMEA is parsed in-house (replacing gpsd on the Pi version). There is no RTC, so GPS time is
  pinned to the monotonic clock
- FatFs R0.16 is bundled. The SPI block driver for the SD card is our own
- A log is finalized **only on IG OFF detection**. CAN engine speed is not used, to keep
  vehicle-specific knowledge out of the firmware
- The time zone is configured through a hidden file `MOTORECO.CFG` on the microSD card (no rebuild)
- The hardware-independent parts (NMEA, encoding, settings, IG decision, clock) **have tests that
  run on a bare Pico 2 or on a PC**

Build instructions, how to read the logs, and troubleshooting are in
[firmware/README.md](firmware/README.md).

## Recording format (`.dat`)

A sequence of fixed-length 16-byte records. No header, no delimiters, everything little-endian.

| Offset | Type | Contents |
|---|---|---|
| 0 | `uint32` | Seconds since the log started |
| 4 | `uint16` | Milliseconds within that second |
| 6 | `uint16` | CAN ID (standard 11-bit) |
| 8 | `uint8[8]` | Payload (zero-padded when DLC is less than 8) |

GPS rides on CAN IDs that cannot exist, as fixed-point values. **`0x7FF`** carries longitude and
latitude, **`0x7FE`** carries altitude and speed. That way CAN and GPS land in one single time
series.

**`0x7FD`** (added for MotoNavi, not written by the Pi version) carries the absolute time of each
fix, since the record time above only counts from the start of the log. One per fix, written when
the fix's RMC sentence arrives and after any `0x7FF` that sentence produced, so it pairs with the
`0x7FF` at or just before it. It is written while standing still too, when `0x7FF` is skipped
because the position has not changed:

| Bytes | Contents |
|---|---|
| 0–5 | UTC of the fix, Unix milliseconds, 48-bit little-endian |
| 6 | Fix kind in the high nibble (0 none, 1 2D, 2 3D, 3 DGPS/SBAS/RTK), satellites used in the low nibble (15 = 15 or more) |
| 7 | HDOP × 10, rounded (255 = 25.5 or more, 0 = not reported) |

Satellites and HDOP come from the most recent GGA, so they can be one fix older than the time.
Readers that do not know `0x7FD` skip it like any other ID.

> This format is a **contract shared with MotoRecoLogger (the Pi version) and MotoRecoViewer**;
> changing it affects all three repositories. `0x7FF`, `0x7FE` and `0x7FD` are reserved in every
> MotoReco repository.

File names are `YYYYMMDD_HHMMSS.dat` (key-off time, local time). The date is unknown until GPS gets
a fix, so recording starts into `TMPnnnnn.DAT` and the file is renamed to the start time once a fix
arrives.

## Status

- **Verified on the bike with both CAN and GPS recording** (2026-09)
- The board is v4.2. Real operation was confirmed on a physical v4.1 board (v4.2 is identical in
  parts, nets and GPIO assignment)
- Not yet confirmed: an end-to-end check of opening a recorded `.dat` in MotoRecoViewer

## Related repositories

- [MotoRecoLogger](https://github.com/SchwarzeLanzenreiter/MotoRecoLogger) — the logger daemon for
  the Raspberry Pi; this firmware was ported from it
- [MotoRecoViewer](https://github.com/SchwarzeLanzenreiter/MotoRecoViewer) — the Windows
  application that views the recorded `.dat` files

## License

MIT License ([LICENSE](LICENSE)).

The bundled ChaN FatFs is covered by its own license
([firmware/third_party/fatfs/LICENSE.txt](firmware/third_party/fatfs/LICENSE.txt)).

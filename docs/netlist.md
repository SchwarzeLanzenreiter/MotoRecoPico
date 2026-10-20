# MotoRecoPico bill of materials and netlist

The design in [hardware-design.md](hardware-design.md), taken down to reference designators and
component values. The schematic is a drawing of exactly this content.

**This is the heavily simplified version made for a smaller board. 58 parts → 38 parts (35 in v2).
The reference designators were renumbered.**

v5 (2026-10-16, **the JLCPCB-assembled version**. Generated with `MRP_ASSY=jlc`; coexists with the
hand-soldered v4.2):
- **All SMD parts collected on the F side**, for single-sided Economic SMT assembly (chips moved to
  0402; see §4.12)
- **The Pico became an SMD part soldered directly by its castellations** (back side, socket
  retired — all 40 through-holes gone)
- **Q2 → SI2309** (−60 V/±20 V, SOT-23; the ZXMP6A17E6Q is not in LCSC stock)
- **Y1 → 3225 SMD (X322516MLB4SI, a JLC Basic part)**, load capacitors down to 12 pF
- **GPS (J3) removed from the board** (the module's 5 wires go straight to the Pico; GP0/GP1/GP6
  become spares)
- The only hand work is the Pico's direct soldering, D6 and J1

v4.2 (2026-08-23, a revision driven by the hand-soldering experience with a physical v4.1 board):
- **The Pico moved to the back side** (flipped. Hole positions, GPIO assignment and firmware are
  unchanged; see §4.11)
- **Q2 moved to the same hand-soldering land as U4** (2.0×0.65 mm pads) and stood on end at 90°
- **L1's pads were extended 1 mm outwards** on a custom land, rotated 90° to get away from U4's SW
  pad
- Parts and values completely unchanged

v4.1 (2026-08-14):
- **The Pico was rotated 180° on the same side** (the USB port now faces the microSD end, y=61;
  see §4.10)
- **GPIO was reassigned by post-rotation position** (SD=SPI0 GP2–5, CAN=SPI1 GP10–13,
  GPS=UART0 GP0/1. **The firmware pin definitions had to change.** IG_SENSE stays on GP28)
- Not a single hole position changed (the 2×20 grid is 180°-rotationally symmetric; the component
  selection is also completely unchanged)

v4 (2026-08-14):
- **The 5V supply went from the M78AR05 (SIP, 10.2 mm tall) to the AP63205WU-7** (TSOT-26
  synchronous buck plus 4 L/C parts, tallest 3 mm; see §4.9)
- **J1 flipped to the front side** (the vehicle connector now plugs in from the Pico side; the hole
  pattern is mirrored)
- **The GPS silkscreen T/R was corrected** (it was reversed in v1; pad 3 = R, pad 4 = T)
- Side effect: the minimum input became 3.8 V, which **resolved the outstanding cranking-reset
  issue**

v3.1 (2026-08-13, **an important bug fix**):
- **Fixed Q2's pad mapping, which was mirrored relative to the real part** (affects v1 through v3;
  see §4.8)

v3 (2026-08-09, soft power-off and more):
- **Added an IGN OFF delay** (C13: about 0.4–0.5 s of Q2 gate delay; R15: IG sensing into GP28;
  see §4.7)
- **Moved C7/C8 right next to Y1** (2.2/2.8 mm, eliminating XTAL's passage through an inner layer)
- **Moved C6 and C11 to the front side, collecting all 23 of the 1608 chips on the front**

v2 (2026-08-08, a revision driven by the assembly experience with a physical v1 board):
- **Retired IG voltage sensing** (removed R5, D4, C12 and the GP28 connection. The IG detection
  switch remains)
- **GND pads went from a direct pour connection to thermal relief** (for hand soldering;
  spoke 0.4 mm / gap 0.3 mm)
- Two layers were attempted and did not work out (see §4.6 below). **Four layers continues**

The main changes in v1:
- The step-down went from an LM2596 plus 4 external parts to a single **M78AR05-0.5** (inductor and
  capacitors built in)
- The GPS LDO was retired and **GPS is supplied from the Pico's 3V3(OUT)**
- The latching supply was retired in favour of **a high-side switch driven by the IG signal (J1.3)**
- 12V sensing was consolidated into one path and all radial components were eliminated
- U1's input capacitor C1 was removed (input ceiling 32 V → 28 V, so D2 became an SMAJ16A)
- CAN termination went from split (R8+R9+C9+JP1) to **a single 120 Ω**. **D5 (PESD1CAN) was also
  retired**
- The Pico's GPIO assignment was redone to match the board layout (CAN=SPI0, SD=SPI1, GPS=UART0)

> **This document is canonical.** The KiCad schematic and board are generated from this content.
> If you find a discrepancy with the generated files, fix this document after checking it against
> the real data.

---

## 1. Bill of materials (BOM)

### 1.1 Connectors and modules

| Ref | Part | Type number | Package | Notes |
|---|---|---|---|---|
| J1 | Vehicle I/F connector | TE 1-967658-1 | MQS .63, 8 way, RA, THT | Mouser. **Front-side mount (v4)** |
| J2 | MicroSD socket | Hirose DM3AT-SF-PEJM5 | SMD RA | Akizuki 102395 |
| J3 | GPS wiring pads | for the GT-502MGG-N | **5 SMD pads (one side)** | No through-holes |
| U4 | Step-down DC-DC | **Diodes AP63205WU-7** | TSOT-26 | 3.8–32 V in / 5 V fixed, 2 A, synchronous. Back-side mount (v4) |
| U2 | CAN controller | Microchip MCP25625T-E/SS | SSOP-28 | Akizuki 112663 |
| U3 | MCU module | Raspberry Pi Pico 2 | 2×20 socket | Front-side mount |

### 1.2 Semiconductors

| Ref | Part | Type number | Package | Purpose |
|---|---|---|---|---|
| Q1 | NPN | MMBT3904 | SOT-23 | Pulls Q2's gate down when IG is detected |
| Q2 | P-ch MOSFET | **Diodes ZXMP6A17E6Q** | SOT-26 (**v4.2: the same hand-soldering land as U4**) | High-side switch. −60 V / −3 A / AEC-Q101 |
| D1 | Schottky | **B5819W (1 A/40 V)** LCSC C8598 | **SOD-123** | Reverse-polarity protection (in series). A JLC Basic part |
| D2 | TVS | **SMAJ16A (400 W)** | **SMA** | Clamps input surges. **Clamp 26.0 V < U1's 28 V ceiling** |
| D3 | Schottky | **B5819W (1 A/40 V)** LCSC C8598 | **SOD-123** | Supplies VSYS. Diode-ORed with USB. A JLC Basic part |
| D6 | Green LED (side emitting) | KODENSHI LP812-010(T) | Side-view 2812 | Akizuki 116854. On the board edge. **The only indicator** |

**There is no ESD protection device on the CAN bus.** It relies on the MCP25625's own bus voltage
rating (±14 V) and ESD tolerance. Be aware that this is a path for surges, wired directly to the
vehicle harness.

### 1.3 Resistors (all **1608** = 1.6×0.8 mm / 0603)

| Ref | Value | Purpose |
|---|---|---|
| R1 | 100k | Q2 gate → source pull-up (off by default) |
| R2 | 100k | Q2 gate → Q1 collector (**limits Vgs to half the supply**) |
| R3 | 100k | IG → Q1 base. Base current limit (under 1 mA even during a surge) |
| R4 | 22k | Q1 base → GND. Pulls the base down reliably when IG floats |
| R6 | 10k | MCP25625 RESET pull-up |
| R7 | 10k | MCP25625 **common pull-up for the three TxnRTS pins** |
| R8 | **120 Ω** | CAN termination. A single resistor between CANH and CANL |
| R10 | 10k | SD CD_DAT3 (CS) pull-up |
| R11 | 10k | SD DAT0 (MISO) pull-up |
| R12 | 10k | SD **common pull-up for DAT1/DAT2** |
| R13 | 10k | SD card detect pull-up |
| R14 | 1k | LED current limit (about 2.0 mA from 5 V) |
| R15 | 10k | **v3** IG sensing. Series protection from Q1's base node to GP28 (ADC2) |

### 1.4 Capacitors (all **1608** = 1.6×0.8 mm / 0603)

Every 100 nF part is rated **50 V**, because at 1608 a 50 V part exists among JLCPCB's Basic parts
(at 1005 only 16 V was available, which meant treating C2 as a special case).

| Ref | Value | Purpose |
|---|---|---|
| C2 | 100 nF / 50 V | **Bypass for V12P (permanent 12V). Closes D2's surge current loop locally** |
| C3 | 100 nF | MCP25625 VIO |
| C4 | 100 nF | MCP25625 VDD |
| C5 | 100 nF | MCP25625 VDDA |
| C6 | 10 µF | MCP25625 VDDA bulk |
| C7 | 22 pF | Crystal load capacitance |
| C8 | 22 pF | Crystal load capacitance |
| C10 | 100 nF | SD VDD |
| C11 | 10 µF | SD bulk (write peaks) |
| C13 | 10 µF / **25 V** | **v3** Across Q2's gate and source (in parallel with R1). About 0.4–0.5 s of IGN OFF delay |
| C14 | 10 µF / **50 V** (**3216**) | **v4** Buck input. 50 V is mandatory because it sees the 26 V clamp directly |
| C15 | 22 µF / 16 V (**2012**) | **v4** Buck output (V5 bulk) |
| C16 | 100 nF / 50 V (**2012**) | **v4** Bootstrap (BST–SW). 2012 to keep the single-sided 1608 rule |

### 1.5 Others

| Ref | Part | Package | Notes |
|---|---|---|---|
| F1 | PTC fuse, 1 A hold | 1206 | Recommended by U1's datasheet (the 5 V part is a 1 A slow-blow) |
| Y1 | 16 MHz crystal | HC-49S | **Back-side mount** |
| L1 | 6.8 µH inductor | 4030 (4×4×3 mm) | **v4** For the buck. ASPI-4030S-6R8M or similar, saturation ≥1.5 A. **v4.2: a custom land with pads extended 1 mm outwards** |

**41 parts in total. The only leaded parts are J1, Y1 and the Pico header.** The only tall part on
the front is J1 (18.1 mm, moved to the front in v4). As of v4.2 the back carries the Pico (8.5 mm
socket plus the module), and the tallest part underneath it is L1 at 3 mm.
(R5, D4 and C12 were removed in v2; C13 and R15 added in v3; U1 → U4/L1/C14/C15/C16 in v4.
See §4.6 through §4.9.)

### CAN termination is always on

R8's 120 Ω is permanently fitted between CANH and CANL. There is no way to disconnect it.

**A motorcycle's CAN bus is normally already terminated at both ends.** Inserting this unit into it
makes the whole bus 120 Ω ∥ 120 Ω = 60 Ω (40 Ω together with the two existing terminations), which
may stop communication from working at all. This configuration is only reasonable when connecting
alone at the end of a bus, or on a bench. If communication does not work on the bike, remove R8
first and check again.

### ~~Rating change caused by removing U1's input capacitor~~ (U1 retired in v4; kept as a record)

The M78AR05's input ceiling goes from **32 V to 28 V** (the datasheet's Note 7 condition — "32 V is
permitted with 22 µF/50 V at the input" — no longer applies). The module has filter capacitors
internally, so operation itself does not need an external one.

To match that, **D2 was changed from an SMAJ18A to an SMAJ16A**.

| | SMAJ18A | SMAJ16A |
|---|---|---|
| Standoff | 18 V | 16 V (above the 14.4 V charging voltage) |
| **Clamping voltage** | 29.2 V (**exceeds 28 V**) | **26.0 V** ✓ |

**Do not go back to the SMAJ18A.** Its 29.2 V clamp exceeds U1's 28 V input ceiling.
**(No longer applicable from v4 onwards.)** With U1 retired, the ceiling became the AP63205's 32 V,
so an SMAJ18A is acceptable too. D2 in v5 is "an SMAJ16A from any manufacturer (first choice
Littelfuse C74561), or an SMAJ18A / SMAJ16CA if unavailable".

---

## 2. Power architecture

```
J1.8 (perm. 12V) - F1(1A PTC) - D1(SOD-123) -+- D2(SMAJ16A TVS)
                                             +- C2(100nF)
                                             +- R1(100k) ---+
                                             |              |
                                             +- Q2.S        |
                                                  Q2.G -----+
                                                  Q2.D      |
                                                   |     R2(100k)
                                                   |        |
                                                   |     Q1.C
J1.3 (IG signal) - R3(100k) -+- Q1.B                Q1.E - GND
                             +- R4(22k) - GND
                             +- R15(10k) - U3.GP28 (ADC2; IG sensing, v3)

(v3) C13(10uF) sits across Q2's G-S, keeping Q2 conducting for about 0.4-0.5 s after IGN OFF

Q2.D -+- C14(10uF/50V) - GND
      +- U4.EN
      +- U4.VIN => U4.SW - L1(6.8uH) -+- V5 -+- U2.VDDA (MCP25625)
(AP63205: C16 across BST-SW, C15/22uF     |
 and U4.FB on V5)                         +- D3 - U3.VSYS (to the Pico; ORed with USB)
                                          +- R14 - D6 green LED

U3.3V3OUT - V3V3 -+- U2.VIO / U2.VDD
                  +- J2 (MicroSD)
                  +- J3.VCC (GPS, 50mA)
                  +- all the pull-ups
```

**After IGN OFF, power is held for about 0.4–0.5 seconds (v3)** because C13 restricts the gate
discharge of Q2 to the path through R1. The firmware detects the loss of IG on GP28 and must
complete flush → close → rename within that window (the contract is detailed in §4.7). There is no
hardware latch, so power physically drops even if the firmware hangs.

**The load on pin 3 is only 98 µA** (100k+22k). No current is drawn from the signal line.

### Current estimate

| Load | Current | As 5 V |
|---|---|---|
| MCP25625 VDDA (maximum, transmitting) | 70 mA @5V | 70 mA |
| Pico itself 20 mA / GPS 50 mA / MCP25625 VDD and VIO 10 mA / SD write peak 100 mA | 180 mA @3.3V | 132 mA |
| **Total** | | **about 202 mA** |

U1 is rated 500 mA, 2.5× the margin. The external load on 3V3(OUT) is 160 mA, within the
recommended 300 mA ceiling.

---

## 3. Netlist

### 3.1 Power

| Net | Connections |
|---|---|
| `V12_RAW` | J1.8, F1.1 |
| `V12_FUSED` | F1.2, D1.A |
| `V12P` | D1.K, D2.K, C2.1, R1.1, Q2.S, C13.2 |
| `V12_SW` | Q2.D, U4.VIN, U4.EN, C14.1 |
| `V5` | L1.2, C15.1, U4.FB, U2.VDDA, C5.1, C6.1, D3.A, R14.1 |
| `SW_5V` | U4.SW, L1.1, C16.1 |
| `BST_5V` | U4.BST, C16.2 |
| `VSYS` | D3.K, U3.VSYS |
| `V3V3` | U3.3V3OUT, U2.VIO, C3.1, U2.VDD, C4.1, J2.VDD, C10.1, C11.1, J3.VCC, R6.2, R7.2, R10.2, R11.2, R12.2, R13.2 |

**GND**: J1.4, D2.A, C2.2, C3.2, C4.2, C5.2, C6.2, C7.2, C8.2,
C10.2, C11.2, C14.2, C15.2, U2.GND, U2.VSS, R4.2, Q1.E, U4.GND,
U3.GND1–GND7, U3.AGND, J2.VSS, **J2.TR**, J2.L1, J2.L2, J2.L3, J2.BR, J3.GND

### 3.2 High-side switch / IG detection

| Net | Connections |
|---|---|
| `IG_RAW` | J1.3, R3.1 |
| `Q1_B` | R3.2, R4.1, Q1.B, R15.1 |
| `IG_SENSE` | R15.2, U3.GP28_ADC2 |
| `Q2_G` | R1.2, R2.1, Q2.G, C13.1 |
| `Q1_C` | R2.2, Q1.C |

### 3.3 CAN

| Net | Connections |
|---|---|
| `CANH` | J1.6, U2.CANH, R8.1 |
| `CANL` | J1.5, U2.CANL, R8.2 |
| `TXCAN` | U2.TXCAN, U2.TXD |
| `RXCAN` | U2.RXCAN, U2.RXD |
| `CAN_SCK` | U3.GP10, U2.SCK |
| `CAN_MOSI` | U3.GP11, U2.SI |
| `CAN_MISO` | U3.GP12, U2.SO |
| `CAN_CS` | U3.GP13, U2.CS |
| `CAN_INT` | U3.GP9, U2.INT |
| `CAN_STBY` | U3.GP20, U2.STBY |
| `CAN_RESET` | U3.GP21, U2.RESET, R6.1 |
| `XTAL1` | U2.OSC1, Y1.1, C7.1 |
| `XTAL2` | U2.OSC2, Y1.2, C8.1 |
| `TXNRTS` | U2.TX0RTS, U2.TX1RTS, U2.TX2RTS, R7.1 |

`TXCAN`/`RXCAN` are the board traces that join the controller section to the transceiver section.
**Omit them and nothing communicates at all.**

`TXNRTS` gathers three unused inputs onto one pull-up. The MCP25625's TxnRTS pins are input-only, so
there is no short-circuit hazard, but going through one resistor keeps some protection against a
misconfiguration.

### 3.4 MicroSD

| Net | Connections |
|---|---|
| `SD_CS` | U3.GP5, J2.CD_DAT3, R10.1 |
| `SD_MOSI` | U3.GP3, J2.CMD |
| `SD_SCK` | U3.GP2, J2.CLK |
| `SD_MISO` | U3.GP4, J2.DAT0, R11.1 |
| `SD_DAT12` | J2.DAT1, J2.DAT2, R12.1 |
| `SD_CD` | U3.GP7, **J2.TL**, R13.1 |

The pull-up on `SD_MOSI` was removed (unnecessary, since the MCU drives it at all times).
`SD_DAT12` gathers the two lines unused by SPI onto one pull-up.

**Card detect is J2.TL (pad 9 of the official KiCad footprint). Settled.**
TR/BR were initially assumed to be the detect switch, which was wrong. The official KiCad
`microSD_HC_Hirose_DM3AT-SF-PEJM5` footprint and our own footprint agree on 13 of 14 pads to within
0.001 mm, and the official one numbers the detect switch pair as pads 9 and 10 (corresponding to TL
and L2 in this document). The symbol `Connector:Micro_SD_Card_Det_Hirose_DM3AT` likewise states
pin 9 = `DET_B` and pin 10 = `DET_A`, so two independent sources back it up. TR, L1, L3 and BR are
the GND legs of the cover.

The older revision tied both ends of the switch (TL and L2) to GND and took the detect signal from
TR, which is a GND leg — so **it read as "inserted" whether or not a card was present**.

### 3.5 GPS and indication (one LED)

| Net | Connections |
|---|---|
| `GPS_RX` | U3.GP1, **J3 pad 4** (connect the GPS module's **TXD**) |
| `GPS_TX` | U3.GP0, **J3 pad 3** (connect the GPS module's **RXD**) |
| `GPS_PPS` | U3.GP6, J3 pad 5 |
| `LED_A` | R14.2, D6.A |
| `LED_STATUS` | D6.K, U3.GP8 |

### J3 wiring (confirmed on hardware, 2026-08-14)

The GT-502MGG-N comes out as five bare wires 1.5 m long. **The wire colors are not documented in the
datasheet and were determined on hardware.**

| J3 pad | Silkscreen | Net | Pico | Wire | Color | Direction |
|---|---|---|---|---|---|---|
| 1 | V | `V3V3` | 3V3(OUT) | VCC | **Red** | — |
| 2 | G | `GND` | GND | GND | **Black** | — |
| 3 | R | `GPS_TX` | GP0 (UART0 TX) <v4.1; formerly GP16> | **RXD** | **Green** | Pico → module |
| 4 | T | `GPS_RX` | GP1 (UART0 RX) <v4.1; formerly GP17> | **TXD** | **Orange** | module → Pico |
| 5 | P | `GPS_PPS` | GP6 <v4.1; formerly GP18> | PPS | **Brown** | module → Pico |

> **In v4 the silkscreen was changed to the module's names (ordered V G R T P).**
> The module's RXD (green) goes to `R` and its TXD (orange) to `T`, so **the wire labels match the
> silkscreen** and it can be wired by reading it off directly.
> The v1 silkscreen was from the board's point of view (pad 3 = T / pad 4 = R), and reading it as
> "orange TXD goes to T" actually produced a miswiring that shorted two outputs together
> (2026-08-14). The net assignment itself has been correct since v1.

**The LED is low-side driven from 5 V. It lights when the GPIO is driven low (inverted logic).**
At 3.3 V there is only 0.3 V of headroom against the green LED's typical VF of 3.0 V, which does not
work.

**There is only one indicator, a green LED.** Errors and states are expressed as blink patterns
(for example: solid = logging, 1 Hz blink = no GPS fix, fast blink = SD error).
The red LED (D7) and R15 were removed and GP5 went back to being a spare.

### 3.6 Deliberately unconnected

| Part | Pins | Reason |
|---|---|---|
| J1 | 1, 2, 7 | Unused on the vehicle side |
| U2 | 2, 25 (NC) | Manufacturer-designated no-connect pins |
| U2 | 11 Rx1BF, 12 Rx0BF, 22 CLKOUT | Outputs. Left open since SPI can do the job (outputs may float) |
| U3 | GP14–GP19, GP22, GP26, GP27 | Spares (after the v4.1 reassignment) |
| U3 | RUN, ADC_VREF, 3V3_EN, VBUS | Unused |

**The MCP25625's TxnRTS pins are inputs and must not be left open** (pulled up by R7). Rx0BF/Rx1BF
are outputs and may be left open.

**ADC_VREF being unconnected is correct.** It is internally connected to filtered 3V3 on the Pico
board.

The schematic carries an explicit No-Connect on all 21 unconnected pins (ERC passes).

### 3.7 Pico GPIO assignment (v4.1)

**It is assigned to match the board layout.** The original plan, decided by logical tidiness, was
the opposite of the physical layout and put CAN's SPI at the microSD end of the board, where it
could not be routed (solved in v1 by reassigning on a distance basis).
**Because the Pico was rotated 180° in v4.1**, every pin moved to its diagonal position
(pin k ↔ k+20) and everything was reassigned on the same distance basis. **The swap of the SPI0 and
SPI1 roles** (SD=SPI0, CAN=SPI1) is the key to a combination that is both legal and nearest-neighbor
at the post-rotation positions.

| Block | Header pins | GPIO | Peripheral |
|---|---|---|---|
| microSD (right column, right beside J2's contacts) | 4,5,6,7 | GP2–GP5 | **SPI0** SCK/TX/RX/CSn |
| the same | 10 | GP7 | CD |
| CAN (right column, beside U2) | 14,15,16,17 | GP10–GP13 | **SPI1** SCK/TX/RX/CSn |
| the same | 12 | GP9 | INT |
| the same (left column, U2's opposite flank) | 26,27 | GP20, GP21 | STBY / RESET |
| GPS (bottom of the right column, directly above J3) | 1,2 | GP0, GP1 | **UART0** TX/RX |
| the same | 9 | GP6 | PPS |
| LED (right column) | 11 | GP8 | GPIO (low side, lights when low) |
| IG sensing (left column) | 34 | GP28 | **ADC2** (unchanged in v4.1. Cannot be read as a digital input — see §4.7) |

**The firmware must follow this assignment.** Differences from firmware for v4 and earlier: SD went
SPI1 → **SPI0**, CAN went SPI0 → **SPI1**, UART0 went GP16/17 → **GP0/1**, and the GPIO numbers for
CD/INT/STBY/RESET/PPS/LED changed as in the table above. **Only GP28's ADC configuration is
unchanged.**
Board-side checks: `mrp_brd.py` asserts, on every build, the pad-to-net mapping of all 17 pins, the
8 GND holes, and which way the USB end faces.

---

## 4. Removed parts and why

| Old ref | Part | Reason for removal |
|---|---|---|
| L1, C3, C4, D3 | 100 µH, 220 µF, 100 nF, SS34 | Built into the M78AR05 |
| U2, C7, C8 | AMS1117-3.3 and its input/output capacitors | GPS is supplied from the Pico's 3V3 |
| Q1, R1, R2, R3, D4, R9 | The latching supply (BAT54C etc.) | Changed to a switch driven directly by IG |
| R6, R7, R8, D8, C6 | The second 12V sensing path | Consolidated into the single IG line voltage |
| D12 | 5.6 V zener | The M78AR05 has built-in short-circuit protection, thermal shutdown and a 1 A output limit |
| C1 | 22 µF/50 V electrolytic | Removed by the user's decision. The input ceiling goes 32 V → 28 V, so D2 became an SMAJ16A |
| D13 | 12 V zener | Q2's gate uses a 100k/100k divider instead, limiting Vgs to half the supply |
| R12, R13 | Two TxnRTS pull-ups | Three pins consolidated onto one pull-up |
| R16, R18 | Pull-ups for the SD's CMD and DAT1/DAT2 | CMD does not need one; DAT1/DAT2 consolidated onto one |
| R9, C9, JP1 | CAN split termination (60 Ω ×2 + 4.7 nF + a disconnect jumper) | Simplified to a single 120 Ω by the user's decision |
| D5 | PESD1CAN (a TVS for CAN) | Removed by the user's decision. **It was also wired incorrectly** (see below) |

### The D5 wiring error (resolved by its removal)

In Table 2 of Nexperia's datasheet Rev.04, the PESD1CAN's **pin 3 is the common cathode** and
pins 1 and 2 are the lines being protected (the footnote to the characteristics table also says
"Measured from pin 1 to 3 or 2 to 3", and the application circuit of Fig 8 has CANH→1, CANL→2,
3→SPLIT).

The old design connected **pin 2 to GND and pin 3 to CANL**. The common terminal and a line terminal
were swapped, which put the protection device in series between CANH and CANL with no path to GND at
all. The part was removed entirely, so the error is gone.

### The type number and package of D1/D3 did not match (resolved by moving to the B5819W)

**The SS14 only exists in SMA (DO-214AC).** The board land is SOD-123, so ordering the SS14 from the
old description's "SS14 or B140" meant a part that physically would not fit. JLCPCB's stock of the
SS14 is also SMA only (C2480).

**It was changed to the B5819W (LCSC C8598)**, a 40 V / 1 A SOD-123 part that satisfies the old
description's requirement of "1 A/40 V, SOD-123" exactly. Its forward voltage is 0.6 V max at 1 A,
about 0.3 V at the roughly 100 mA actually used, so the voltage-drop estimate for cranking (see
"Outstanding issues" below) does not change. It is also a JLCPCB Basic part, so there is no feeder
charge either.

### All chip parts changed from 1005 to 1608

The trigger was C2's voltage rating. C2 sits on V12P and goes up to D2's clamping voltage of 26 V,
but the 100 nF 0402 parts in JLCPCB's Basic list are **16 V only**. Initially C2 alone was raised to
1608 to deal with that.

Then **whether all 21 chips could be 1608** was measured and confirmed, and adopted (details in
[study-1608-chips.md](study-1608-chips.md)).

| | 1005 | **1608 (adopted)** |
|---|---|---|
| Total chip area | 37.8 mm² | 91.0 mm² |
| Board size | 21×61 mm | **unchanged** |
| Unrouted / DRC / unconnected | 0 / 0 / 0 | **0 / 0 / 0** |
| Total decoupling distance | 101.0 mm | **84.7 mm** |

The layout works even at 2.4× the area, and **the bypass capacitors actually ended up closer to the
ICs**. On top of that every 100 nF is now 50 V, and hand soldering and rework get easier. The
special-case handling for C2 alone is no longer needed.

### JP1 did not do what it claimed (resolved by its removal)

The old description said "disconnects the CAN termination; closed by default", but in the actual
circuit JP1 was merely in series with C9: opening it still left R8+R9's 120 Ω across the bus. It did
not have the function the document described.

---

## 4.5 Part categories for JLCPCB assembly

Only the **34 surface-mount parts** are assembled by them. The through-hole parts
**J1, U1, U3 and Y1** are fitted by hand and are split out into
`fab/MotoRecoPico-bom-tht.csv`.

| Category | Count | Parts |
|---|---|---|
| **Basic** (feeder charge $0) | 26 | 13 resistors, 10 capacitors (all 1608), Q1, **D1/D3** |
| **Extended** ($3 each) | 6 | D2, D4, F1, J2, Q2, U2 |
| **Not in JLC stock** | 2 | D6 (the side-view LED), J3 (the GPS module) — fitted by hand or supplied |

**Moving to 1608 took C2 from Extended to Basic** (the 1608 100 nF has a 50 V Basic part).

LCSC numbers of the Basic parts (all 0603 / 1608):

| Value | LCSC |
|---|---|
| 100 kΩ | C25803 |
| 22 kΩ | C31850 |
| 10 kΩ | C25804 |
| 120 Ω | C22787 |
| 1 kΩ | C21190 |
| 100 nF 50 V | C14663 |
| 22 pF 50 V | C1653 |
| 10 µF 10 V | C19702 |
| MMBT3904 | C20526 |
| B5819W | C8598 |

**Why the rest could not be made Basic**

| Part | Reason |
|---|---|
| Q2 | The Basic P-channel part is the AO3401A (SOT-23-3 / 30 V / **Vgs ±12 V**). The gate uses a 100k/100k divider so Vgs = V12P/2, which is **13 V when D2 clamps — over the rating**. The package is different too |
| U2 | Basic only has the two-chip combination of MCP2515 (SOIC-18) + TJA1051 (SOIC-8). The SOIC-18 is 300 mil wide and **would stretch the board by roughly 10 mm**, which is not worth a redesign to save $3. The MCP25625-E/SS (**C184944**) is in stock as Extended |
| D2 | The only Basic TVS is the P6SMB6.8CA (SMB / 6.8 V). There is no SMA part with a 16 V standoff |
| D4 | Basic only has the ZMM3V3 (**LL-34 MELF**). There is no SOD-123 zener. Switching to LL-34 would make it Basic |
| F1, J2 | Basic has **not a single** resettable fuse or connector |
| ~~C2~~ | Moved to Basic when all chips went to 1608. **Resolved** |

The Basic list used for this comparison is a community-published snapshot (691 parts), while JLC's
current "Basic & Promotional Extended" has 1,586. Some of the Extended parts above may currently
have no feeder charge, so check the badges on the ordering page.

---

## 4.6 Record of the v2 revision (2026-08-08)

Two changes driven by hand-assembling a physical v1 board, and the re-verification that followed.

### Retiring IG voltage sensing

Measuring the IG line voltage with the Pico's ADC (GP28) was dropped. What was removed were
**the parts that existed only for the ADC**: D4 (the ADC protection zener), C12 (the ADC filter) and
R5 (the resistor isolating the ADC tap from Q1's base).

**The IG detection switch (power on when the key turns) remains.** Q1's base was connected directly
to the R3/R4 divider point:

- R3 (100k) limits the base current (IG is a line with no TVS, but even a 100 V surge delivers under
  1 mA to the base)
- R4 (22k) pulls the base to GND when IG floats
- Against about 100 µA of base drive at 14 V, the load on Q2's gate is about 70 µA, so it saturates
  at an hFE of 3 or more — two orders of magnitude of margin against the MMBT3904's hFE of 30–300

**What was lost**: voltage logging, and detecting an engine stop from the drop in charging voltage
(14.4 V → 12.6 V). Closing the log before the key goes off has to be guaranteed by bus information
or by an incremental flush design (see §2).

### Thermal relief on the GND pads

In v1 every GND pad was connected directly to the pour, and heat escaped into the plane during hand
soldering, making it hard to work with (a trade-off that was anticipated and documented at design
time, but judged unacceptable on a physical board). In v2 **every zone uses thermal relief**
(spoke 0.4 mm / gap 0.3 mm, minimum spoke count 1).

The current capacity of a 0.4 mm spoke is ample against this board's maximum pin current
(about 200 mA).

### The two-layer attempt, which did not work out

Since three parts had been removed, whether two layers would work was checked by measurement. The
result was **no**.

| Condition | Result |
|---|---|
| 2 layers / 0.15 mm | 3 nets unrouted (CAN_SCK, CAN_MOSI, LED_A; in all 20 runs) |
| 2 layers / 0.127 mm (process minimum) | **Everything routes**, but 2 to 6 pads are left without a GND thermal spoke reaching them (across all combinations of 5 seeds × gaps of 0.3/0.25/0.2) |
| **4 layers / 0.15 mm** | **Works on the first try.** 0 violations, 0 unconnected, 0 floating copper |

The spokes do not reach on two layers because the outer pour is chopped into fragments by the
routing, leaving no copper around the pads. It is exactly the problem recorded in v1 as "with
thermal relief, the Pico's GND pins do not connect", and removing parts did not fix it.
**Going back to a direct pour connection does make two layers work**, but that contradicts the point
of this change (hand solderability), so it was not taken and four layers continues.

One more placement-related lesson to add: after R5 was removed, the auto-placer put the divider's R3
and R4 in different rows, far apart, and the two pads between them (Q1_B) alone were unroutable in
all 20 runs. Changing the packing order within the zone to `R1,R3,R4,R2` to make them adjacent fixed
it (there is a comment in `mrp_brd.py`).

---

## 4.7 Record of the v3 revision (2026-08-09)

### The IGN OFF delay (soft power-off)

In v2 power dropped within a few milliseconds of IG OFF, leaving no time to close the log. v3
creates a window of **"0.4 s guaranteed, 0.5 s typical"** with two parts.

**C13 (10 µF/25 V, across Q2's G–S, i.e. exactly parallel to R1)**
After IG OFF turns Q1 off, the gate can only discharge through R1 (100k). Vgs decays from −7 V with
τ = R1×C13, and Q2 keeps conducting while |Vgs| > 3.5 V. Including the MLCC's DC bias derating
(6–7 µF effective), t = 0.69τ ≈ **0.4–0.5 s**.

| Side effect | Assessment |
|---|---|
| Power-on delay ≈ 0.2–0.3 s | Buried in the boot sequence. Harmless |
| Key contact chatter | Absorbed by the RC (an improvement) |
| Q2 passing through its linear region | ≤1 W for 0.1–0.3 s during the transition. Acceptable for the transient thermal capability of a SOT-26 |

**Fail-safe**: unlike a latching supply, power physically drops even if the firmware hangs. There is
no mode that can flatten the battery.

**R15 (10k, from Q1's base node to GP28)**
Q1's base is **always clamped to about 0.7 V or less** by its own B-E junction (and surges are
limited by R3's 100k), so no zener, divider or filter is needed at all. It reads about 0.65 V when
IG is on and 0 V when off.

> **It must be read with the ADC (a 0.35 V threshold is recommended).** 0.7 V is below VIH
> (about 2 V), so a digital input cannot read it. All that can be read is ON/OFF; the battery
> voltage logging of v1 is not coming back.

### The firmware contract

1. Poll GP28 (ADC2) **at 50 ms or faster**
2. When it falls below 0.35 V: flush → close → rename → idle
3. The time available is **0.4 s guaranteed** (which accommodates a worst-case SD write stall of
   ~250 ms)
4. To extend the window, add C13 in parallel (about +0.4 s per 10 µF). **Do not change R1** (it
   breaks the Vgs divider ratio and brings the gate close to its ±20 V rating during a load dump)

### Moving C7/C8 right next to Y1

Measurements on v2 found C7/C8 11.1 / 9.0 mm from the crystal, with **XTAL1 running 8.5 mm inside
In1 (the GND plane layer)**. Functionally acceptable (the frequency deviation is tens of ppm, a
hundredth of what CAN tolerates), but a permanent 16 MHz aggressor carving a trench through the plane
is poor EMI practice.

Y1 is through-hole and has lands on the front side too, so they were **explicitly placed** to the
left and right of Y1's two front-side barrels (x13.15 / x16.85). The result:

| | v2 | **v3** |
|---|---|---|
| C7 ↔ Y1 | 11.12 mm | **2.24 mm** |
| C8 ↔ Y1 | 8.96 mm | **2.77 mm** |
| XTAL1 | 21.2 mm, 8.5 mm on an inner layer | **9.7 mm, 0 vias, 0 inner layer** |
| XTAL2 | 17.1 mm, 1 via | 10.5 mm, 1 via, 0 inner layer |

Left to the zone it failed twice because spill from C6 stole the slot, so it is an EXPLICIT placement
(see `mrp_brd.py`; C8 is rotated 270° — at 90° its pin 1 faces the far side and the distance becomes
4.1 mm).

### Moving C6 and C11 to the front (consolidating 1608 on one side)

The two 1608 parts left on the back were brought into a front-side zone, so **all 23 of the 1608
chips are on the front**. The automatic "all chips on the front" check was extended to C1608 as well.
The back now has only SOT/SOD/SMA/THT parts.

| | v2 | v3 |
|---|---|---|
| C6 ↔ U2.VDDA | 11.52 mm (back side, via a via) | **3.85 mm (same side)** |
| C11 ↔ J2.VDD | 4.55 mm (back side, via a via) | 7.36 mm (same side) |

C11 got numerically worse, but the SD's high-frequency content is handled by C10 (4.74 mm) and the
3V3 plane on In2, and a 10 µF bulk capacitor is insensitive to a few millimeters, so it was accepted.

---

## 4.8 v3.1: fixing the mirrored G/S in Q2's footprint (2026-08-13)

**In every version from v1 to v3, the pad mapping of Q2 (the ZXMP6A17E6Q) was mirrored relative to
the real part.** The error was introduced when the EAGLE library was created, and came to light
during bench testing of a physical v1 board.

### Checked against the datasheet

Diodes DS36685 Rev 3-2 p.1, "Pin Out - Top View" (the PDF was verified directly on 2026-08-13):

| Pin | 1 | 2 | **3** | **4** | 5 | 6 |
|---|---|---|---|---|---|---|
| Real part | D | D | **G** | **S** | D | D |
| Old library (wrong) | G | D | D | D | D | S |

With the G/S pair mirrored, **no mounting orientation of the real part works**:

| Orientation | What happens | Bench symptom |
|---|---|---|
| A | The common drain metal shorts V12P, V12_SW and Q2_G together | **V12_SW = 13.8 V regardless of IG (always on)** |
| B (A rotated 180°) | G sits on V12P and S on the divider node, so the FET is permanently off | **V12_SW never comes up at all** |

Both symptoms were measured on the v1 board, and this check pinned down the root cause.

### What was changed (v3.1)

- Added a SOT26 remapping to PADMAP in `mrp_design.py` (G→3, S→4, D→1,2,5,6)
- Renumbered the pins of the custom symbol `MotoRecoPico:ZXMP6A17E6Q` to the same mapping
- Added a **permanent check** to `mrp_brd.py`: the build fails if Q2's pad-to-net mapping does not
  agree with the datasheet
- Rerouted and fully re-verified (DRC 0/0, 38 nets agree, 0 inner-layer XTAL, C7/C8 still ≤2.8 mm).
  As a by-product the 12V chain shortened from 13.81 to **12.53 mm** (the S pad moved to the D1 side)

### Field fix for the physical v1 board (for the record)

Swapping just the two G/S leads makes it work. With the marking readable (dot at the lower left), the
**two leads at the right-hand end** are G (near) and S (far). Point that end toward U1, solder the
four D leads normally, lift the two G/S leads, and cross-jumper them: the part's S to the V12P pad
(the one that beeps as continuous with D1's cathode) and the part's G to the other pad. Verified:
with IG on/off, Q2_G goes 7 V ⇔ 13.8 V and V12_SW goes 13.8 V ⇔ 0 V.


## 4.9 Record of the v4 revision (2026-08-14)

### Shrinking the 5V supply: M78AR05 → AP63205WU-7

U1 (SIP-3, 10.2 mm tall, rated 500 mA) was oversized for a load of about 200 mA and was the
second-tallest part on the board. It was replaced by the AP63205WU-7 (TSOT-26, 3.8–32 V → fixed 5 V,
2 A synchronous, 1.1 MHz, spread-spectrum EMI measures, Iq 22 µA) plus L1/C14/C15/C16.
**The pinout (1=FB 2=EN 3=VIN 4=GND 5=SW 6=BST) was checked against both the official KiCad symbol
and DS41326 p.1** (the lesson from Q2). A per-build automatic check was added as well.

| Measured | Value |
|---|---|
| SW node (U4.5 ↔ L1.1) | 2.09 mm |
| Input capacitor (U4.3 ↔ C14.1) | 3.04 mm |
| Bootstrap (U4.6 ↔ C16.2) | 4.00 mm |
| 12V chain total | 14.83 mm |

**The 3.8 V minimum input** structurally resolves the outstanding cranking-reset issue.
The 32 V input ceiling leaves 6 V of margin over the 26 V clamp.

### Moving J1 to the front side

The vehicle connector now plugs in from the opposite side of the board (the Pico side). The MQS
staggered grid is asymmetric left to right, so flipping the side **mirrored the hole pattern as
well** (the 12V entry moved from x17.5 to x3.5). The power row (F1→D1→Q2→buck) was mirrored to match,
keeping the order of flow. The pin-to-net mapping follows the footprint, so it is automatically
correct.
**The assumptions about the enclosure invert**: the front is J1 at 18.1 mm plus the Pico, the back is
3 mm tall at most.

### Correcting the GPS silkscreen T/R

The v1 silkscreen had pad 3 = T / pad 4 = R, but pad 3 is wired to the Pico's UART TX, so from the
module's point of view R (connect the module's RXD wire) is correct. It came to light while wiring a
physical board. The generated string in mrp_fp.py was corrected from "VGTRP" to "VGRTP".

### An XTAL guard on the routing seed

During v4's rerouting it turned out that with some seeds XTAL2 ran 5–7 mm inside In1 (the GND
plane). xtal_check.py was added so that **the seed selection loop automatically rejects any solution
that does not satisfy "both XTAL nets: 0 inner layer, ≤1 via"** (wired into try_thermal.ps1).

---

## 4.10 Record of the v4.1 revision (2026-08-14)

**The only changes are the Pico's orientation and the GPIO assignment. Component selection, values
and hole positions are completely unchanged.**

### Rotating the Pico 180° (same side)

Once J1 came to the front in v4, the Pico's USB connector faced 0.6 mm away from the 18.1 mm MQS
housing (the y=0 side) and the USB cable could not be plugged in. At the user's request it was
rotated 180° in plane without changing sides, pointing **USB toward the same y=61 edge as the
microSD and the LED**.

Why the rotation is cheap (two symmetries):

- The 2×20 header grid is 180°-rotationally symmetric, so **not a single hole moves** (verified
  against the v4 drill file for coordinate agreement). Only which pin goes into which hole changes,
  swapping diagonally (k ↔ k+20)
- The GND pins (3/8/13/18 ↔ 23/28/33/38) are GND at both symmetric positions, so the role of every
  GND barrel is unchanged too

### Reassigning the GPIO (a firmware change is required)

Because every pin position flips, the assignment was re-derived on the same basis as v1 — distance
plus the legality of RP2350 pin multiplexing. **Swapping the SPI0 ↔ SPI1 roles** is the key to being
both nearest-neighbor and legal at the post-rotation positions. The authoritative table is §3.7.
The gist: **SD=SPI0 (GP2–5), CAN=SPI1 (GP10–13), GPS=UART0 (GP0/1), and IG_SENSE=GP28 (ADC2) is
unchanged.**
`mrp_brd.py` asserts, on every build, which way the USB end faces, the 8 GND holes, and all 17
peripheral pins.

### The stitching-via moat rule (a permanent fix, as a by-product)

After the rotation, the 3V3 pin (pad 36, the only hole fed from the In2 plane) came into the
neighborhood of J2's GND pads, got surrounded on all sides by the 1 mm grid of GND stitching vias,
and **had its In2 thermal spokes blocked in every direction — a deterministic 1 unconnected in all
8 seeds**. It was resolved by adding a moat rule to gnd_stitch.py: **no stitch within 2.2 mm of a
non-GND through-hole that has a net**. It applies to every revision from then on.

### Verification results

- Freerouting routed everything; 0 DRC violations, 0 unconnected (seed 3, thermal gap 0.3 mm)
- compare_net 40/40 agree (the schematic follows automatically from the net definitions)
- XTAL1 10.2 mm (0 vias, 0 inner layer) / XTAL2 10.4 mm (1 via, 0 inner layer); C7 2.24 / C8 2.77 mm
  maintained
- In1 GND plane: 1 region, 100% (992.3 mm²); In2 3V3 plane: 1 region; 0 floating copper
- Power chain 14.83 mm, and every buck-loop distance identical to v4 (no component changes)
- 317 tracks / 186 vias (149 of them GND)

### Post-assembly acceptance checks (physical v4.1 board, 2026-08-14)

On first power-up, **5 V came out with IG open**. Measuring Q2's gate (pad 3) against GND gave 0 V.
The cause was **an unsoldered (floating) Q2 gate lead**. With a floating gate a PMOS drifts toward
the on state through leakage and capacitive coupling, and the meter's input resistance was pulling
the floating lead to 0 V as seen (the pad side is at V12P through R1). Resoldering fixed it.

This is an assembly defect the design cannot detect, so always check the following once a board is
built:

| Condition | Expected |
|---|---|
| IG open, V12_RAW=12 V | Q2 gate = source (≈ V12P, within 0.1 V), **V12_SW ≈ 0 V, no 5 V** |
| 12 V on IG | Q2 gate ≈ V12P/2, V12_SW ≈ V12P, 5 V present |
| IG switched off | 5 V drops 0.4–0.5 s later (C13's delay) |

If the gate reads all the way down at 0 V, suspect **a floating gate** first rather than a short
(even a completely shorted Q1 would stop at V12P/2 thanks to the R1/R2 divider, never 0 V).

---

## 4.11 Record of the v4.2 revision (2026-08-23)

**A revision driven by the hand-soldering experience with a physical v4.1 board. Parts, values, nets
and GPIO assignment are completely unchanged.**

### Moving the Pico to the back side

Flipping sides only mirrors the footprint about x=10.5, so **not a single hole moves**; the column of
pins 1–20 swaps to the left (x=1.61) and 21–40 to the right. The v4.1 GPIO assignment was chosen by
proximity in y (and both U2 and J2 are symmetric in x), so the distance relationships survive the
mirror and there is **no reassignment, hence no firmware change**. USB still faces the y=61 edge.
Only GPS (J3, on the back right at x15) is affected: GP0/1 moved to the left column and are now
about 13 mm away, which is a non-issue for three UART lines at 9600 bps — keeping the left routing
lane clear was preferred over moving J3.

The effect: everything on the front except J1 is now a low-profile part (U2, J2, and the 23 1608
chips), so **assembly, rework and probing can be done without removing the Pico**.
The power pocket, Y1 and J3 on the back fit under the Pico (socket height 8.5 mm).
**Soldering the GPS wires at J3 must be done with the Pico removed.**

### A hand-soldering land for Q2

The standard SOT-23-6 land of v4.1 (1.3×0.6 mm) had short pads and invited the unsoldered gate lead
(see the §4.10 acceptance checks). It was changed to **the same TSOT-23-6_HandSoldering land as U4**
(2.0×0.65 mm, 0.6 mm longer on the outside). That makes it 6 mm wide, which does not fit in the F1/D1
row, so it is stood on end at 90° in the right-hand column (S/G in the column toward D1, the drain
toward the board edge). The pad numbering stays in JEDEC order, so `PADMAP['SOT26']` and **the
per-build assertion of G=3 / S=4 remain in force**.

### A custom land for L1

The standard land for the ASPI-4030S (1.1×3.4 mm, outer edge 2.05) had almost no copper outside the
4×4 body and was also close to U4's SW pad. `mrp_fp.py` generates `L_4030_HandSolder` with
**each pad extended 1.0 mm outwards** (2.1×3.4 mm, outer edge 3.05), and it is **rotated 90°** so the
pads face ±y. The SW-side pad sits next to U4's SW column (4.1 mm) and the V5 side connects directly
to C15 below it (3.1 mm).

### Rearranging the back-side pocket

To make room for Q2 standing on end and L1 rotating, U4 moved 1 mm down, C14 went under Q2, C16 under
U4, C15 directly below L1's V5 pad, and the zones for D2, Q1 and D3 shifted 3–4 mm down. Every
placement is recorded in the EXPLICIT / ZONES tables of `mrp_brd.py`.

**R15 moved to the left end of the CAN termination band (x≈3.2, directly behind Q1).** In v4.1 it was
placed at x≈10–12.8, next to GP28 in the left column, but that is on the straight line from U2.OSC2
(10.8, 29.5) to Y1.2 (15.0, 32.6), and the first 8 seeds all failed with either XTAL2 diving into the
In1 plane or nets left unrouted. GP28 moved to the right column (x19.39) with the side flip, so there
was no longer any reason to keep R15 in the middle; it retreated to the left edge and freed up a
surface corridor for XTAL2 (IG_SENSE is a DC ADC line, so crossing the board once is harmless).

**The power stack at the left edge of the back side (L1, C15, D2, Q1, D3) is kept 2.5 mm from the
Pico's left pad column.** With the side flip, CAN's four SPI lines come out of **the left column**
(pads 14–17, y17–25) while U2's SPI pins are at **the right end** of its upper row (x≈13–14.7). On
the front the row of 1608 decoupling above U2 is a wall, and on the back the left-edge stack is a
vertical wall with only 0.5 mm of gap, leaving just 1.3 mm between it and the Pico pads —
**CAN_MISO was unrouted in 19 of 20 seeds**. Shifting the whole stack 1.2 mm to the right (L1's left
edge to x4.9, D2/Q1/D3's to x4.0) secured a 2.5 mm lane.

| Segment | v4.1 | **v4.2** |
|---|---|---|
| Power chain total (J1.8 → U4.VIN) | 14.83 mm | 15.12 mm |
| SW node (U4.SW ↔ L1) | 2.09 mm | 3.99 mm |
| Input capacitor (U4.VIN ↔ C14) | 3.04 mm | 2.90 mm |
| Output capacitor (L1 ↔ C15) | 4.10 mm | 3.07 mm |
| Bootstrap (U4.BST ↔ C16) | 4.00 mm | 4.03 mm |

The SW node grew by 2 mm, which is acceptable at 1.1 MHz and 0.2 A (with wider pads, solder joint
quality matters more).

### Reworking the schematic into a hand-placed layout (2026-08-26)

After it was pointed out as hard to read, schematic generation was replaced: from `gen_ksch3.py`
(one column per block with banded auto-routing) to **`gen_ksch4.py`, which places every component
and every wire at hand-written coordinates**. The conventions adopted are the standard ones: signals
left to right (J1 → power section → MCU → SD/GPS/LED), potential top to bottom (the 12V rail at the
top, GND symbols pointing down), the high-side switch drawn vertically, pull-ups and bypass
capacitors placed vertically, real wires within a block and labels between blocks, and the `#PWR`
reference names of power symbols hidden.
**Correctness of the connections does not depend on the layout**: an internal short/connection check
runs on every generation, and beyond that the kicad-cli netlist is compared with compare_net against
mrp_design.NETS for 40/40 agreement, and ERC is confirmed to show only the one known note about a U3
pin type. One A3 sheet.

### Verification results

- Freerouting routed everything; 0 DRC violations, 0 unconnected (seed 8 of 12, thermal gap 0.3 mm.
  The first 20 seeds all failed because of the R15 and left-edge-stack problems; it worked after the
  two fixes above)
- compare_net 40/40 agree; ERC shows only the one known note about a U3 pin type
- The coordinates of U3's 40 holes agree exactly with v4 and v4.1 (drill file comparison)
- XTAL1 9.7 mm (0 vias, 0 inner layer) / XTAL2 14.6 mm (**0 vias**, 0 inner layer); C7 2.24 /
  C8 2.77 mm maintained
- In1 GND plane: 1 region, 100% (972.2 mm²); In2 3V3 plane: 1 region (881.1 mm²); 0 floating copper
  (a pad-contact test was added to island_check — it had been falsely flagging a 0.3 mm² spoke
  fragment connected to U3's pad 3)
- Power chain 15.12 mm; SW 3.99 / input 2.90 / output 3.07 / BST 4.03 mm
- 328 tracks / 182 vias (137 of them GND). Inner-layer signal: In1 158 mm / In2 145 mm
  (up from 120 / 50 mm in v4.1. Moving the Pico to the back swapped the roles of the left and right
  columns, and the router used the inner layers for the extra difficulty. Both planes are still one
  region at 100%, undivided)
- Fabrication data: BOM 38 SMT + 3 THT (J1, U3, Y1), CPL 38, cross-check agrees.
  **The BOM is regenerated from the schematic every time** (fab_all.ps1). Hand-added rows (`Ver4.1`)
  are erased, so notes for JLC belong in a separate file

---

## 4.12 Record of the v5 revision (2026-10-16) — the JLCPCB-assembled version

**A variant of the generation system, `MRP_ASSY=jlc`** (the default `hand` is v4.2). The reasoning
and sources for the part selection are in [study-jlc-assembly.md](study-jlc-assembly.md).

### Configuration

| Item | hand (v4.2) | **jlc (v5)** |
|---|---|---|
| Chips | all 1608 | **still 1608 (0603)** (C13=0805/25 V, C15=0805, C14=1206/50 V). It was initially designed at 0402, but JLCPCB raised a **tombstoning warning for 0402 assembly on a HASL finish** and required ENIG (+$25), so it went back to 0603. With the holes gone there is enough room for 0603 to fit entirely on the F side as well, and it worked on the first seed. Side benefits: every 100 nF is a 50 V Basic part, and the LCSC numbers for the resistors are the ones already verified in §4.5. A 0402 version can still be generated with `MRP_CHIP=1005` |
| Q2 | ZXMP6A17E6Q (SOT-26, hand-soldering land) | **SI2309** (SOT-23, 1=G/2=S/3=D. LCSC C10493) |
| Y1 | HC-49/S THT, back side | **3225 SMD, front side** (X322516MLB4SI, Basic). C7/C8 = 12 pF, lid grounded |
| U4/L1 | extended hand-soldering lands | standard lands (TSOT-23-6 / ASPI-4030S) |
| Pico | socket on the back (40 THT holes) | **soldered directly by its castellations on the back** (custom `Pico2-SMD-Flush`, with pads stopping 0.3 mm inside the board edge. **Zero holes**) |
| GPS | J3 pads (back side) | **removed from the board**. The module's wires go straight to the Pico |
| D6 | back side center, y59.8 | **moved to x17.3** (to clear the Pico's USB connector; the emitting face stays at y=61) |
| Design rules | 0.15 mm | **0.127 mm** (with margin over JLC's 4-layer minimum of 0.09) |
| Assembly | all hand soldered | **JLC single-sided SMT, 37 parts**. Hand work = the Pico, D6 and J1 only |

### Design notes

- **Losing the through-holes was the biggest routing win**: the socket's 40 holes and Y1's 2 holes are
  gone, taking the routable width on the front from 15.2 mm to nearly the full width. It absorbed the
  13 parts that moved over and still got **DRC 0/0 on the first seed** (v4.2 only worked after 20
  seeds had failed)
- The Pico's SMD pads overhang the board edge by 0.79 mm in the standard KiCad footprint (which
  assumes a wider carrier), so `Pico2-SMD-Flush` was generated, stopping 0.3 mm inside the edge. The
  overlap with the module's underside lands is about 1.1 mm, and it can be hand soldered through the
  castellation slots
- Q2's new pad map (G=1/S=2/D=3) is **asserted on every build**, as v3.1 taught. The SI2309 is worse
  than the current part on Id/Rds (2 A/0.2 Ω) but has 30× the margin against the 0.3 A load. It is
  not AEC-Q101 qualified
- Moving Y1 to 3225 made XTAL as short as possible on the surface: **XTAL1 7.1 mm / XTAL2 13.8 mm,
  0 vias, 0 inner layer**, with C7/C8 at 1.84 mm each on the same side
- An **LCSC column** is added to the BOM automatically (verified numbers only; blanks are to be
  chosen at order time)

### Updating to Freerouting 2.5.0 (2026-10-17)

The routing engine was updated from 2.2.4 to **2.5.0 (the MSI/jpackage build, in
`E:\Claude\_tools\fr25msi`)** and the v5 board was rerouted. What the migration revealed:

- **The native CLI build (freerouting-cli) is unusable**: it crashes inside the autorouting loop with
  a GraalVM native-image serialization exception (BoardSnapshotManager). The MSI build is fine
- **`-oit` (cut off improvement) was removed.** The optimizer stops on its own criteria, so it is run
  with `-mp 20` (about 50 seconds per seed). The format of the result lines also changed, so
  try_thermal's parser was updated
- It now **fans out every SMD pad with a via**, which meant (1) the 0.095 mm neck-down at the pad
  escape conflicted with the old 0.127 minimum width → the jlc variant's minimum width constraint was
  relaxed to JLC's 4-layer manufacturing minimum of **0.09 mm** (normal routing width stays 0.127),
  (2) XTAL nets tend to pick up vias → xtal_check was adjusted to "0 inner layer strictly, ≤2 vias",
  and (3) the inner planes split more easily → **plane_gate.py was added** (rejecting any seed that
  leaves a fragment of 10 mm² or more in the In1 GND)
- Results for the seed adopted: DRC 0/0, **XTAL1 13.1 mm / XTAL2 5.3 mm (both 0 vias, 0 inner
  layer)**, In1 GND **1 region, 100% (1069.8 mm²)**, In2 1 region, 0 floating copper, compare_net
  37/37. 389 tracks / 237 vias (178 of them GND). Equivalent quality to 2.2.4's final result

### J4: adding solder pads for a direct connection (2026-10-18)

So that in future the **5 wires for power, IGN, GND and CANH/CANL can be soldered directly** to the
vehicle without going through the MQS connector, 5 bare pads were added in the 1.9 mm band between
J1 and the board edge (y=0) — in the jlc variant only.

- **Footprint** `PAD-5P-DIRECT` (custom, generated by mrp_fp.py): SMD pads 2.4×1.8 mm on a 4.0 mm
  pitch. Silkscreen letters L / H / I / G / V (0.7 mm) are marked next to each pad
- **Side and order**: the Pico mounting side (the B side). Seen from the back, left to right:
  **L, H, I, G, V** (= CANL, CANH, IGN, GND, 12V). **That is exactly the same left-to-right order as
  the corresponding pins of J1**, so the five connections just drop straight down with no crossings
  and no vias (the original V..L order was reversed because every net crossed the board and needed
  vias). Center x=10.0 (0.5 mm left of the board center), giving 0.59 mm clearance between the end
  silkscreen letters and the edge
- **Connections**: V connects **upstream of F1 (V12_RAW, the same node as J1.8)**, so the PPTC still
  protects a directly wired supply. I=IG_RAW, G=GND, H=CANH and L=CANL are in parallel with the
  corresponding J1 pins
- **The traces are written directly by script (0 vias)**: dsn_export hides J4 from the router (and
  also puts a keepout in the y<2.8 band on the B side), and after importing the session ses_import
  draws the five traces as straight plus 45° segments, **0.4 mm wide on the B side, from J1's
  through-hole barrels**. The rear-row pins (CANL/CANH/12V) drop straight down between the front-row
  barrels to y=2.8 and enter their pads diagonally. The front-row pins (IGN/GND) connect with a
  single diagonal segment
- **Excluded from the BOM/CPL**: being copper only with no physical part, the schematic symbol is
  marked `in_bom no` and the footprint carries the "exclude from position files" attribute, so it
  appears in none of the BOM, the CPL or the hand-soldering list (managed by the `NO_BOM` set in
  mrp_design)
- Verification (2026-10-18, seed 6 / gap 0.3): compare_net 37/37, only the one known ERC note,
  **DRC 0/0**, the five J4 traces 0.4 mm wide on the B side with **0 vias** straight from J1's
  barrels, XTAL1 14.6 mm / XTAL2 5.3 mm (0 vias, 0 inner layer), In1 GND 1052 mm² (the only fragment
  is 1.8 mm²), 0 floating copper.
  Alongside this, drc_detail.py was added to print the breakdown of DRC violations when a seed is
  rejected (the recurring violations=2 turned out to be a neck-down where the router thins to
  0.076 mm around y20.5 — a seed-dependent phenomenon unrelated to J4)

### Verification results (the first v5)

- Seed 1 / gap 0.3 gives **0 DRC violations and 0 unconnected** (both the 0402 and the 0603 version).
  compare_net **37/37** agree; ERC shows only the one known note about a U3 pin type
- In1 GND plane: 1 region, 100% (1139.6 mm²); In2 3V3: 1 region (1026.9 mm²); 0 floating copper
- Power chain 14.9 mm; buck loop SW 4.2 / input 2.3 / output 5.0 / BST 3.9 mm
- Fabrication data: BOM 39 SMT + 1 THT (J1), CPL-top 37 parts, cross-check agrees

---

## 5. Outstanding issues

| Item | Details |
|---|---|
| Vehicle pin 3 | Confirm against the bike's own schematic that a 98 µA load is acceptable |
| CAN termination | R8's 120 Ω is always on. Inserting into an already terminated bus risks communication failure (see "CAN termination is always on" above) |
| Negative surges | D1's (B5819W) reverse rating is 40 V. D2 is behind D1 and never sees a negative surge on the input side. To cover something like ISO 7637-2, D1 needs a higher rating, or D2 needs to be bidirectional and moved ahead of it |

**Decided** (items knowingly left as they are)

| Item | Decision |
|---|---|
| **F1's 16 V rating** | For a 1206 PPTC with 1.1 A hold, **16 V is the ceiling** (the others are 6 V and 8 V). On top of that, F1 is **upstream of D2 (the TVS)** (J1.8 → F1 → D1 → D2), making it the only part that takes vehicle-side surges without them being clamped. A 24 V/30 V part would need a larger size such as 1812 and therefore a board change, so **by the user's decision it stays at 16 V** (2026-07-31). Littelfuse **1206L110/16WR** is the intended part |

**Resolved** (formerly outstanding)

| Item | Result |
|---|---|
| **J3's silkscreen T / R** | **Resolved in v4.** The silkscreen was changed to the module's point of view (pad 3 = R / pad 4 = T, ordered V G R T P) so the wire labels match when read directly (see §3.5). The v1 board is from the board's point of view, so continue to watch out there |
| **GPS wire colors** | Determined on hardware (2026-08-14). **Red=VCC / black=GND / orange=TXD / green=RXD / brown=PPS**. For the assignment to J3, see the table in §3.5 |
| **GPS baud rate** | Determined on hardware (2026-08-14). **9600 bps**, exactly the firmware default |
| Voltage drop during cranking | **Resolved in v4.** The AP63205's minimum input is 3.8 V. Even subtracting about 0.4 V for F1+D1, the 5 V rail keeps running unless the battery drops to 4.2 V (below about 5.6 V the output follows the input) |
| The role of DM3AT's auxiliary pads | Settled by the official KiCad footprint and symbol. The detect switch is TL and L2 (pads 9 and 10 officially) |
| DM3AT's keep-out areas | The official KiCad footprint carries all 5 keep-out areas, verified by DRC |
| Enclosure | As of v4.2: the front has only J1 (18.1 mm) — U2, J2 and the 1608 chips are exposed; the back has the Pico (8.5 mm socket plus the module, with USB at the y=61 edge) and the parts underneath it (3 mm tallest). Openings are concentrated on the y=61 end face, with SD (front), USB (back) and the LED; the y=0 end has only MQS |

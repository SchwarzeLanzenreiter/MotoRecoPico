# MotoRecoPico hardware design specification

A Raspberry Pi Pico 2 based CAN logger, carried on a motorcycle.

- Scope: vehicle 12V supply only, direct MQS 8-way connector, MicroSD mounted on the board
- Board: 4 layers, 21 × 61 mm. Chip components are 1608 (assembled by JLCPCB)

> **The details of the circuit (bill of materials, netlist, power architecture, pin assignment) are
> defined canonically in [netlist.md](netlist.md).** This document covers only the derivations from
> the primary connector and footprint sources, plus the board layout.
>
> The descriptions of the LM2596, the GPS LDO, the latching supply, the red LED and the GPS
> through-holes that appeared in older revisions have been invalidated by design changes and
> removed.

---

## 1. Component selection (settled)

| Part | Type number | Package | Notes |
|---|---|---|---|
| MCU | Raspberry Pi Pico 2 (RP2350) | 2×20 2.54 mm header | Socketed so it can be replaced |
| CAN | MCP25625T-E/SS | SSOP-28 (0.65 mm) | Controller and transceiver in one |
| GPS | GT-502MGG-N | External (5 bare wires, 1.5 m) | Soldered directly to single-sided SMD pads |
| Storage | Hirose DM3AT-SF-PEJM5 | SMD, right angle | Push-push, with card detect |
| Vehicle I/F | TE 1-967658-1 | MQS .63, 8 way, 2 rows, 4 mm, right-angle TH | Mates with the vehicle connector |
| Step-down | **Diodes AP63205WU-7** (v4) | TSOT-26 plus 4 L/C parts | 3.8–32 V in / 5 V fixed, 2 A, synchronous. The tallest part is L1 at 3 mm |
| High-side switch | **Diodes ZXMP6A17E6Q** | SOT-26 | −60 V / −3 A / AEC-Q101 |

### MQS connector pin assignment (specified by the user)

| Pin | Signal |
|---|---|
| 3 | 12V when IG is on |
| 4 | GND |
| 5 | CAN Low |
| 6 | CAN High |
| 8 | Permanent 12V |

Unused: 1, 2, 7

### MQS connector footprint (derived from drawing 967658 Rev.C3)

The origin is the center mark on the connector's front reference line. +X is to the right, +Y is
toward the board edge (the mating direction). The holes go in the −Y direction from the reference
line (into the board).

| Pin | X [mm] | Y [mm] | Row |
|---|---|---|---|
| 1 | +7.0 | −3.76 | Front row (lower contacts) |
| 2 | +3.0 | −3.76 | Front row |
| 3 | −1.0 | −3.76 | Front row |
| 4 | −5.0 | −3.76 | Front row |
| 5 | +5.0 | −6.30 | Rear row (upper contacts) |
| 6 | +1.0 | −6.30 | Rear row |
| 7 | −3.0 | −6.30 | Rear row |
| 8 | −7.0 | −6.30 | Rear row |

| Item | Value |
|---|---|
| Hole diameter | ⌀1.0 mm or more (8 places) → 1.1 mm drill, ⌀1.8 mm pad recommended |
| Pitch within a row | 4.0 ±0.1 mm |
| Pitch between rows | 2.54 ±0.1 mm |
| Horizontal offset between rows | 2.0 mm (staggered) |
| Reference line to front row | 3.76 mm (= 6.3 − 2.54) |
| Reference line to rear row | 6.30 −0.1 mm |
| Housing width | 19.6 mm max |
| Housing overall height | 18.1 ±0.15 mm |
| Tail protrusion | 3 ±0.25 mm |
| Positional tolerance | ⌀0.3 (8 places) |

**How the pin numbering was derived**: looking at the mating face, the lower row is 1·2·3·4 from the
left and the upper row is 5·6·7·8 from the left. From cross-sections G-M, the tails of the upper
contacts come down on the side away from the mating face (the rear row) and the lower ones on the
near side (the front row) — necessarily so, since otherwise the vertical part of an upper tail would
collide with the horizontal part of a lower contact. Left and right as seen from the mating face are
mirrored relative to the top view of the board, which gives the X coordinates in the table above.

Materials: housing PBT-GF20 (Ultradur B4300-G4), black / contacts CuFe2, tin-plated.
Mating housing: AMP 1534229 (8 way). Product specification: 108-18568.

Note 7 of the drawing says "this area is a full-perimeter sealing surface (Ra 0.7 µm)", so
**it does have a sealing structure when mated with the matching housing**. The earlier statement
that "MQS is not waterproof" was wrong. A separate seal is still needed where it passes through a
case.

---

## 6.1 Board layout

Outline **21 × 61 mm (12.8 cm²)**, **4 layers**. The design data is the KiCad 10 project in
`cad/kicad/`.

| Side | Components |
|---|---|
| Front | 26 parts. **J1 (moved in v4)**, U2, J2, and all 23 of the 1608 chips. **In v4.2 the Pico moved to the back, so everything except J1 is low-profile and exposed** |
| Back (Pico side) | 15 parts. **U3 (the Pico socket, v4.2)**, Y1, J3, the buck set (U4/L1/C14-C16), and the SOT/SOD/SMA parts (**no 1608 chips**). The tallest part under the Pico is 3 mm |

### Stackup

| Layer | Use | Actual routed length |
|---|---|---|
| F.Cu | Signal | 227 mm |
| In1.Cu | **GND plane** | 158 mm |
| In2.Cu | **3V3 plane** | 145 mm |
| B.Cu | Signal | 196 mm |

**Two layers were tried twice, and rejected twice.** In v1 it needed 0.127 mm (the process minimum)
and still left the GND pour split into 8 pieces with 7 islands. It was tried again in v2 when three
components were removed; the routing completed, but **2 to 6 GND pads were left without a thermal
spoke reaching them** (across all combinations of 5 seeds × 3 gap values), which was incompatible
with thermal relief, the whole point of v2. Four layers works at 0.15 mm with every spoke reaching.
The board cost goes from $2.10 to $7.10 for five pieces (+$5.00, or +17% on the total including
shipping).

**There is signal on the inner layers too.** The router was given the inner layers as planes, but
Freerouting routes over them as needed. Even so the GND copper on In1 is **one region covering 100%
of it** (v4.2: 972.2 mm², zero splits; the 3V3 on In2 is likewise one region of 881.1 mm²), so as a
plane it is not divided. 0.15 mm traces only work at this routing density, so inner-layer use is
tolerated as long as it does not split the plane.

**The outer pours are filled after routing.** Filling GND into F.Cu / B.Cu before routing left
Freerouting unable to use the signal layers fully, and nets remained unrouted. GND is carried by the
inner planes, so the outer layers are only poured once routing is done.

**All 23 of the 1608 chips are collected on the front side** (v3, so that assembly and rework are
confined to one side). Anything larger than that is split across both sides.

### Why the outline dimensions are what they are

**The 61 mm length is close to the geometric minimum.** The MQS holes are only 1.89 mm away in X
from the Pico's header hole columns, which with ⌀1.8 mm pads leaves a 0.09 mm gap — not
manufacturable. So MQS needs a band of its own outside the header grid.

The MQS housing overhangs the board by 8.0 mm. At the user's request the clearance to the header is
**0.57 mm**. Since the Pico socket moved to the back in v4.2, the MQS tails (soldered on the back)
and the socket body come within 0.1 mm of each other on the same side. **The assembly order —
solder MQS first, then fit the socket — is a prerequisite.**

**The 21 mm width is the Pico's footprint width itself.** The header hole columns are at x=1.61 and
19.39, so the usable area is only the 15.2 mm between them. The side-view LED and the GPS pads
cannot go on the long edges, so the LED went to a short edge (y=61) and the GPS pads to the middle of
the back side.

| Edge | What is on it |
|---|---|
| Short edge y=0 | J1 (**front side** as of v4, with the mating opening facing off the board. The hole pattern is the mirror of v3.1) |
| Short edge y=61 | J2 MicroSD (front, with the slot about 1 mm inside the board edge), D6 side-view LED (back), **the Pico's USB port (moved to the y=61 edge in v4.1; on the back side at socket height +8.5 mm in v4.2)** |

**Because the LED, the card slot and USB are all on the same short edge, the case openings can all be
on that one end face.** The y=0 end has only the MQS connector.

**The Pico is mounted on the back side (v4.2).** Flipping sides mirrors the footprint in x, so not a
single hole moves; the column of pins 1–20 just swaps to the left (x=1.61) and 21–40 to the right.
The v4.1 GPIO assignment was chosen by proximity in y and is preserved under the mirror, so
**there was no reassignment and no firmware change**. The aim is to have everything on the front
except J1 be low-profile (U2, J2, the 1608 chips), so that assembly, rework and probing can be done
without removing the Pico. The power pocket, Y1 and J3 on the back fit under the Pico (socket height
8.5 mm, tallest part 3 mm). **The GPS wiring at J3 is soldered with the Pico removed.**

**The Pico is rotated 180° (v4.1).** Once J1 came to the front in v4, the USB connector faced 0.6 mm
away from the 18.1 mm MQS housing and the cable physically could not be plugged in. The 2×20 header
grid is 180°-rotationally symmetric, so **not a single hole moved**; only which pin goes into which
hole changed, swapping diagonally (k ↔ k+20). The 8 GND pins are GND at both symmetric positions, so
their role as barrels is unchanged too. But the physical positions of all the signal pins flip, so
**the GPIO assignment was re-derived from the post-rotation distances** (the key is the swap of
SPI0 ↔ SPI1 roles; the authoritative table is netlist.md §3.7, and the firmware pin definitions had
to change).

### Design requirements honored in the placement

**U2 is rotated 90°.** Before the rotation, the two rows of the SSOP-28 faced the long edges and all
28 pins were trying to escape into a 3.1 mm wide side corridor. Physically impossible.

**U2's decoupling sits to the left and right of the IC, outside the escape area.** Putting it above
and below would block the fan-out of 28 pins. The chips are stood on end at 90° to fit in the side
gaps.

**The two large parts on the back (Y1 and J3) are rotated 90° and stacked vertically on the right.**
Laid flat they are about 12 mm wide and would cross the 15.2 mm band, splitting the back side so
that nothing could pass vertically.

**Y1 (the crystal) is on the back.** The HC-49/S is a through-hole part, so the side it is inserted
from is the side the body sits on. Up to v4.1 the Pico occupied the front, so it was inserted from
the back. In v4.2 the Pico moved to the back too, but the HC-49/S height (3.5–4.7 mm) fits under the
8.5 mm socket height, so it stayed. It has pads on both sides, so no via is needed.

**C7/C8 (the crystal load capacitors) are explicitly placed on the front, flanking Y1's two barrels
(v3).** Y1's through-holes carry the XTAL nets up to the front, so the capacitors can connect
straight to the barrels without a via. In v2 they were left to the left-edge zone and ended up 11.1 /
9.0 mm away, with XTAL1 running 8.5 mm inside the In1 plane. Measured in v4.2: C7 2.24 mm / C8
2.77 mm, XTAL1 9.7 mm (0 vias, 0 inner layer) / XTAL2 14.6 mm (0 vias, 0 inner layer). Left to the
zone, spill from other zones steals the slot, so they are pinned with EXPLICIT. C8's rotation is
270° (at 90° its pin 1 would face the far side).

**The power components are in a single row following the flow of current.** J1 → F1 → D1 → Q2 →
U4 (buck), laid out on the back in the order that connects naturally from the position of the supply
pins. **In v4 the whole row was mirrored left-right when J1 flipped sides** (the 12V entry moved from
x17.5 to x3.5, so it goes F1 → D1 → Q2 from left to right, with the buck directly below).

| Segment | Pin-to-pin distance |
|---|---|
| V12_RAW (J1.8 → F1.1) | 3.47 mm |
| V12_FUSED (F1.2 → D1.2) | 1.85 mm |
| V12P (D1.1 → Q2.4) | 3.40 mm (v4.2, with Q2 stood on end) |
| V12_SW (Q2 → U4.VIN) | 6.40 mm |
| **Total** | **15.12 mm** (v4.2; v4 was 14.83 mm) |
| SW node (U4.SW → L1) | 3.99 mm (v4.2, L1 rotated; v4 was 2.09 mm) |
| Input capacitor (U4.VIN ↔ C14) | 2.90 mm |
| Output capacitor (L1 ↔ C15) | 3.07 mm |
| Bootstrap (U4.BST ↔ C16) | 4.03 mm |

Before the review the same path was **36.1 mm**. The 12V came in at J1's right edge (x=17.5) but F1
was 17 mm below in the middle, from where the path drew a U-shape to the left edge, up, and back to
the right (17.4 mm in the first segment alone). At about 100 mA DC this is not a functional problem,
but it was fixed to tighten the input current loop and to keep the layout legible.

**Q2 is placed on the same hand-soldering land as U4, stood on end at 90° (v4.2).** The standard
SOT-23-6 land of v4.1 had short pads and an unsoldered gate lead actually happened on a physical
board. Making the pads 2.0×0.65 mm takes the width to 6 mm, which does not fit in the F1/D1 row, so
it is stood up in the right-hand column with S/G facing the D1 side and the drain facing the board
edge. The pad numbering remains in JEDEC order, so the per-build assertion of G=3 / S=4 still holds.

**L1 uses a custom land with pads extended 1 mm outwards, rotated 90° (v4.2).** The standard land has
no copper outside the 4×4 body for an iron tip to rest on, and it was also close to U4's SW pad. The
extended land (`L_4030_HandSolder`) is placed facing ±y, with the SW-side pad next to U4's SW column
(4.0 mm) and the V5 side connected to C15 directly below (3.1 mm).

**The power stack at the left edge of the back side is kept 2.5 mm away from the Pico's left pad
column (v4.2).** With the side flip, CAN's four SPI lines come out of the left column and head for
the SPI pins at the right end of U2's upper row, so with the earlier packing that left only 1.3 mm at
the left edge, CAN_MISO was unrouted in 19 out of 20 seeds. L1's left edge is now at x4.9, and
D2/Q1/D3's at x4.0.

**The buck set (U4, L1, C14–C16) sits in the pocket on the back that U1 vacated (y13–21), as of
v4.** In v4, U4 and L1 were explicitly placed in a rotation that puts their SW pads facing each
other (the other way round stretches the SW node to 8.5 mm). C14 (the 10 µF/50 V input) is just to
the right of U4's VIN column — the first attempt put it inside U4's hand-soldering courtyard and DRC
flagged a short, so the whole block was pulled 2.4 mm to the left to make room. In v4.2, to go with
Q2 standing on end and L1 rotating, U4 moved 1 mm down, C14 went under Q2, C16 under U4, and C15
directly below L1's V5 pad (all pinned with EXPLICIT in `mrp_brd.py`).

**C2 has a zone of its own on the right flank.** Back when the chips were still 1005, C2 alone was
raised to 1608 for voltage rating reasons. The auto-placer packs by descending area, so C2 then took
the left end of the voltage-divider resistors' zone and **pushed R1–R4 2.75 mm to the right**. That
alone cost SD_CD its corridor past J2, and one net stayed unrouted under every router setting
(6 combinations of `-oit` and `-mp`, all with one left). Giving C2 a dedicated zone and putting the
divider row back where it was produced zero unrouted nets on the first try.

The chips later all became 1608 and C2 is no longer a special size, but **the zone is kept because it
is a verified placement**.

**R8 (the 120 Ω CAN termination) is on the south side of U2, directly below the pads where CANH/CANL
come out.** It was originally on U2's west side (x 2.9–5.6), where it blocked the only corridor from
U2's northwest corner around to the Pico's left column, and CAN_RESET could not be routed at all.
This is where the part belongs, and moving it took the unrouted count to zero.

### Verified items

| Item | Result |
|---|---|
| Component placement | 41 parts, 26 front / 15 back (v4.2: the Pico is on the back) |
| Chip parts (1608) | All 23 on the front (the automatic check was extended to C1608) |
| Overlapping parts on the same side | None |
| A body sitting over another part's hole | None (automatic check) |
| Protrusion beyond the board outline | None (only J1 crosses the y=0 edge, by design) |
| Net agreement with the schematic | All 40 nets agree |
| Routing | Complete. 328 tracks / 182 vias (137 of them GND). Zero inner-layer XTAL is guaranteed automatically by the seed selection (xtal_check) |
| DRC violations | 0 |
| Unconnected pads | 0 |
| Floating copper (pour regions with no via and no pad) | 0 (all 6 regions measured) |
| Power chain length | 14.83 mm (36.1 mm in the first v1) |

**Every GND pad uses thermal relief (v2).** Spoke 0.4 mm / gap 0.3 mm / minimum spoke count 1. The
direct pour connection of v1 sank far too much heat during hand soldering, and was judged
unacceptable on a physical board, so it was changed. That the spokes reach every pad is guaranteed by
DRC reporting zero unconnected.

Routing is done with Freerouting. GND is poured on three layers (F.Cu / In1.Cu / B.Cu) and the four
layers are tied together with stitching vias. Stitching vias are checked before placement to satisfy
not only the clearance to every pad and via but also **a 0.25 mm drill-to-drill spacing** (clearance
rules disappear between same-net objects, but **the drill spacing rule remains** — missing this once
led to GND vias being excluded from the obstacle set and eating into the holes of routing vias).

**Stitching vias keep a 2.2 mm moat from the through-holes of other nets (v4.1).** A through via
opens a clearance void in both inner planes, so when a 1 mm grid of vias surrounds another net's PTH
pad it can block, in every direction, the thermal spokes that pad should receive from the plane. With
the Pico rotation in v4.1 the 3V3 pin (pad 36, fed from the In2 plane) came into the neighborhood of
J2's GND pads, got caged in by vias, and produced a deterministic **1 unconnected in all 8 seeds**.
Excluding 2.2 mm around any non-GND PTH pad that has a net always leaves a ring of fill at least
0.5 mm wide outside the pad's thermal void, so a spoke can form.

The fabrication data is in `cad/kicad/fab/`. Gerbers are output for all four layers (`.gtl` / `.g1` /
`.g2` / `.gbl`) and collected in `MotoRecoPico-gerber.zip`. The drills are split into PTH / NPTH.

---

## 7. References

- [MCP25625 datasheet (DS20005282B)](https://akizukidenshi.com/goodsaffix/mcp25625.pdf) — pinout
  Table 1-1 (p.4), reference circuit Figure 1-2 (p.5)
- [GT-502MGG datasheet](https://akizukidenshi.com/goodsaffix/GT-502MGG.pdf) — pin descriptions
  section 2.1 (p.7), DC characteristics section 1.3 (p.6)
- [DM3AT-SF-PEJM5 2D drawing ADC-325165-00-01](https://www.hirose.com/ja/product/p/CL0609-0031-0-00)
  — recommended board pattern (mounting-side view)
- [M78AR05-0.5 datasheet](https://akizukidenshi.com/goodsaffix/m78a.pdf) — mechanical dimensions
  p.3, Notes 6/7 p.2
- [LP812-010(T) datasheet](https://akizukidenshi.com/goodsaffix/lp812-010(t).pdf) — recommended
  solder pattern p.3
- [Raspberry Pi Pico 2 datasheet](https://datasheets.raspberrypi.com/pico/pico-2-datasheet.pdf) —
  3V3(OUT) load limit, how to supply VSYS
- [TE 1-967658-1 product page](https://www.te.com/en/product-1-967658-1.html)
- [ZXMP6A17E6Q datasheet](https://www.diodes.com/datasheet/download/ZXMP6A17E6Q.pdf)

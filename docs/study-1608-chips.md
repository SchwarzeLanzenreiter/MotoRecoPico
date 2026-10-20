# Study: moving the chip components from 1005 to 1608

> **Adopted on 2026-07-31.** This document exists to record the reasoning behind the decision.
> The current design has all 21 chips at 1608. The default in `mrp_design.py` is `CHIP='1608'`.

The design at the time mounted its 20 chips (13 resistors, 7 capacitors) as **1005 (0402 inch)**.
Whether **1608 (0603 inch)** would still work was checked by actually running the pipeline.

**Conclusion: it works.** The board stays 21 × 61 mm, with everything placed, everything routed,
zero DRC violations and zero unconnected. The decoupling distances actually get shorter.

## Measured comparison

| Item | 1005 (then current) | **1608** |
|---|---|---|
| Chip count | 20 | 20 |
| Keepout per part | 1.91 × 0.99 mm | 3.01 × 1.51 mm |
| Total occupied area | 37.8 mm² | **91.0 mm²** (+53.2) |
| Board size | 21 × 61 mm | **21 × 61 mm** (unchanged) |
| Everything placed | yes | **yes** |
| Unrouted | 0 | **0** (on the first try) |
| DRC violations / unconnected | 0 / 0 | **0 / 0** |
| In1.Cu GND plane | 1 region, 100% | 1 region, 100% |
| Routed length | 694 mm | 664 mm |
| Vias | 233 | 248 |
| Parts outside their zone | 2 | 9 |

## Decoupling distances (pad to pad, same net)

**Even though the area went up, this got better.** That is a result of the auto-placer repacking,
not a systematic advantage of 1608 — but at the very least it does not get worse.

| Capacitor | Purpose | 1005 | **1608** |
|---|---|---|---|
| C3 | MCP25625 VIO | 8.88 mm | 8.69 mm |
| C4 | MCP25625 VDD | 5.80 mm | 4.45 mm |
| C5 | MCP25625 VDDA | 11.68 mm | **5.66 mm** |
| C6 | VDDA bulk (back side) | 11.52 mm | 11.52 mm |
| C10 | microSD VDD | 8.18 mm | 6.50 mm |
| C11 | microSD bulk (back side) | 4.55 mm | 4.55 mm |
| C7 | crystal load (Y1, back side) | 11.40 mm | 11.12 mm |
| C8 | crystal load (Y1, back side) | 9.90 mm | 8.96 mm |
| C12 | ADC filter | 20.37 mm | **14.52 mm** |
| C2 | V12P bypass | 8.74 mm | 8.74 mm |
| **Total** | | **101.01 mm** | **84.72 mm** |

The "parts outside their zone" figure does go from 2 to 9, but that is merely an internal metric for
how far a part strayed from the rectangle the auto-placer defined; the actual electrical distances
improve as the table above shows. **That metric is misleading and should not be used to decide
anything.**

## Effect on JLCPCB part categories

**Moving to 1608 improves the Basic-part situation.**

| Value | Basic at 1005 | Basic at 1608 |
|---|---|---|
| 100 kΩ | C25741 | C25803 |
| 22 kΩ | C25768 | C31850 |
| 10 kΩ | C25744 | C25804 |
| 120 Ω | C25079 | C22787 |
| 1 kΩ | C11702 | C21190 |
| 22 pF | C1555 (50 V) | C1653 (50 V) |
| **100 nF** | C1525 (**16 V only**) | C14663 (**50 V**) |

Every value is available as Basic at 1608 too. And because **100 nF becomes 50 V**, the special case
of raising C2 alone to 1608 (`PKG_OVERRIDE` in `mrp_design.py`) is no longer needed.

## Ease of hand assembly

1608 is 1.6 × 0.8 mm, 1005 is 1.0 × 0.5 mm — 2.4× the area and 1.6× the edge length.
**That is a large difference in how easy hand soldering and rework are.** If JLCPCB does the
assembly it makes no difference, but if there is any chance of fixing it yourself, 1608 wins.

## How to do it

`mrp_design.py` has an environment-variable switch for this.

```
MRP_CHIP=1608  ->  R_0603_1608Metric / C_0603_1608Metric (default)
MRP_CHIP=1005  ->  R_0402_1005Metric / C_0402_1005Metric (for comparison)
```

**Accompanying changes, already made**

- `PKG_OVERRIDE` for C2 in `mrp_design.py` now only takes effect when `CHIP=='1005'`
  (at 1608 every 100 nF is 50 V, so it is unnecessary)
- The text of `netlist.md` §1.3 / §1.4 / §4.5 and of `hardware-design.md` was updated to 1608
- `eagle_pkg(r) == 'C1005'` in `mrp_brd.py` refers to **the package name on the EAGLE side** and
  stays as it is. Only the KiCad footprint changes to 1608, so the "all chips on the front" check
  still works

## Measured values after adoption (the final 1608 board)

| Item | Value |
|---|---|
| Unrouted | 0 (on the first try) |
| DRC violations / unconnected | 0 / 0 |
| Routing | 329 tracks / 251 vias (207 of them GND) |
| Routed length per layer | F.Cu 232 / In1 158 / In2 108 / B.Cu 170 mm |
| In1.Cu GND plane | 1 region covering 100% (992.3 mm²; the runner-up is 3.0 mm²) |
| Floating copper | 0 (all 6 regions) |
| Net agreement with the schematic | 38/38 |
| Power chain length | 13.81 mm (unchanged) |

---

## A problem discovered during the study: the routing result is not reproducible

Rebuilding the board with `mrp_brd.py` **changes the order of the pins within a net**. KiCad's DSN
export writes out the internal order of the pads as-is, and that order depends on the UUIDs, which
are regenerated every time.

Two DSN files produced from an identical placement (verified: all 38 parts matching in coordinates,
rotation and side) were the same size, 58,074 bytes, but 24 lines differed — and what differed was
only the pin ordering, such as `(pins U3-19 R13-1 J2-9)`.

**That difference changes Freerouting's result.** The verified board had zero unrouted on the first
try, but a rebuild of the same placement left SD_CS unrouted twice, four runs in a row.

The countermeasure is to **keep the routed board file itself as the deliverable**. Do not assume that
rebuilding from `mrp_brd.py` will give the same result. Even during this study it was overwritten
once and had to be recovered from a backup (`MotoRecoPico_backup_20260731-1400`).

---

*Measured 2026-07-31. The shipping data in use at the time was left at 1005.*

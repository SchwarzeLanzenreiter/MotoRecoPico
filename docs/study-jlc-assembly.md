# Study: the JLCPCB-assembled version (the v5 proposal) — consolidating on the F side and shrinking

2026-10-16. Starting from v4.2 (the finished hand-soldered version,
`MotoRecoPico_backup_20261016-0643_v4.2-handbuild-final`), this studies a configuration that
**sends the F side out for single-sided SMT assembly at JLCPCB**. The board data is unchanged.

## Direction (specified by the user)

1. Where a smaller part exists for an existing F-side component, use it and increase the density
2. Move as many parts as possible from the B side to the F side, making them smaller where possible
3. Parts that stay on the B side keep their current type numbers
4. F-side parts must be purchasable at JLCPCB. Replace them with cheaper equivalents where available

Standardizing on 1608 was a constraint of "ease of building" for the hand-soldered version, so it is
dropped and the chips go down to 0402 (1005). v1 was designed and successfully routed with all chips
at 1005 (`MRP_CHIP=1005` still exists in the generation system), so there is a track record both
electrically and in terms of routing.

## 1. Shrinking the existing F-side parts

| Part | Current | Proposed change | JLC part | Category |
|---|---|---|---|---|
| R1–R4, R6–R8, R10–R15 (13 parts) | 1608 | **0402** | The Basic series (every value is available) | Basic |
| C3, C4, C5, C10 (100 nF, the 3.3V/5V rails) | 1608/50 V | **0402 / 16 V** (plenty against the rail voltage) | C1525 | Basic |
| C2 (100 nF, V12P permanent 12V) | 1608/50 V | **No change** (the 0402 Basic part is 16 V only; 50 V is mandatory) | C14663 | Basic |
| C6, C11 (10 µF bulk) | 1608 | No change (10 µF at 0402 is not practical) | C19702 | Basic |
| C13 (10 µF/25 V, Q2 gate delay) | 1608/25 V | **No change recommended.** JLC's 0603 10 µF Basic part very likely stops at 16 V, and a 25 V part either **gets bigger** at 0805 or is Extended. Check 0603/25 V stock when ordering and fall back to an 0805 Basic part (the CL21A106KAYNNNE family) if there is none | To be confirmed | — |
| C7, C8 (22 pF crystal load) | 1608 | **0402, value changed to 12 pF** (because moving Y1 to 3225 makes CL=9 pF) | 0402 NP0 Basic | Basic |
| U2 (MCP25625-E/SS) | SSOP-28 | No change | C184944 | Extended |
| J2 (DM3AT-SF-PEJM5) | — | No change | In stock (Extended) | Extended |

## 2. Moving parts from the B side to the F side

### Moving (purchasable at JLC)

| Part | Current (B side, hand-soldering land) | Proposed move to the F side | JLC part | Category |
|---|---|---|---|---|
| U4 AP63205WU-7 | TSOT-26 hand-soldering land | **The standard TSOT-26 land** (no extension needed for machine assembly) | **C2071056** (24k in stock, about $0.44) | Extended |
| L1 6.8 µH 4030 | Custom extended land | **The standard 4030 land.** The part becomes Sunlord **SWPA4030S6R8MT** (Isat 2.75 A ✓, same dimensions and land as the ASPI-4030S) | **C62684** | Extended |
| C14 10 µF/50 V | 3216 | Stays 3216, moves to the F side (a 50 V 10 µF does not get smaller than this) | A 1206/50 V part to be chosen at order time | Assumed Extended |
| C15 22 µF/16 V | 2012 | Stays 2012, moves to the F side (there is an 0805 22 µF in the Basic series) | To be confirmed | Assumed Basic |
| C16 100 nF (BST) | 2012/50 V | **0402 / 16 V** (the potential difference across BST–SW is about 5.3 V) | C1525 | Basic |
| D1, D3 B5819W | SOD-123 | No change, moves to the F side | C8598 | Basic |
| D2 SMAJ16A | SMA | No change, moves to the F side | Diodes **C134959** / Littelfuse C74561 (the genuine parts are recommended; equivalents such as C908385 are about $0.02) | Extended |
| F1 PPTC 1.1 A/16 V | 3216 | No change, moves to the F side. Littelfuse 1206L110/16WR = **C3761685** ($0.15). The equivalent LUTE 16NR = C7542961 ($0.03, but slow at a 300 ms trip) → **the genuine part is recommended** | C3761685 | Assumed Extended |
| Q1 MMBT3904 | SOT-23 | No change, moves to the F side | C20526 | Basic |
| **Y1 16 MHz (THT HC-49S)** | B side, THT | **Replaced by a 3225 SMD part and moved to the F side**: YXC **X322516MLB4SI** (CL=9 pF, ±10 ppm). Two through-holes disappear, it can sit right next to U2.OSC, and the XTAL traces become as short as possible on the surface layer (which effectively retires the xtal_check constraint as well). C7/C8 change to 12 pF | **C13738** | **Basic** |
| **Q2 → replaced by the SI2309 and moved to the F side** (added 2026-10-16) | SOT-26 hand-soldering land | **Replaced by an SI2309-family −60 V PMOS in SOT-23.** It has the same Vds −60 V / Vgs ±20 V as the ZXMP6A17E6Q (which is not in LCSC stock), so the gate divider (Vgs=13 V when clamped) and the surge rating still work unchanged. Id 1.6–2 A / Rds(on) 0.2 Ω@−10 V is lower than the current part (3 A/0.115 Ω), but at this circuit's load (about 0.3 A average into the buck) that is ~20 mW of loss and 30× the margin. The package also shrinks from SOT-26 to SOT-23. Genuine = Vishay **Si2309CDS-T1-GE3 (C10493)**; the cheap equivalent = BORN SI2309 (**C431496**, 95k in stock, 2 A/200 mΩ) | C10493 / C431496 | Extended |

That leaves **D6 (the side-view LED) as the only hand-soldered SMD part on the B side**.

**A design caution when adopting the SI2309**: its pinout becomes the SOT-23 standard
**1=G / 2=S / 3=D**, which is completely different from the SOT-26 (D×4/G/S). As v3.1 taught,
the synthetic DEVMAP entry and **the per-build pad-map assertion must be rewritten for the new
pinout** (G=1, S=2, D=3). Losing AEC-Q101 qualification is accepted (consumer grade; if that is a
concern, treat the hand-soldered v4.2 as the automotive-grade version).

### Not moving (staying on the B side with their current type numbers)

| Part | Reason |
|---|---|
| D6 side-view LED | Not in JLC stock (Kodenshi). Its optical placement on the back at the y=61 edge (the emitting face at the board edge) is also not being changed. On the front there is physically no room at y59–60 because of J2's shell |
| J3 GPS pads | Not a part but copper pads (the GPS wires are hand soldered) |
| U3 Pico socket, J1 MQS | Through-hole. Out of scope for JLC SMT (fitted by hand as before) |

## 3. Rough cost (assembly)

- **Single-sided (F side only) SMT** works → eligible for Economic PCBA
- Extended parts: U2, U4, L1, J2, D2, F1, Q2 (SI2309), plus C14/C15 depending on their category ≈
  **7–9 parts × $3 = $21–27** in feeder charges
- Basic: all the chips (even after moving to 0402), D1/D3, Q1, Y1 (3225)
- The parts themselves come to roughly $3–4 per board in small quantities (U2 dominates)

## 4. Feasibility (area and routing)

- Free space on the F side: the power band (y8.3–21), about 190 mm², plus what is left of the middle
  band (y31–42). Moving the 23 chips to 0402 frees up about another 40 mm²
- Space taken by the parts being moved (rough, including clearance): U4 9 + L1 20 + C14 8 + C15 5 +
  D1/D3 14 + D2 13 + F1 10 + Q1 7 + Y1 (3225) 12 ≈ **100 mm²**, or about 200 mm² with a packing
  factor of 2
- → **It looks feasible, but it is the highest density since v1.** The design rules go back to the
  0.127 mm that 0402 assumes (with margin over JLC's 4-layer minimum of 0.09/0.09). Expect to need a
  generous number of seeds
- The B side becomes almost empty (Q2, D6, the socket and Y1's holes all gone), so back-side routing
  gets easier
- Because Q2 moves to the F side too, the power chain (J1.8→F1→D1→D2→Q2→U4) is completed entirely on
  one side

## 5. How to implement it in the generation system (when it is done)

1. Add a new `MRP_ASSY=jlc` environment variable that switches mrp_design/mrp_brd/ksym_map:
   - `CHIP='1005'` (the existing mechanism; the override keeping C2 at 1608 also already exists)
     plus a per-part size table for C13/C6/C11
   - STOCK swaps: `TSOT26-HS`→`TSOT-23-6`, `L4030`→`L_Abracon_ASPI-4030S` (the standard land),
     `HC49S`→`Crystal_SMD_3225-4Pin` (**4 pads**: 2 and 4 are GND → a synthetic DEVMAP entry and
     GND additions to NETS are required), and C7/C8 to 12 pF
   - Placement: rewrite EXPLICIT/ZONES completely for the F side (only Q2 stays on the B side)
2. Rules at 0.127, roughly `try_thermal.ps1 -Seeds 16`. xtal_check stays even after the move to 3225
   (it is harmless)
3. Add an LCSC number column to the BOM (a JLC upload auto-matches when there is an `LCSC Part#`
   column)
4. Verification as before: DRC 0/0, compare_net, the planes, and updating the shopping list

## 6. To confirm when ordering (points that could not be settled from the web)

- The Basic/Extended category and the stock of F1 (C3761685), C14 (1206/50 V 10 µF),
  C15 (0805/16 V 22 µF) and C13
- Whether any of the Extended parts have become "Promotional Extended" (no feeder charge)
- The final load capacitance against the X322516MLB4SI's CL=9 pF (measure and adjust to 10–15 pF
  including board stray capacitance)

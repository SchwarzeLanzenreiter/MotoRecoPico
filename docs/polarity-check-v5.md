# Record of the v5 polarized-part datasheet check (2026-10-16)

For every polarized or orientation-sensitive part on the v5 board (the JLC-assembled version), the
manufacturer's datasheet was saved into `docs/datasheets/` and its pinout drawing was checked against
the board's actual pads (coordinates, nets and silkscreen). This is the procedure that prevents a
recurrence of the v3.1 Q2 mirroring accident (trusting the library and never looking at the
datasheet). The board data is as of
`MotoRecoPico_backup_20261016-0748_v5-jlc-0603`.

## Result: all 6 parts pass

| Part | Datasheet (saved name) | Pinout per the datasheet | Measured on the board (pad → net) | Verdict |
|---|---|---|---|---|
| **Q2 SI2309** (new, and the most important) | Q2_SI2309CDS_Vishay.pdf (68980, Top View p.1) | SOT-23: **1=G (the two-pin side, upper), 2=S (the two-pin side, lower), 3=D (the single-pin side)** | pad1(12.96,8.95)=Q2_G ✓ / pad2(12.96,10.85)=V12P=the source ✓ / pad3(14.84,9.90)=V12_SW=the drain ✓. The geometry also agrees, with 1 and 2 in the left column and 3 on the right | **Pass** |
| **Q1 MMBT3904** | Q1_MMBT3904_PANJIT.pdf (Top View p.1) | SOT-23: **1=Base, 2=Emitter, 3=Collector** | pad1=Q1_B ✓ / pad2=GND (E) ✓ / pad3=Q1_C ✓ | **Pass** |
| **D1/D3 B5819W (= 1N5819HW)** | D1D3_1N5819HW_B5819W_Diodes.pdf (DS30217, Device Schematic p.1) | SOD-123: **1=CATHODE (the band side), 2=ANODE** | D1: pad1=V12P (K) / pad2=V12_FUSED (A) — current flows A→K ✓. D3: pad1=VSYS (K) / pad2=V5 (A) ✓. The silkscreen band is at the pad 1 end on both ✓ | **Pass** |
| **D2 SMAJ16A** | D2_SMAJ16A_Littelfuse.pdf (SMAJ series, Physical Spec p.5) | SMA: **the colored band is the cathode** (unidirectional). SMAJ16A: VR 16 V / VBR 17.8–19.7 / **Vc 26.0 V** | pad1(19.20,13.80)=V12P=the cathode (reverse-biased at rest) ✓ / pad2=GND=the anode ✓. The silkscreen band is at the pad 1 end (y15.3) ✓. Vc 26.0 V is exactly the design value | **Pass** |
| **D6 LP812-010(T)** | D6_LP812-010_Kodenshi.pdf (outline dimensions p.3) | **(1)=Cathode, (2)=Anode**. Recommended pattern: two 1.4 mm pads, 1.0 mm apart | pad1=LED_STATUS (K; GPIO low-side drive, lights when low) ✓ / pad2=LED_A (+5 V through R14) ✓. The pads of our custom footprint (1.4×0.9, centers at ±1.2) match the recommended pattern. The cathode bar on the silkscreen is on the pad 1 side | **Pass** (when hand soldering: put the part's terminal (1) toward the bar) |
| **U4 AP63205WU-7** | U4_AP63205WU_C2071056.pdf (the DS41326 family) | TSOT-26: 1=FB 2=EN 3=VIN 4=GND 5=SW 6=BST (cross-checked against the KiCad symbol in v4) | pad1=V5 (FB) / pad2,3=V12_SW (EN, VIN) / pad4=GND / pad5=SW_5V / pad6=BST_5V — **enforced by a per-build assertion** ✓. The pin 1 dot on the silkscreen is at the pad 1 corner | **Pass** |

## Parts excluded from the check

- **U2 MCP25625**: checked against datasheet Table 1-1 back in v1, with a track record of working
  hardware (unchanged)
- **Y1 3225 crystal**: the crystal's two terminals (pads 1 and 3) have no polarity. The lid's GND
  (2 and 4) is wired from the net definitions
- **The Pico (U3)**: `mrp_brd.py` asserts the pin 1 column, which way the USB end faces, and the
  8 GND pads on every build
- MLCCs, resistors, the PPTC: not polarized

## How the verification works (restated)

The "pad number → net" mapping for Q2 and U4 is hard-coded in `mrp_brd.py` as the expected values
derived from the datasheets, and **it is asserted on every build**. Even if the footprint or the
library is swapped out, the board will not be generated unless the check passes.

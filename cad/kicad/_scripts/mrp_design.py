# -*- coding: utf-8 -*-
"""Single source of truth for MotoRecoPico: parts, nets and package mapping.

Reads the existing (verified) EAGLE schematic and library, then translates every
package to a KiCad footprint. Two corrections are applied here:

  * SD card detect. The Hirose drawing did not label the auxiliary pads, so the
    EAGLE library guessed TR/BR were the detect switch. KiCad's official
    footprint - which matches ours on 13 of 14 pads to within 0.001mm - shows the
    switch is TL/L2 (its pads 9/10) and TR/L1/L3/BR are cover ground legs. As
    drawn in EAGLE both switch terminals sit on GND and the sense line hangs off
    a ground leg, so card detect would read "inserted" forever.
  * BR pad geometry. Ours was at x+4.195 2.55x1.36; the official one is at
    x+6.675 1.3x1.9. The official footprint wins.
"""
import os
import xml.etree.ElementTree as ET

# Chip size for every resistor and capacitor that EAGLE calls C1005. 1608 is the
# design; 1005 is kept only so the two can be measured against each other.
# See docs/study-1608-chips.md.
CHIP = os.environ.get('MRP_CHIP', '1608')
assert CHIP in ('1005', '1608'), f"MRP_CHIP must be 1005 or 1608, got {CHIP!r}"

LBR = r"E:\Claude\MotoRecoPico\cad\electronics\MotoRecoPico.lbr"
SCH = r"E:\Claude\MotoRecoPico\cad\electronics\MotoRecoPico.sch"
SHARE = r"D:\Program Files\KiCad\10.0\share\kicad\footprints"
PRETTY = r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.pretty"

_lib = ET.parse(LBR).getroot().find('./drawing/library')
_sch = ET.parse(SCH).getroot().find('./drawing/schematic')

PKG = {p.get('name'): p for p in _lib.findall('./packages/package')}
DEVMAP = {}
for _d in _lib.findall('./devicesets/deviceset'):
    for _dev in _d.findall('./devices/device'):
        if _dev.get('package'):
            DEVMAP[(_d.get('name'), _dev.get('name'))] = (
                _dev.get('package'),
                {c.get('pin'): c.get('pad') for c in _dev.findall('./connects/connect')})

PARTS = {p.get('name'): (p.get('deviceset'), p.get('device'), p.get('value') or '')
         for p in _sch.findall('./parts/part')}
NETS = {n.get('name'): [(r.get('part'), r.get('pin')) for r in n.findall('.//pinref')]
        for n in _sch.findall('./sheets/sheet/nets/net')}


PKG_OVERRIDE = {}          # only used when reverting to CHIP=1005; see below


def eagle_pkg(ref):
    return PKG_OVERRIDE.get(ref) or DEVMAP[(PARTS[ref][0], PARTS[ref][1])][0]


# ------------------------------------------------------- correction: SD card detect
def _fix_card_detect():
    def drop(net, pin):
        NETS[net] = [(p, q) for p, q in NETS[net] if not (p == 'J2' and q == pin)]
    drop('SD_CD', 'TR')          # TR is a cover ground leg, not the switch
    drop('GND', 'TL')            # TL is one switch terminal -> becomes the sense line
    NETS['SD_CD'].append(('J2', 'TL'))
    NETS['GND'].append(('J2', 'TR'))
    # L2 (the other switch terminal) and L1/L3/BR (ground legs) stay on GND


_fix_card_detect()


# ------------------------------------- correction: Pico GPIO map follows the board
# The original map was chosen logically and turned out to be the physical inverse
# of the layout, and v1 could not be routed until the pins were re-assigned by
# DISTANCE to the part each one serves. v4.1 rotates the Pico 180 deg in-plane
# (USB now faces the microSD edge), which sends every pin to its diagonal
# opposite (pin k's hole now holds pin k+20), so the whole map is re-derived for
# the rotated orientation. Two symmetries make the rotation cheap: the 2x20 grid
# is 180-deg symmetric, so no HOLE moves; and the GND pins (3/8/13/18 <->
# 23/28/33/38) land on each other, so every ground barrel stays a ground barrel.
#
# Rotated pin positions: pads 1..20 run UP the right column (pad 1 at y57.6 by
# the USB/microSD end, pad 20 at y9.4), pads 21..40 run DOWN the left column
# (pad 21 at y9.4, pad 40/VBUS at y57.6).
#
# The role swap SPI0 <-> SPI1 is what makes a legal nearest-pin map exist:
#   SPI0  SCK GP2  TX GP3  RX GP4  CSn GP5      -> microSD, beside J2's contacts
#   SPI1  SCK GP10 TX GP11 RX GP12 CSn GP13     -> CAN, beside U2
#   UART0 TX  GP0  RX GP1                       -> GPS, right above J3
# IG_SENSE keeps GP28/ADC2 (pad 34, now left column y42.4) - the firmware's ADC
# setup is untouched; only the SPI/UART pin definitions change.
U3_REMAP = {
    # right column, next to U2 (pads y 17.0 .. 29.7 after rotation)
    '14': 'CAN_SCK',     # GP10, SPI1 SCK
    '15': 'CAN_MOSI',    # GP11, SPI1 TX
    '16': 'CAN_MISO',    # GP12, SPI1 RX
    '17': 'CAN_CS',      # GP13, SPI1 CSn
    '12': 'CAN_INT',     # GP9, U2's lower flank (y29.7)
    # left column, U2's other flank (slow control lines)
    '26': 'CAN_STBY',    # GP20 (y22.1)
    '27': 'CAN_RESET',   # GP21 (y24.6)
    # right column, next to J2's contact row at y~43.9
    '4': 'SD_SCK',       # GP2, SPI0 SCK (y50.0)
    '5': 'SD_MOSI',      # GP3, SPI0 TX  (y47.5)
    '6': 'SD_MISO',      # GP4, SPI0 RX  (y44.9)
    '7': 'SD_CS',        # GP5, SPI0 CSn (y42.4)
    '10': 'SD_CD',       # GP7, slow - one row up is fine (y34.8)
    # right column bottom, right above J3's pads (bottom face, y 44 .. 56)
    '1': 'GPS_TX',       # GP0, UART0 TX (y57.6)
    '2': 'GPS_RX',       # GP1, UART0 RX (y55.1)
    '9': 'GPS_PPS',      # GP6 (y37.3), nearest free pin
    '11': 'LED_STATUS',  # GP8 (y32.2) - DC drive, distance does not matter
    # '34' (GP28/ADC2) is IG_SENSE, wired in _add_soft_poweroff() below.
}


def _remap_pico():
    pads = DEVMAP[(PARTS['U3'][0], PARTS['U3'][1])][1]
    pin_of_pad = {pad: pin for pin, pad in pads.items()}
    assert len(set(U3_REMAP.values())) == len(U3_REMAP), "a net was assigned twice"
    moved = set(U3_REMAP.values())
    for net in moved:
        assert net in NETS, f"{net} is not a net"
        NETS[net] = [(p, q) for p, q in NETS[net] if p != 'U3']
    for pad, net in U3_REMAP.items():
        NETS[net].append(('U3', pin_of_pad[pad]))
    # nothing that used to be on U3 may have been dropped on the floor
    orphan = [n for n in moved if not any(p == 'U3' for p, _ in NETS[n])]
    assert not orphan, f"lost the Pico end of {orphan}"


_remap_pico()


# --------------------------------------- simplification: plain 120R termination
# Dropped at the user's request: the split termination (R8+R9 with C9 to ground
# through JP1) and the PESD1CAN bus TVS. R8 alone now spans the bus at 120R.
#
# D5's wiring was wrong anyway - the Nexperia datasheet (Rev.04, Table 2) makes
# pin 3 the common cathode, not pin 2, so the part had been drawn with its common
# terminal on CANL and one protected line on ground. Removing it settles that.
# JP1 was documented as "disconnects the CAN termination" but sat in series with
# C9 alone, so opening it left the 120R across the bus regardless.
DROPPED = ('D5', 'JP1', 'C9', 'R9')


def _simplify_can_termination():
    for ref in DROPPED:
        PARTS.pop(ref, None)
    for net in list(NETS):
        NETS[net] = [(p, q) for p, q in NETS[net] if p in PARTS]
        if not NETS[net]:
            del NETS[net]
    for dead in ('CAN_TERM_MID', 'CAN_TERM_C'):
        NETS.pop(dead, None)
    NETS['CANL'].append(('R8', '2'))          # R8 pin 1 is already on CANH
    ds, dev, _ = PARTS['R8']
    PARTS['R8'] = (ds, dev, '120')
    assert ('R8', '1') in NETS['CANH'], "R8 lost its CANH end"


_simplify_can_termination()


# ------------------------------------ simplification: no more IG voltage sensing
# Dropped at the user's request (2026-08-08, after the v1 boards arrived): the
# Pico no longer measures the IG line, so the ADC tap and everything that only
# served it goes away - D4 (the ADC clamp zener), C12 (the ADC filter), and the
# GP28 connection. R5 goes too: it only decoupled the ADC tap from Q1's base.
#
# The IG POWER SWITCH stays - it is what turns the logger on with the key. Q1's
# base now hangs directly on the R3/R4 divider: R3 (100k) limits base current
# (a 100V transient on the unclamped IG line pushes under 1mA into the B-E
# junction), R4 (22k) holds the base at ground when IG floats. At 14V the base
# sees ~100uA of drive against the ~70uA collector load of Q2's gate divider -
# fine for any beta above ~3. The IG_DIV net disappears into Q1_B.
IG_DROPPED = ('R5', 'D4', 'C12')


def _drop_ig_sense():
    for ref in IG_DROPPED:
        assert ref in PARTS, f"{ref} already gone?"
        PARTS.pop(ref)
    for net in list(NETS):
        NETS[net] = [(p, q) for p, q in NETS[net] if p in PARTS]
        if not NETS[net]:
            del NETS[net]
    # IG_DIV survives as (R3.2, R4.1) plus the Pico ADC pin; fold it into Q1_B
    # and cut the Pico loose.
    assert 'IG_DIV' in NETS and 'Q1_B' in NETS
    NETS['Q1_B'].extend((p, q) for p, q in NETS.pop('IG_DIV') if p != 'U3')
    got = sorted(NETS['Q1_B'])
    want = sorted([('Q1', 'B'), ('R3', '2'), ('R4', '1')])
    assert got == want, f"Q1_B ended up as {got}"


_drop_ig_sense()


# ---------------------------------------- v3: soft power-off (delay + IG detect)
# IG OFF used to kill the 5V rail within milliseconds - no chance to close the
# log file. Two parts fix that (2026-08-09, user request):
#
#   * C13, 10uF/25V, across Q2's gate-source IN PARALLEL WITH R1. At IG-off the
#     gate can only drift back to the source through R1 (100k), so Q2 keeps
#     conducting for t = R1*C13_eff*ln(7V/3.5V) - about 0.4s guaranteed with the
#     MLCC derated to ~6uF at bias, ~0.5s typical. Bonus: the same RC swallows
#     key-contact chatter, and if the firmware ever hangs the power still dies -
#     there is no battery-draining latch mode.
#   * R15, 10k, from Q1's base node to GP28 (pin 34). The base is clamped at or
#     below ~0.7V by its own B-E junction (R3 limits surge current), so the pin
#     needs no zener, divider or filter. 0.7V is below VIH though - the firmware
#     MUST read it with the ADC (threshold ~0.35V), not as a digital input.
#
# Firmware contract: poll GP28's ADC at <=50ms; below 0.35V flush, close and
# rename within the guaranteed 0.4s. To stretch the window add another C13 in
# parallel (+0.4s per 10uF); do NOT grow R1, that skews the Vgs divider toward
# the +/-20V gate rating during load dump.
def _add_soft_poweroff():
    ds, dev, _ = PARTS['C6']              # any existing 1608 capacitor device
    PARTS['C13'] = (ds, dev, '10uF/25V')
    ds, dev, _ = PARTS['R6']              # any existing chip resistor device
    PARTS['R15'] = (ds, dev, '10k')

    NETS['Q2_G'].append(('C13', '1'))
    NETS['V12P'].append(('C13', '2'))
    NETS['Q1_B'].append(('R15', '1'))
    pads = DEVMAP[(PARTS['U3'][0], PARTS['U3'][1])][1]
    pin_of_pad = {pad: pin for pin, pad in pads.items()}
    NETS['IG_SENSE'] = [('R15', '2'), ('U3', pin_of_pad['34'])]

    assert not any(p == 'U3' for p, _ in NETS['Q1_B']), "Q1_B must not touch U3"


_add_soft_poweroff()


# --------------------------------------------- v4: 5V rail shrinks to a TSOT buck
# U1 (MinMax M78AR05-0.5, SIP-3) was 10.2mm tall - the second-tallest part - and
# rated 500mA against a 200mA load. Replaced 2026-08-14 (user request) with an
# AP63205WU-7: 3.8-32V in, fixed 5V out, 2A synchronous, TSOT-26, plus four
# passives. Tallest part of the converter is the 4030 inductor at 3mm.
#
# Two ratings anchor the choice: VIN max 32V clears D2's 26V clamp by 6V, and
# VIN min 3.8V retires the open issue about cranking dips below M78AR05's 6.5V
# UVLO - the 5V rail now rides through any start the battery survives.
#
# Pinout 1=FB 2=EN 3=VIN 4=GND 5=SW 6=BST, confirmed against BOTH the KiCad
# symbol (Regulator_Switching:AP63205WU) and DS41326 p.1 "Pin Assignments" on
# 2026-08-14 - the Q2 mirror bug taught us not to trust one source alone.
# Fixed-output part: FB ties to VOUT, EN ties to VIN (enabled whenever powered).
def _replace_5v_buck():
    dead = PARTS.pop('U1')
    assert dead[2] == '' or 'M78' in str(dead), "expected to be removing the SIP module"

    DEVMAP[('AP63205', '')] = ('TSOT26-HS', {
        'FB': '1', 'EN': '2', 'VIN': '3', 'GND': '4', 'SW': '5', 'BST': '6'})
    DEVMAP[('L-POWER', '')] = ('L4030', {'1': '1', '2': '2'})
    DEVMAP[('C-2012', '')] = ('C2012', {'1': '1', '2': '2'})
    DEVMAP[('C-3216', '')] = ('C3216', {'1': '1', '2': '2'})

    PARTS['U4'] = ('AP63205', '', 'AP63205WU-7')
    PARTS['L1'] = ('L-POWER', '', '6.8uH')
    PARTS['C14'] = ('C-3216', '', '10uF/50V')     # input: sees the 26V clamp
    PARTS['C15'] = ('C-2012', '', '22uF/16V')     # output bulk on V5
    PARTS['C16'] = ('C-2012', '', '100nF/50V')    # bootstrap; 2012 so it can sit
                                                  # beside U4 on the BOTTOM without
                                                  # breaking the 1608-on-top rule

    def drop(net, ref):
        NETS[net] = [(p, q) for p, q in NETS[net] if p != ref]
    drop('V12_SW', 'U1'); drop('V5', 'U1'); drop('GND', 'U1')

    NETS['V12_SW'] += [('U4', 'VIN'), ('U4', 'EN'), ('C14', '1')]
    NETS['V5'] += [('L1', '2'), ('C15', '1'), ('U4', 'FB')]
    NETS['GND'] += [('U4', 'GND'), ('C14', '2'), ('C15', '2')]
    NETS['SW_5V'] = [('U4', 'SW'), ('L1', '1'), ('C16', '1')]
    NETS['BST_5V'] = [('U4', 'BST'), ('C16', '2')]

    # the two ties a fixed-output buck cannot live without
    assert ('U4', 'FB') in NETS['V5'], "FB must sense VOUT on the fixed part"
    assert ('U4', 'EN') in NETS['V12_SW'], "EN must ride VIN"
    assert not any(p == 'U1' for n in NETS.values() for p, _ in n), "U1 lingers"


_replace_5v_buck()


# ------------------------------- correction: parts JLCPCB can actually fit and stock
# Both of these came out of matching the BOM against JLCPCB's basic-parts library.
#
#   * D1/D3 were written up as "SS14 or B140 (1A/40V)" on a SOD-123 land, but SS14
#     only exists in SMA (DO-214AC) - ordering it would have put an SMA body on a
#     SOD-123 footprint. B5819W is 40V/1A in SOD-123 AND is a basic part, so the
#     package error and the feeder fee go away together. Vf is 0.6V max at 1A, so
#     at the ~100mA this board draws the drop is unchanged from the SS14 estimate.
#   * C2 sits on V12P, which D2 clamps at 26V, and the only 100nF 0402 in the
#     basic library is rated 16V - not enough. At CHIP=1608 every 100nF on the
#     board is already a 50V part, so the override only matters when reverting
#     the rest of the chips to 1005 for comparison.
def _jlc_basic_parts():
    for ref in ('D1', 'D3'):
        ds, dev, val = PARTS[ref]
        assert val == 'SS14', f"{ref} was expected to be SS14, found {val!r}"
        assert DEVMAP[(ds, dev)][0] == 'SOD123', f"{ref} is not on a SOD-123 land"
        PARTS[ref] = (ds, dev, 'B5819W')

    if CHIP == '1005':
        ds, dev, _ = PARTS['C2']
        assert DEVMAP[(ds, dev)][0] == 'C1005', "C2 was expected to start as an 0402"
        PKG_OVERRIDE['C2'] = 'C1608'


_jlc_basic_parts()

# ------------------------------------------------------------- footprint mapping
STOCK = {
    'C1608': ('Capacitor_SMD', 'C_0603_1608Metric'),
    'C2012': ('Capacitor_SMD', 'C_0805_2012Metric'),
    'C3216': ('Capacitor_SMD', 'C_1206_3216Metric'),
    'TSOT26-HS': ('Package_TO_SOT_SMD', 'TSOT-23-6_HandSoldering'),
    'MICROSD-DM3AT': ('Connector_Card', 'microSD_HC_Hirose_DM3AT-SF-PEJM5'),
    'RPI-PICO2-THT': ('Module', 'RaspberryPi_Pico_Common_THT'),
    'SSOP28': ('Package_SO', 'SSOP-28_5.3x10.2mm_P0.65mm'),
    'SOT23': ('Package_TO_SOT_SMD', 'SOT-23'),
    # v4.2: Q2 on the same hand-soldering land as U4 (2.0 x 0.65mm pads at
    # +/-1.71 instead of 1.3 x 0.6 at +/-1.14). Pad NUMBERING is the same
    # JEDEC order, so PADMAP['SOT26'] and the per-build G/S assert still hold.
    'SOT26': ('Package_TO_SOT_SMD', 'TSOT-23-6_HandSoldering'),
    'SMA': ('Diode_SMD', 'D_SMA'),
    'SOD123': ('Diode_SMD', 'D_SOD-123'),
    'HC49S': ('Crystal', 'Crystal_HC49-4H_Vertical'),
    'SJ2': ('Jumper', 'SolderJumper-2_P1.3mm_Bridged_Pad1.0x1.5mm'),
}
CUSTOM = {
    'MQS-8-RA-967658': 'MQS-8-RA-1-967658-1',
    'GPS-5P-SMD': 'GPS-5P-SMD-2.54',
    'LED-SIDE-2812': 'LED-SideView-2.8x1.2',
    'SIP3-M78AR05': 'MinMax-M78AR05-SIP3',
    # v4.2: stock ASPI-4030S land with the pads stretched 1mm outward (mrp_fp.py)
    'L4030': 'L_4030_HandSolder',
}
# 1005 and 1206 land patterns differ between the resistor and capacitor libraries,
# so they are chosen by what the part actually is, not by the shared EAGLE package.
BY_PREFIX = {
    'C1005': {'R': ('Resistor_SMD', 'R_0402_1005Metric'),
              'C': ('Capacitor_SMD', 'C_0402_1005Metric')},
    'C1206': {'F': ('Fuse', 'Fuse_1206_3216Metric'),
              'C': ('Capacitor_SMD', 'C_1206_3216Metric')},
}

# Every chip is a 1608. It was measured against 1005 (docs/study-1608-chips.md):
# the strip between the Pico's header columns still takes all 20 even though they
# occupy 91mm2 instead of 38mm2, the board stays 21x61, and the bypass caps end up
# CLOSER to their pins, not further (101mm of pad-to-pad total against 85mm).
# 1608 is also far easier to rework by hand, and JLCPCB's basic 100nF is 50V at
# this size against 16V at 1005.
#
# The EAGLE package name stays C1005 - only the KiCad footprint changes - so the
# "every chip on the top face" check still keys off the same name.
if CHIP == '1608':
    BY_PREFIX['C1005'] = {'R': ('Resistor_SMD', 'R_0603_1608Metric'),
                          'C': ('Capacitor_SMD', 'C_0603_1608Metric')}

# EAGLE pad name -> KiCad pad name, per package. Only where they differ.
PADMAP = {
    'MICROSD-DM3AT': {'TL': '9', 'L2': '10',
                      'TR': 'SH', 'L1': 'SH', 'L3': 'SH', 'BR': 'SH'},
    # Q2 (ZXMP6A17E6Q). The EAGLE library numbered the SOT26 pads G=1, S=6,
    # D=2..5 - a MIRROR of the real device. Diodes DS36685 Rev 3-2 p.1
    # ("Pin Out - Top View", verified against the PDF on 2026-08-13) numbers it
    #   1=D  2=D  3=G  4=S  5=D  6=D
    # which also matches KiCad's SOT-23-6 pad geometry (pin 1 at the dot corner).
    # The v1 boards carry the mirrored layout: no rotation of the real part fits
    # them - one orientation shorts V12P to V12_SW through the common drain
    # (always-on), the other lands G on V12P and S on the divider (never-on).
    # Both failure modes were observed on the bench before this was found.
    'SOT26': {'1': '3',                        # G
              '6': '4',                        # S
              '2': '1', '3': '2', '4': '5', '5': '6'},   # D (interchangeable)
}


def footprint_of(ref):
    """-> (library directory, footprint name)"""
    pk = eagle_pkg(ref)
    if pk in CUSTOM:
        return PRETTY, CUSTOM[pk]
    if pk in BY_PREFIX:
        lib, fp = BY_PREFIX[pk][ref[0]]
    else:
        lib, fp = STOCK[pk]
    return SHARE + '\\' + lib + '.pretty', fp


def pads_of(ref, pin):
    """-> list of KiCad pad names carrying this schematic pin."""
    pk = eagle_pkg(ref)
    eagle_pads = DEVMAP[(PARTS[ref][0], PARTS[ref][1])][1][pin].split()
    m = PADMAP.get(pk, {})
    return [m.get(p, p) for p in eagle_pads]


if __name__ == '__main__':
    import sys, io, collections
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
    by_pkg = collections.defaultdict(list)
    for r in PARTS:
        by_pkg[eagle_pkg(r)].append(r)
    print(f"{len(PARTS)} parts, {len(NETS)} nets\n")
    print(f"{'EAGLE package':<18} {'n':>2}  KiCad footprint")
    for pk in sorted(by_pkg):
        refs = sorted(by_pkg[pk])
        fps = sorted({footprint_of(r)[1] for r in refs})
        print(f"{pk:<18} {len(refs):>2}  {' + '.join(fps)}")
        print(f"{'':<21} {' '.join(refs)}")
    print("\nJ2 nets after the card-detect fix:")
    for n in ('SD_CD', 'GND'):
        print(f"  {n:<6}", sorted(q for p, q in NETS[n] if p == 'J2'))

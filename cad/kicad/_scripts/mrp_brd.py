# -*- coding: utf-8 -*-
"""Build MotoRecoPico.kicad_pcb - 21 x 65 mm, 2 layer.

KiCad board coordinates: origin top-left, X right, Y DOWN.
  y = 0   vehicle connector edge (J1 mating face points off-board)
  y = 65  microSD slot + status LED edge

Two changes against the 21 x 60 EAGLE layout:
  * the board grew 5mm at the connector end. The MQS housing overhangs the board
    by 8mm, and the Pico header row started 8.37mm from the datum, so the housing
    cleared the header pads by 0.37mm - it would have fouled the Pico. It now
    clears by 5.37mm.
  * Y1 moved to the BOTTOM. It is an HC-49/S through-hole can, so the side it is
    inserted from is the side its body ends up on, and the Pico occupies the top.
    Its pads live on both layers either way, so nothing about the oscillator
    routing changes.
"""
import sys, io, os
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import pcbnew
import mrp_design as D

OUTDIR = r"E:\Claude\MotoRecoPico\cad\kicad"
BRD = os.path.join(OUTDIR, "MotoRecoPico.kicad_pcb")
W, H = 21.0, 61.0
MIDX0, MIDX1 = 2.9, 18.1              # clear strip between the Pico header columns
MM, TOMM = pcbnew.FromMM, pcbnew.ToMM


def V(x, y):
    return pcbnew.VECTOR2I(MM(x), MM(y))


# ---------------------------------------------------------------- placement
# (x, y, rotation degrees, side, anchor). anchor 'pad' = centre of the pad bbox,
# 'origin' = the footprint's own origin, which is the controlled datum on the two
# connectors.
EXPLICIT = {
    # ---- BOTTOM (the side away from the Pico)
    # v4: J1 moved to the TOP face (user request - the vehicle coupler now plugs
    # in on the Pico side). The MQS pin grid is staggered and NOT mirror
    # symmetric, so this is not just a housing move: the face flip mirrors the
    # hole pattern, which is exactly what mounting the real part from the other
    # side requires. Pad-to-pin correspondence follows the footprint through the
    # flip, so every net stays on the right vehicle pin automatically.
    'J1': (10.5, 0.0, 0, 'T', 'origin'),   # MQS datum on y=0; housing covers y 0..8
    # Y1 and J3 are each ~12mm wide. Left flat they sat across the 15.2mm
    # strip and cut the bottom layer into three disconnected pockets, so nothing
    # could run lengthwise. Turned 90 deg and stacked down the right-hand side
    # they leave a clear vertical channel on the left, which is the side the Pico
    # header pins that talk to U2 and J2 are on.
    'Y1': (15.0, 35.0, 90, 'B', 'pad'),    # HC-49/S crystal, inserted from below
    'J3': (15.0, 50.0, 90, 'B', 'pad'),    # GPS pads, under the microSD socket
    'D6': (10.5, 59.8, 0, 'B', 'pad'),     # side-view LED, emitting face at y=61
    # ---- the Pico, which stands off on socket strips
    # v4.1: rotated 180 deg so the USB port faces the microSD edge (y=61) instead
    # of sitting 1.4mm from the 18.1mm MQS housing. The 2x20 grid is rotation
    # symmetric, so every drilled hole stays where it was - only the pin sitting
    # in it changes (k <-> k+20). U3_REMAP was re-derived for this orientation.
    # v4.2: moved to the BOTTOM face (user request). The face flip mirrors the
    # footprint about x=10.5, so the two hole columns swap roles: pads 1..20 now
    # run up the LEFT column (x=1.61) and 21..40 down the RIGHT - again no hole
    # moves, and the USB end stays at y=61. U3_REMAP was chosen by y-proximity,
    # which the mirror preserves (U2 and J2 are x-symmetric about the centre),
    # so the GPIO map - and the firmware - stay as in v4.1. The 1608 chips keep
    # the TOP face, which the Pico no longer covers.
    'U3': (10.5, 33.5, 180, 'B', 'pad'),   # Pico 2; header columns at x=1.61/19.39
    # MCP25625 turned 90 deg. Unrotated its two 14-pad rows face the long edges,
    # so every escape had to squeeze into a 3.1mm side corridor - 28 pins cannot
    # fit through that on two layers. Rotated, the rows face along the board and
    # escape into the open middle strip.
    'U2': (10.5, 26.0, 90, 'T', 'pad'),
    'J2': (10.5, 51.6, 0, 'T', 'origin'),  # microSD: contacts at origin-7.725, shell
                                           # ends +8.4, so the slot is ~1mm inside y=61
    # v3: the crystal's load caps flank Y1's through-hole barrels on the TOP face.
    # The barrels carry XTAL1/XTAL2 to this face, so each cap connects without a
    # via and every pad sits within 2.6mm of its barrel (they were 9-11mm away on
    # the far side of the board, and XTAL1 wandered 8.5mm through the In1 plane).
    # A pocket BETWEEN the barrels only holds one cap - zone packing put the
    # second one 3-4mm off - so both are pinned explicitly, standing upright,
    # clear of the pads' x-band (14.25..15.75).
    # Rotations chosen so pin 1 (the XTAL net) faces its barrel: C7.1 south
    # toward Y1.1 (y37.44), C8.1 north toward Y1.2 (y32.56). At 90 deg C8's
    # pin 1 pointed the wrong way and measured 4.1mm instead of 2.5mm.
    'C7': (13.15, 35.4, 90, 'T', 'origin'),
    'C8': (16.85, 35.4, 270, 'T', 'origin'),
}

# 12V enters at J1.8, and the chain is J1.8 -> F1 -> D1 -> [D2 shunt] -> Q2 ->
# U4.VIN; it should read that way on the board instead of doubling back.
# Variant B died with U1 (it existed to try the SIP regulator upright); only A
# remains meaningful since v4, where the buck replaces U1 in the same pocket.
VARIANT = (sys.argv[1] if len(sys.argv) > 1 else 'A').upper()
assert VARIANT == 'A', "variant B retired with U1 in v4"

# The series parts are placed explicitly rather than packed: their ORDER is the
# whole point, and the packer sorts by area and scans left to right.
# v4: flipping J1 to the top face MIRRORED its pin grid - 12V now enters at
# J1.8 = (3.50, 6.30) on the LEFT. The whole flow-order row mirrors with it:
# F1 left, D1 centre, Q2 right, buck below with VIN on the right, and V5
# leaving L1 to the LEFT, which is where its consumers sit (D3 in the switch-
# drive zone, C5/C6 via the plane). Rotations are the mirror of v3.1's,
# confirmed by the printed pad positions each build.
# v4.2 (hand-soldering feedback from the v4.1 boards): Q2 moves to the same
# 2.0 x 0.65mm hand-soldering land as U4, which is 6mm across - too wide to stay
# in the F1/D1 row, so it stands UPRIGHT (90 deg) in the right-hand column with
# its S/G pair in the column next to D1's cathode and its drains in the column
# by the board edge, above U4's VIN. L1 gets a custom land with the pads stretched 1mm outward and
# turns 90 deg so its pads face +/-y: the pad that used to butt against U4's SW
# column now sits beside it with copper to land an iron on. The whole pocket
# moves down ~1mm to make room; C15/D2/Q1/D3 zones follow below.
EXPLICIT.update({
    'F1': (5.30, 9.75, 180, 'B', 'pad'),    # pad 1 (from J1) on the left
    'D1': (10.20, 9.75, 0, 'B', 'pad'),     # anode left, cathode right
    'Q2': (16.20, 11.40, 90, 'B', 'pad'),   # upright; S (pad 4) top-left by D1, G below it, drains on the edge side
    # The left-hand power stack (L1, C15, D2, Q1, D3) sits 1.2mm further right
    # than the first v4.2 cut: with the Pico mirrored, the CAN SPI lines leave
    # the LEFT header column (pads 14-17, y 17..25) and have to reach U2's SPI
    # pins at the right end of its top row. Packed at x3.2 the stack left a
    # 1.3mm slot next to the header pads and CAN_MISO stayed unrouted on 19 of
    # 20 seeds; at x4.9 the slot is 2.5mm and four 0.15mm tracks pass.
    'U4': (12.90, 16.50, 0, 'B', 'pad'),    # VIN column right, SW column left
    'L1': (7.20, 18.50, 270, 'B', 'origin'), # pads face +/-y; pad 1 (SW) at top (90 put it at the bottom: 5.7mm)
    'C14': (16.90, 20.70, 270, 'B', 'origin'),  # input cap, below Q2, right of U4
    # bootstrap cap upright below U4's SW/BST column, ~4mm from the BST pin
    'C16': (11.70, 20.50, 90, 'B', 'origin'),
})
# D2 on the left below the buck pocket. Moving it right was tried in v1 and cost
# two more connections - it closed the right-hand lane instead of the left one.
SHUNT_ZONE = ('input TVS on V12P', (4.0, 25.3, 11.8, 30.5), 'B', ['D2'])   # v4.2: below C15, clear of the left lane

# zone: label, (x0,y0,x1,y1), side, members. Parts are packed largest first, row by
# row from the top of the rect, so a big part cannot strand a band by landing in
# the middle of it. Rect sizes come from the measured keepout dump, not guesses.
# Every 1005 chip stays on the TOP - fine-pitch assembly has to be single sided.
#
# TOP:    U2 (rotated) occupies y 21.2..30.8 across most of the strip, so its
#         decoupling goes in the bands immediately above and below it. Since
#         v4.2 the Pico is on the BOTTOM, so this face is fully exposed.
# BOTTOM: Pico socket strips down both edges (body 8.5mm above the board); the
#         power pocket, Y1 / J3 down the right at x>10.6, small parts on the left.
ZONES = [
    # Beside U2, not above and below it. Rotating U2 turned its escape fan-out into
    # the +/-y bands, and these eight chips were sitting right in it. U2's body only
    # spans x 5.9..15.1, so the flanks are free and the fan-out stays open.
    ('VIO decoupling', (MIDX0, 19.5, 5.6, 32.5), 'T', ['C3']),
    # R8 belongs where CANH/CANL leave U2 - pads 3 and 4 sit at x 7.6/8.2 on the
    # y=29.5 row. Parking it in the left flank blocked the only corridor from U2's
    # north-west corner round to the Pico's left header column.
    # v4.1: R15 (IG sense series R) moves in from the power-in zone. Its Pico end
    # (GP28, pad 34) rotated from y24.6 to y42.4 and its other end is Q1's base
    # (bottom face, y 24..34), so this band is now the midpoint of its run.
    # v4.2: R15 goes FIRST so it takes the left-hand slot (x~3.2, right over Q1 on
    # the other face) and R8 follows at x~6.9, still under U2's CANH/CANL pads.
    # With R15 at x 9.75..12.76 it sat on the straight line from U2.OSC2 (10.8,
    # 29.5) to Y1.2 (15.0, 32.6) and XTAL2 dived into the In1 plane on seed after
    # seed. GP28 is on the RIGHT column since the face flip, so R15 has no reason
    # to be near the centre any more; its ADC line crosses the board once.
    ('CAN termination + IG sense', (MIDX0, 31.5, 10.0, 34.2), 'T', ['R15', 'R8']),
    # R14 feeds the status LED from the 5V rail and looks like it belongs here,
    # but adding it to this band crowds C4/C5 away from U2's power pins and cost
    # 11 extra unrouted connections. It stays with the SD pull-ups.
    # C6 (VDDA bulk) moved up from the bottom face in v3 - every 1608 lives on
    # the top now. As a bulk cap it is distance-tolerant; it goes last so the
    # 100nF decouplers keep the seats nearest U2's power pins.
    ('VDD/VDDA decoupling + CAN pull-ups', (15.4, 19.5, MIDX1, 32.5), 'T',
     ['C4', 'C5', 'R6', 'R7', 'C6']),
    # Down against U2, not up at the board edge. These parts talk to Q1 (bottom,
    # y~25) and to the Pico's GP28 at y=24.6; sitting at y=1 left both ends of
    # the divider a board-length away from what they drive.
    # C2 became an 0603 (the basic-library 100nF 0402 is only rated 16V and this
    # one sits on V12P). Left in this zone it sorts largest-first, takes the
    # left-hand slot and pushes R1..R4 2.75mm right, which costs SD_CD its corridor
    # past J2. Its own slot on the right flank keeps the divider row where it was.
    ('V12P bypass', (16.3, 9.5, MIDX1, 13.2), 'T', ['C2']),
    # Listed in placement order, not designator order: the packer fills the row
    # left to right and C2 caps it at x=16.3, so four resistors do not fit in one
    # row and the last one wraps to row two. R3.2-R4.1 is a net (Q1's base), so
    # R3 and R4 must land side by side; wrapping R2 instead costs nothing because
    # both its pins leave by via anyway (Q2's gate and Q1's collector are on the
    # bottom face). With R4 wrapped, Q1_B failed identically in all 20 router runs.
    # v3 added C13 (Q2's gate-delay cap, electrically parallel with R1) and R15;
    # v4.1 moved R15 south to the CAN termination band because the GP28 hole it
    # feeds rotated to y42.4 with the Pico. C13 follows R2 into row two.
    ('power in', (MIDX0, 9.5, 16.1, 16.5), 'T',
     ['R1', 'R3', 'R4', 'R2', 'C13']),
    # C11 (SD bulk) moved up from the bottom face in v3, same reasoning as C6.
    # It goes FIRST: the zone is oversubscribed and whoever is last spills, and a
    # pull-up resistor cares less about distance than the bulk cap does.
    ('SD pull-ups + LED resistor', (MIDX0, 39.0, MIDX1, 42.3), 'T',
     ['C11', 'C10', 'R10', 'R11', 'R12', 'R13', 'R14']),
    SHUNT_ZONE,
    # v4: C15 (output bulk) packs into the strip under L1's V5 end. C14 and C16
    # are explicit above.
    # v4.2: C15 (output bulk) under L1's V5 pad, which now points -y (y20.5)
    ('5V buck caps', (4.2, 22.3, 9.5, 25.0), 'B', ['C15']),
    ('switch drive', (4.0, 30.5, 10.2, 42.0), 'B',
     ['Q1', 'D3']),
]

# GAP is the clear lane left between neighbouring courtyards. 0.5mm threads a
# 0.15mm track with clearance either side. Opening it to 0.7 was tried and made
# routing worse, not better: the parts spread out and every net got longer.
CLR, GAP, STEP, EDGE = 0.4, 0.5, 0.25, 0.5

# ------------------------------------------------------------------- board
board = pcbnew.BOARD()
ds = board.GetDesignSettings()
# MRP_LAYERS=2 tries the two-layer build (the v2 part reduction might fit);
# 4 is the proven stack. MRP_RULE overrides track/clearance - the 2-layer v1
# only closed at 0.127mm, JLCPCB's process floor.
LAYERS = int(os.environ.get('MRP_LAYERS', '4'))
assert LAYERS in (2, 4)
RULE = float(os.environ.get('MRP_RULE', '0.15'))
board.SetCopperLayerCount(LAYERS)
ds.SetBoardThickness(MM(1.6))

# 0.15mm track / 0.15mm clearance. JLCPCB's standard 2-layer process goes down to
# 0.127mm, so this keeps a margin while giving the router a 0.30mm channel pitch
# instead of 0.40mm - the difference between routable and not at 21mm wide.
ds.m_TrackMinWidth = MM(0.127)
ds.m_MinClearance = MM(RULE)
ds.m_CopperEdgeClearance = MM(0.3)
ds.m_ViasMinSize = MM(0.45)
ds.m_MinThroughDrill = MM(0.25)
ds.m_HoleToHoleMin = MM(0.25)
# A 0402 pad in a 15mm strip cannot resolve two thermal spokes, and neither can
# a header pin boxed in by tracks. One spoke carries this board's currents fine.
ds.m_MinResolvedSpokes = 1
nc = ds.m_NetSettings.GetDefaultNetclass()
nc.SetClearance(MM(RULE))
nc.SetTrackWidth(MM(RULE))
nc.SetViaDiameter(MM(0.6))
nc.SetViaDrill(MM(0.3))

for a, b, c, d in ((0, 0, W, 0), (W, 0, W, H), (W, H, 0, H), (0, H, 0, 0)):
    s = pcbnew.PCB_SHAPE(board)
    s.SetShape(pcbnew.SHAPE_T_SEGMENT)
    s.SetStart(V(a, b)); s.SetEnd(V(c, d))
    s.SetLayer(pcbnew.Edge_Cuts); s.SetWidth(MM(0.1))
    board.Add(s)

NETINFO = {}
for name in D.NETS:
    n = pcbnew.NETINFO_ITEM(board, name)
    board.Add(n)
    NETINFO[name] = n


def load(ref):
    lib, name = D.footprint_of(ref)
    fp = pcbnew.FootprintLoad(lib, name)
    assert fp is not None, f"{ref}: cannot load {lib}::{name}"
    return fp


def pad_bbox(fp):
    xs, ys = [], []
    for p in fp.Pads():
        b = p.GetBoundingBox()
        xs += [TOMM(b.GetLeft()), TOMM(b.GetRight())]
        ys += [TOMM(b.GetTop()), TOMM(b.GetBottom())]
    return min(xs), min(ys), max(xs), max(ys)


def keepout_bbox(fp):
    """Courtyard if the footprint has one, else the pad bbox with a little margin."""
    xs, ys = [], []
    for g in fp.GraphicalItems():
        if g.GetLayer() in (pcbnew.F_CrtYd, pcbnew.B_CrtYd):
            b = g.GetBoundingBox()
            xs += [TOMM(b.GetLeft()), TOMM(b.GetRight())]
            ys += [TOMM(b.GetTop()), TOMM(b.GetBottom())]
    if xs:
        return min(xs), min(ys), max(xs), max(ys)
    x0, y0, x1, y1 = pad_bbox(fp)
    return x0 - 0.25, y0 - 0.25, x1 + 0.25, y1 + 0.25


ANCHOR = {'pad': pad_bbox, 'keepout': keepout_bbox}


def place(ref, cx, cy, rot, side, anchor='pad'):
    """Put ref's anchor point on (cx, cy). Flip() reads the parent board, so the
    footprint has to be added before it can be flipped."""
    fp = load(ref)
    fp.SetReference(ref)
    fp.SetValue(D.PARTS[ref][2])
    board.Add(fp)
    if rot:
        fp.SetOrientationDegrees(rot)
    if side == 'B':
        fp.Flip(fp.GetPosition(), pcbnew.FLIP_DIRECTION_LEFT_RIGHT)
    if anchor == 'origin':
        fp.SetPosition(V(cx, cy))
        return fp
    x0, y0, x1, y1 = ANCHOR[anchor](fp)                # measured after transforms
    p = fp.GetPosition()
    fp.SetPosition(pcbnew.VECTOR2I(p.x + MM(cx - (x0 + x1) / 2),
                                   p.y + MM(cy - (y0 + y1) / 2)))
    return fp


FP, KEEP_BOTH, KEEP_T, KEEP_B = {}, [], [], []


def register(ref, fp, side):
    FP[ref] = fp
    for p in fp.Pads():                                # drilled holes block both faces
        if p.GetAttribute() in (pcbnew.PAD_ATTRIB_PTH, pcbnew.PAD_ATTRIB_NPTH):
            b = p.GetBoundingBox()
            KEEP_BOTH.append(((TOMM(b.GetLeft()) - CLR, TOMM(b.GetTop()) - CLR,
                               TOMM(b.GetRight()) + CLR, TOMM(b.GetBottom()) + CLR),
                              ref + ".hole"))
    if ref == 'U3':
        return                                          # elevated on socket strips
    x0, y0, x1, y1 = keepout_bbox(fp)
    (KEEP_B if side == 'B' else KEEP_T).append(
        ((x0 - CLR, y0 - CLR, x1 + CLR, y1 + CLR), ref))


for ref, (cx, cy, rot, side, anchor) in EXPLICIT.items():
    register(ref, place(ref, cx, cy, rot, side, anchor), side)


def clashes(side, x0, y0, x1, y1):
    """-> the ref of the first thing in the way, or None."""
    for b, who in KEEP_BOTH + (KEEP_B if side == 'B' else KEEP_T):
        if not (x1 < b[0] or x0 > b[2] or y1 < b[1] or y0 > b[3]):
            return who
    return None


def try_rect(ref, side, rx0, ry0, rx1, ry1, centre=None):
    # A left-right flip mirrors X about the anchor, so it changes neither the
    # keepout width nor its height - the probe does not need to be flipped.
    # Both orientations are tried: U2 leaves only 2.2mm either side, and a 1005
    # lying flat needs 2.37mm with its gap but only 1.47mm stood on end.
    kx0, ky0, kx1, ky1 = keepout_bbox(load(ref))
    DIM = {0: (kx1 - kx0, ky1 - ky0), 90: (ky1 - ky0, kx1 - kx0)}
    # Both orientations are offered at every position, not one orientation over
    # the whole zone first. Exhausting rot=0 first let C3 take a slot below U2
    # rather than standing on end in the 2.2mm gap beside it, purely because a
    # worse spot existed and was reachable without rotating.
    cands = []
    cy = ry0
    while cy <= ry1:
        cx = rx0
        while cx <= rx1:
            for rot, (w, h) in DIM.items():
                if cx + w <= rx1 and cy + h <= ry1:
                    cands.append((cy, cx, rot))
            cx += STEP
        cy += STEP
    if centre:
        mx, my = centre
        cands.sort(key=lambda c: (c[1] + DIM[c[2]][0] / 2 - mx) ** 2
                   + (c[0] + DIM[c[2]][1] / 2 - my) ** 2)
    blockers = {}
    for cy, cx, rot in cands:
        w, h = DIM[rot]
        box = (cx - GAP / 2, cy - GAP / 2, cx + w + GAP / 2, cy + h + GAP / 2)
        who = clashes(side, *box)
        if who:
            blockers[who] = blockers.get(who, 0) + 1
            continue
        register(ref, place(ref, cx + w / 2, cy + h / 2, rot, side, 'keepout'), side)
        return None
    return sorted(blockers, key=blockers.get, reverse=True)[:3] or ['zone too small']


def a(r):
    x0, y0, x1, y1 = keepout_bbox(load(r))
    return (x1 - x0) * (y1 - y0)


def dist(ref, want):
    x0, y0, x1, y1 = pad_bbox(FP[ref])
    return (((x0 + x1) / 2 - want[0]) ** 2 + ((y0 + y1) / 2 - want[1]) ** 2) ** 0.5


fail, far = [], []
for label, (zx0, zy0, zx1, zy1), side, members in ZONES:
    want = ((zx0 + zx1) / 2, (zy0 + zy1) / 2)
    for ref in sorted(members, key=a, reverse=True):          # largest first
        if try_rect(ref, side, zx0, zy0, zx1, zy1) is None:
            continue
        why = try_rect(ref, side, MIDX0, EDGE, MIDX1, H - EDGE, centre=want)
        if why is not None:
            fail.append(f"  NO ROOM: {ref} from {label} - blocked by {'/'.join(why)}")
            continue
        far.append(f"{ref} spilled out of [{label}], now {dist(ref, want):.1f}mm away")
assert not fail, "\n".join(fail)

missing = set(D.PARTS) - set(FP)
assert not missing, f"unplaced: {sorted(missing)}"

# v3: EVERY chip - both EAGLE sizes - must end up on the top face, so a person
# hand-soldering the small parts only ever works one side.
CHIPS = ('C1005', 'C1608')
stray = sorted(r for r in FP if D.eagle_pkg(r) in CHIPS and FP[r].IsFlipped())
assert not stray, f"chips on the BOTTOM: {stray}"

# A body may not sit on top of somebody else's drilled hole. Explicit placements
# skip the packer's clash test, so this is the net that catches them - it is how
# U1 was found sitting over the Pico's right-hand header column.
PTH = (pcbnew.PAD_ATTRIB_PTH, pcbnew.PAD_ATTRIB_NPTH)
holes = [(r, p.GetNumber(), TOMM(p.GetPosition().x), TOMM(p.GetPosition().y))
         for r, f in FP.items() for p in f.Pads() if p.GetAttribute() in PTH]
onhole = []
for ref, fp in FP.items():
    if ref == 'U3':
        continue                                   # elevated on socket strips
    x0, y0, x1, y1 = keepout_bbox(fp)
    for r, num, px, py in holes:
        if r != ref and x0 <= px <= x1 and y0 <= py <= y1:
            onhole.append(f"{ref} body covers the {r}.{num} hole at ({px:.2f},{py:.2f})")
assert not onhole, "BODY OVER A HOLE:\n  " + "\n  ".join(sorted(set(onhole)))

# nothing outside the outline (J1's housing deliberately overhangs y<0)
oob = []
for ref, fp in FP.items():
    x0, y0, x1, y1 = pad_bbox(fp)
    if x0 < -0.01 or x1 > W + 0.01 or y0 < -0.01 or y1 > H + 0.01:
        oob.append(f"{ref} pads ({x0:.2f},{y0:.2f})-({x1:.2f},{y1:.2f})")
assert not oob, "PADS OUTSIDE OUTLINE:\n  " + "\n  ".join(oob)

# ------------------------------------------------------- board-level corrections
# The Pico stands ~8.5mm off the board on socket strips (bottom face since v4.2),
# so its body occupies no board area at all - only its 40 header holes do. Keeping the stock courtyard
# would forbid the whole middle strip, which is the entire point of the layout.
# Its silkscreen is the module body outline, which is as wide as the board itself,
# so it runs off both edges and over J2's pads - it belongs on the fab layer too.
u3 = FP['U3']
KILL = (pcbnew.F_CrtYd, pcbnew.B_CrtYd, pcbnew.F_SilkS, pcbnew.B_SilkS)
dropped = [g for g in u3.GraphicalItems() if g.GetLayer() in KILL]
for g in dropped:
    u3.Remove(g)
# RaspberryPi_Pico_Common_THT carries the Pico W antenna keep-out. This design uses
# a plain Pico 2, which has no radio - drop it, and re-check if a W is ever fitted.
zones = list(u3.Zones())
for z in zones:
    u3.Remove(z)
print(f"U3: dropped {len(dropped)} courtyard/silk shapes and {len(zones)} keep-out zones")

# 21mm of width cannot carry legible silkscreen for 23 0402s. Reference designators
# live on the fabrication layer, which is what the assembly drawing and the JLCPCB
# CPL are generated from; silkscreen keeps only the polarity and pin-1 marks.
for ref, fp in FP.items():
    t = fp.Reference()
    t.SetLayer(pcbnew.B_Fab if fp.IsFlipped() else pcbnew.F_Fab)
    t.SetTextSize(pcbnew.VECTOR2I(MM(0.6), MM(0.6)))
    t.SetTextThickness(MM(0.1))
    fp.Value().SetVisible(False)

# ---------------------------------------------------------------------- nets
assigned = 0
for net, members in D.NETS.items():
    ni = NETINFO[net]
    for ref, pin in members:
        want = D.pads_of(ref, pin)
        hit = [p for p in FP[ref].Pads() if p.GetNumber() in want]
        assert hit, f"{ref} pin {pin}: no pad named {want}"
        for p in hit:
            p.SetNet(ni)
            assigned += 1

os.makedirs(OUTDIR, exist_ok=True)
pcbnew.SaveBoard(BRD, board)

top = sorted(r for r in FP if not FP[r].IsFlipped())
bot = sorted(r for r in FP if FP[r].IsFlipped())
print(f"wrote {BRD}")
print(f"board {W} x {H} mm = {W*H/100:.2f} cm2")
print(f"placed top={len(top)} bottom={len(bot)} total={len(FP)}")
print(f"nets={len(D.NETS)} pads netted={assigned}")
print(f"chips (1608): {sum(1 for r in FP if D.eagle_pkg(r) in CHIPS)}, all on TOP")
print("TOP   :", " ".join(top))
print("BOTTOM:", " ".join(bot))
if far:
    print("\nspilled out of their zone (still placed):")
    for s in far:
        print("  " + s)

for face, flipped in (("TOP", False), ("BOTTOM", True)):
    print(f"\n{face} occupancy (keepout boxes, y order):")
    for ref in sorted((r for r in FP if FP[r].IsFlipped() == flipped),
                      key=lambda r: keepout_bbox(FP[r])[1]):
        x0, y0, x1, y1 = keepout_bbox(FP[ref])
        print(f"  {ref:4s} x {x0:6.2f}..{x1:6.2f}  y {y0:6.2f}..{y1:6.2f}   "
              f"({x1-x0:.2f} x {y1-y0:.2f})")

print("\n12V chain, pin to pin along the flow:")


def pad_at(ref, nums):
    """Centroid of the named pads of ref, in board coordinates."""
    want = nums if isinstance(nums, (list, tuple)) else [nums]
    pts = [(TOMM(p.GetPosition().x), TOMM(p.GetPosition().y))
           for p in FP[ref].Pads() if p.GetNumber() in want]
    assert pts, f"{ref} has no pad in {want}"
    return sum(p[0] for p in pts) / len(pts), sum(p[1] for p in pts) / len(pts)


# Q2 pads follow the datasheet numbering since the v3.1 PADMAP fix: S=4, G=3,
# D=1/2/5/6 (the v1 boards had G/S mirrored - see PADMAP in mrp_design).
# v4: the chain ends at the buck's VIN (U4 pad 3).
CHAIN = [('V12_RAW', 'J1', ['8'], 'F1', ['1']),
         ('V12_FUSED', 'F1', ['2'], 'D1', ['2']),
         ('V12P', 'D1', ['1'], 'Q2', ['4']),
         ('V12_SW', 'Q2', ['1', '2', '5', '6'], 'U4', ['3'])]

# The lesson of v1: the footprint must agree with DS36685, not with the EAGLE
# library. Checked on every build from here on.
_q2 = {p.GetNumber(): p.GetNetname() for p in FP['Q2'].Pads()}
assert _q2 == {'1': 'V12_SW', '2': 'V12_SW', '3': 'Q2_G',
               '4': 'V12P', '5': 'V12_SW', '6': 'V12_SW'}, \
    f"Q2 pad map does not match the ZXMP6A17E6Q datasheet: {_q2}"
total = 0.0
for net, a, ap, c, cp in CHAIN:
    (x1, y1), (x2, y2) = pad_at(a, ap), pad_at(c, cp)
    d = ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5
    total += d
    print(f"  {net:<11} {a}.{'/'.join(ap):<9} ({x1:5.2f},{y1:5.2f}) -> "
          f"{c}.{'/'.join(cp):<4} ({x2:5.2f},{y2:5.2f})  {d:5.2f} mm")
print(f"  {'TOTAL':<11} {total:52.2f} mm")

# The buck's hot loop and its bypasses have to be TIGHT - measure, never assume.
print("\n5V buck loop, pad to pad:")
for label, a, ap, c, cp in [('SW node', 'U4', ['5'], 'L1', ['1']),
                            ('input cap', 'U4', ['3'], 'C14', ['1']),
                            ('output cap', 'L1', ['2'], 'C15', ['1']),
                            ('bootstrap', 'U4', ['6'], 'C16', ['2'])]:
    (x1, y1), (x2, y2) = pad_at(a, ap), pad_at(c, cp)
    d = ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5
    print(f"  {label:<10} {a}.{ap[0]} ({x1:5.2f},{y1:5.2f}) -> "
          f"{c}.{cp[0]} ({x2:5.2f},{y2:5.2f})  {d:5.2f} mm")

# U4's pad map, same discipline as Q2: datasheet DS41326 order, checked per build
_u4 = {p.GetNumber(): p.GetNetname() for p in FP['U4'].Pads()}
assert _u4 == {'1': 'V5', '2': 'V12_SW', '3': 'V12_SW',
               '4': 'GND', '5': 'SW_5V', '6': 'BST_5V'}, \
    f"U4 pad map does not match the AP63205 datasheet: {_u4}"

# v4.1: the Pico is rotated 180 deg. Three things must hold on every build:
# the USB end (pad 1's corner) faces the microSD edge, all eight ground
# barrels are still grounds (the grid's rotation symmetry maps them onto each
# other), and each peripheral sits on the pin U3_REMAP promised the firmware.
_u3 = {p.GetNumber(): (TOMM(p.GetPosition().y), p.GetNetname())
       for p in FP['U3'].Pads()}
assert _u3['1'][0] > 50, \
    f"U3 pad 1 (USB end) at y={_u3['1'][0]:.1f} - USB does not face the microSD edge"
for _g in ('3', '8', '13', '18', '23', '28', '33', '38'):
    assert _u3[_g][1] == 'GND', f"U3 pad {_g} lost its ground: {_u3[_g][1]!r}"
_want_u3 = dict(D.U3_REMAP, **{'34': 'IG_SENSE'})
_bad = {p: (_u3[p][1], n) for p, n in _want_u3.items() if _u3[p][1] != n}
assert not _bad, f"U3 pin map disagrees with U3_REMAP (pad: (is, want)): {_bad}"
_u3x = {p.GetNumber(): TOMM(p.GetPosition().x) for p in FP['U3'].Pads()}
assert FP['U3'].IsFlipped(), "v4.2: the Pico belongs on the BOTTOM face"
assert _u3x['1'] < 10.5 < _u3x['21'], \
    f"U3 mirror wrong: pad 1 at x={_u3x['1']:.2f}, pad 21 at x={_u3x['21']:.2f}"
print(f"\nU3: BOTTOM face, rotated 180 - USB end at y={_u3['1'][0]:.2f}, pad 1 column "
      f"x={_u3x['1']:.2f}, 8 GND barrels + {len(_want_u3)} peripheral pins verified")

print("\nclearance check at the connector end:")
pico_pads = pad_bbox(FP['U3'])
print(f"  Pico header pads span y {pico_pads[1]:.2f} .. {pico_pads[3]:.2f}")
print(f"  MQS housing overhangs the board to y = 8.00")
print(f"  gap = {pico_pads[1] - 8.0:.2f} mm")
for ref in ('Y1', 'J2', 'D6'):
    x0, y0, x1, y1 = pad_bbox(FP[ref])
    print(f"  {ref} pads ({x0:.2f},{y0:.2f})-({x1:.2f},{y1:.2f})")

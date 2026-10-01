# -*- coding: utf-8 -*-
"""Generate MotoRecoPico.kicad_sch, hand-laid-out for readability (v4.2).

gen_ksch3 packed each block into one row and auto-routed every local net
through horizontal bands below the parts. Electrically fine, visually a rope
factory - the power block alone had 17 parts in a row with nets snaking 100mm.
This generator draws the schematic the way the textbooks say to draw it
(signal flow left to right, higher potential up, GND symbols down, pull-ups
and decouplers vertical, hand-drawn wires inside a block, labels between
blocks), with every part position and every local wire written out by hand.

Correctness does NOT rest on the hand layout being right: the same checks as
before run on the result - an internal splice/short scan here, then
`kicad-cli sch export netlist` + compare_net.py proving the drawn circuit is
pin-for-pin identical to mrp_design.NETS, then ERC.

Coordinates: G = 1.27mm grid units, sheet y-DOWN. Symbol pin tables are y-UP;
a pin at local (px,py) on a symbol at (sx,sy,rot) lands at
sx + R(px,py).x, sy - R(px,py).y with R the CCW rotation.
"""
import sys, io, os, math, hashlib
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mrp_design as D
import ksym
import ksym_map as M
from sexp import dump, q

OUT = r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.kicad_sch"
G = 1.27

POWER = {'GND': 'power:GND', 'V3V3': 'power:+3V3', 'V5': 'power:+5V'}
# V5 is driven through L1 (passive pin - the buck's power output is the SW
# node), and V12_SW / VSYS through Q2 / D3: ERC needs flags on all three.
FLAGS = {'#FLG01': 'V12_SW', '#FLG02': 'VSYS', '#FLG03': 'V5'}

SYMMAP = dict(M.SYMMAP)
for f in FLAGS:
    SYMMAP[f] = 'power:PWR_FLAG'

# ------------------------------------------------------------------ placement
# ref: (x, y, rot) in grid units. Rotations are CCW like KiCad's.
P = {
    # ---- vehicle input / high-side switch / buck (top strip, 12V rail y=33)
    'J1':  (24, 41, 180),    # pins exit right; pin 8 (+12V) top, NCs bottom
    'F1':  (36, 33, 90),     # horizontal, pin 1 (from J1) left
    'D1':  (46, 33, 180),    # anode left - power flows left to right
    'D2':  (56, 39, 270),    # TVS, K up to the rail
    'C2':  (66, 39, 0),      # bypass, pin 1 up
    'Q2':  (82, 42, 180),    # S top on the rail, D bottom, G right
    'R1':  (94, 39, 0),      # gate pull-up, V12P top / gate line bottom
    'C13': (104, 39, 180),   # gate delay cap, pin 1 (gate) bottom
    'R2':  (90, 46, 0),      # gate line down to Q1's collector
    'Q1':  (88, 62, 0),      # IG switch, emitter to GND (below C14's ground)
    'R3':  (74, 62, 90),     # IG_RAW in from the left
    'R15': (79, 68, 0),      # IG sense tap, label below
    'R4':  (82, 68, 0),      # base pull-down to GND
    'U4':  (104, 52, 0),     # buck; IN/EN left, SW/BST/FB right
    'C14': (86, 53, 0),      # input cap on the V12_SW run
    'C16': (123, 52, 270),   # bootstrap cap in the BST row, pin 1 (SW) right
    'L1':  (130, 50, 90),    # horizontal, pin 1 (SW) left
    'C15': (137, 53, 0),     # output bulk under the V5 wire
    'D3':  (148, 50, 180),   # V5 -> VSYS, anode left
    '#FLG01': (93, 50, 0),
    '#FLG03': (141, 50, 0),
    '#FLG02': (154, 50, 0),
    # ---- CAN (left column, below the power strip)
    'U2':  (55, 115, 0),
    'C3':  (78, 88, 0), 'C4': (84, 88, 0), 'C5': (90, 88, 0), 'C6': (96, 88, 0),
    'R7':  (36, 107, 270),     # TxnRTS common pull-up, sideways in the free row
    'R6':  (100, 120, 180),  # RESET pull-up (free-standing, label style)
    'R8':  (86, 102, 0),     # 120R termination across CANH/CANL
    'Y1':  (76, 120, 270),   # crystal vertical between the OSC rows
    'C7':  (86, 117, 90),    # load caps, GND off to the right
    'C8':  (86, 123, 90),
    # ---- MCU (centre)
    'U3':  (150, 118, 0),
    # ---- microSD (right column)
    'J2':  (232, 105, 0),
    'R10': (180, 92, 180), 'R11': (186, 92, 180), 'R12': (192, 92, 180),
    'R13': (198, 92, 180),   # pull-up bank; SD_DAT12 joins J2 by name
    'C10': (196, 124, 0), 'C11': (202, 124, 0),
    # ---- GPS + status LED (bottom right)
    'J3':  (300, 160, 0),    # pins exit left, into the sheet
    'R14': (250, 158, 0),    # +5V on top
    'D6':  (250, 167, 90),   # LED below, cathode down to the GPIO label
}
assert sorted(P) == sorted(list(D.PARTS) + list(FLAGS)), \
    f"placement mismatch: {set(P) ^ set(list(D.PARTS) + list(FLAGS))}"

PINS = {r: ksym.pins(SYMMAP[r]) for r in P}


def rot_ccw(px, py, rot):
    return {0: (px, py), 90: (-py, px), 180: (-px, -py), 270: (py, -px)}[rot]


def pin_xy(ref, num):
    sx, sy, rot = P[ref]
    for n, _nm, px, py, prot, _et in PINS[ref]:
        if n == num:
            rx, ry = rot_ccw(px / G, py / G, rot)
            return sx + rx, sy - ry, (prot + rot) % 360
    raise KeyError(f"{ref}.{num}")


# ------------------------------------------------------------- hand-drawn nets
# Each entry: net -> list of wire segments ((x1,y1),(x2,y2)) in grid units.
# Every pin of these nets is covered here; the auto stub pass skips them.
HAND = {
    'V12_RAW':   [((28, 33), (33, 33))],
    'V12_FUSED': [((39, 33), (43, 33))],
    # the 12V rail with its shunt branches (D2, C2), Q2's source drop and the
    # gate pull-up branches (R1, C13)
    'V12P':      [((49, 33), (104, 33)), ((56, 33), (56, 36)), ((66, 33), (66, 36)),
                  ((80, 33), (80, 36)), ((94, 33), (94, 36)), ((104, 33), (104, 36))],
    # Q2.D down, across to the buck; EN tied to VIN; C14 and the ERC flag tap in
    'V12_SW':    [((80, 48), (80, 50)), ((80, 50), (96, 50)),
                  ((94, 50), (94, 54)), ((94, 54), (96, 54))],
    'Q2_G':      [((88, 42), (104, 42)), ((90, 42), (90, 43))],
    'Q1_C':      [((90, 49), (90, 58))],
    'Q1_B':      [((77, 62), (84, 62)), ((79, 62), (79, 65)), ((82, 62), (82, 65))],
    'SW_5V':     [((112, 50), (127, 50)), ((126, 52), (126, 50))],
    'BST_5V':    [((112, 52), (120, 52))],
    'V5':        [((133, 50), (145, 50)), ((143, 50), (143, 48)),
                  # FB ties to VOUT; drawn as a short +5V tap pointing down
                  ((112, 54), (114, 54))],
    'VSYS':      [((151, 50), (158, 50))],
    # J1's ground: right, then down at x44, past the label texts
    'GND':       [((28, 41), (44, 41)), ((44, 41), (44, 45)),
                  # J3's ground: left below the labels, then down
                  ((296, 158), (281, 158)), ((281, 158), (281, 166))],
    'V3V3':      [((296, 156), (290, 156)), ((290, 156), (290, 151))],
    'CANH':      [((67, 99), (86, 99))],
    'CANL':      [((67, 101), (82, 101)), ((82, 101), (82, 105)),
                  ((82, 105), (86, 105))],
    'TXCAN':     [((67, 107), (72, 107)), ((72, 107), (72, 111)),
                  ((72, 111), (67, 111))],
    'RXCAN':     [((67, 105), (74, 105)), ((74, 105), (74, 113)),
                  ((74, 113), (67, 113))],
    'TXNRTS':    [((43, 109), (39, 109)), ((43, 111), (39, 111)),
                  ((43, 113), (39, 113)), ((39, 107), (39, 113))],
    'XTAL1':     [((67, 117), (83, 117))],
    'XTAL2':     [((67, 123), (83, 123))],
    'LED_A':     [((250, 161), (250, 164))],
}
# pins whose connection the HAND wires already make: the auto pass must not
# add a second stub for them
HANDLED = {
    ('J1', '8'), ('F1', '1'), ('F1', '2'), ('D1', '1'), ('D1', '2'),
    ('D2', '1'), ('C2', '1'), ('Q2', '4'), ('R1', '1'), ('C13', '2'),
    ('Q2', '1'), ('Q2', '2'), ('Q2', '5'), ('Q2', '6'),
    ('U4', '2'), ('U4', '3'), ('C14', '1'),
    ('Q2', '3'), ('R1', '2'), ('C13', '1'), ('R2', '1'),
    ('R2', '2'), ('Q1', '3'),
    ('R3', '2'), ('Q1', '1'), ('R15', '1'), ('R4', '1'),
    ('U4', '5'), ('L1', '1'), ('C16', '1'),
    ('U4', '6'), ('C16', '2'),
    ('L1', '2'), ('C15', '1'), ('U4', '1'), ('D3', '2'),
    ('D3', '1'),
    ('J1', '4'), ('J3', '2'), ('J3', '1'),
    ('J1', '6'), ('R8', '1'), ('J1', '5'), ('R8', '2'),
    ('U2', '20'), ('U2', '24'), ('U2', '21'), ('U2', '28'),
    ('U2', '6'), ('U2', '7'), ('U2', '23'), ('R7', '1'),
    ('U2', '9'), ('Y1', '1'), ('C7', '1'), ('U2', '8'), ('Y1', '2'), ('C8', '1'),
    ('R14', '2'), ('D6', '2'),
}
# labels dropped onto hand wires (net, x, y, angle) - CANH/CANL reach J1 by
# name, VSYS reaches the Pico
HAND_LABELS = [
    ('CANH', 74, 99, 0), ('CANL', 74, 101, 0), ('VSYS', 158, 50, 0),
    ('CANH', 36, 37, 0), ('CANL', 36, 39, 0),
]
# extra J1-side label wires (stubs drawn by hand so their lengths differ)
HAND['CANH'].append(((28, 37), (36, 37)))
HAND['CANL'].append(((28, 39), (36, 39)))
HANDLED |= {('J1', '6'), ('J1', '5')}
# power symbols sitting on hand wires (lib, x, y)
HAND_POWER = [('power:GND', 44, 45, 0), ('power:GND', 281, 166, 0),
              ('power:+3V3', 290, 151, 0), ('power:+5V', 143, 48, 0),
              ('power:+5V', 114, 54, 180)]
# ERC flags attach where placed; their single pin is at the anchor
FLAG_AT = {f: P[f][:2] for f in FLAGS}

TITLES = [
    ("VEHICLE 12V IN - IG HIGH-SIDE SWITCH - 5V BUCK", 20, 24),
    ("CAN  MCP25625", 20, 82),
    ("MCU  Raspberry Pi Pico 2", 122, 82),
    ("microSD  DM3AT", 186, 82),
    ("GPS  GT-502MGG-N", 274, 144),
    ("STATUS LED", 242, 146),
]
NOTES = [
    ("IG OFF -> C13 holds Q2 on ~0.5s (log close grace)", 96, 68),
    ("decoupling at U2", 74, 84),
    ("SD pull-ups", 178, 88),
]

# --------------------------------------------------------------- net plumbing
NETPINS = {}
for net in D.NETS:
    NETPINS[net] = sorted(M.netpins(net))
for f, net in FLAGS.items():
    NETPINS[net].append((f, '1'))

wires, junctions, labels, powersyms, noconn = [], [], [], [], []
for net, segs in HAND.items():
    for (x1, y1), (x2, y2) in segs:
        wires.append((x1 * G, y1 * G, x2 * G, y2 * G, net))
for net, x, y, a in HAND_LABELS:
    labels.append((net, x * G, y * G, a, 'left' if a == 0 else 'right'))
for lib, x, y, r in HAND_POWER:
    powersyms.append((lib, x * G, y * G, r))

STUB_P, STUB_L = 2 * G, 4 * G


def esc(px, py, rot, length):
    a = math.radians(rot + 180)
    return px + length * round(math.cos(a)), py - length * round(math.sin(a))


for net, pins in NETPINS.items():
    for ref, num in pins:
        if (ref, num) in HANDLED or ref in FLAGS:
            continue
        px, py, rot = pin_xy(ref, num)
        px, py = px * G, py * G
        if net in POWER:
            ex, ey = esc(px, py, rot, STUB_P)
            if (px, py) != (ex, ey):
                wires.append((px, py, ex, ey, net))
            powersyms.append((POWER[net], ex, ey, 0))
        else:
            ex, ey = esc(px, py, rot, STUB_L)
            if (px, py) != (ex, ey):
                wires.append((px, py, ex, ey, net))
            if ey > py:            # downward stub: vertical text below
                labels.append((net, ex, ey, 90, 'right'))
            elif ey < py:          # upward stub
                labels.append((net, ex, ey, 90, 'left'))
            else:
                labels.append((net, ex, ey, 0, 'left' if ex > px else 'right'))

# flags: pin at anchor point, must coincide with a wire of their net
for f, net in FLAGS.items():
    fx, fy = FLAG_AT[f]
    junctions.append((fx * G, fy * G))

# ------------------------------------------------ junctions where a T happens
END = {}
for x1, y1, x2, y2, net in wires:
    for p in ((x1, y1), (x2, y2)):
        END.setdefault((round(p[0], 3), round(p[1], 3)), []).append(net)
PIN_PT = {}
for r in P:
    if r in FLAGS:
        continue
    for n, *_ in PINS[r]:
        x, y, _ = pin_xy(r, n)
        PIN_PT.setdefault((round(x * G, 3), round(y * G, 3)), []).append((r, n))


def on_interior(px, py, x1, y1, x2, y2):
    if (px, py) in ((x1, y1), (x2, y2)):
        return False
    if abs(x1 - x2) < 1e-6:
        return abs(px - x1) < 1e-6 and min(y1, y2) < py < max(y1, y2)
    if abs(y1 - y2) < 1e-6:
        return abs(py - y1) < 1e-6 and min(x1, x2) < px < max(x1, x2)
    return False


NET_OF_PIN = {(r, n): net for net, v in NETPINS.items() for r, n in v}
for (px, py), owners in list(END.items()) + \
        [(k, [NET_OF_PIN.get(p) for p in v]) for k, v in PIN_PT.items()]:
    for x1, y1, x2, y2, net in wires:
        if on_interior(px, py, x1, y1, x2, y2):
            if all(o == net for o in owners if o):
                junctions.append((px, py))
            else:
                raise AssertionError(
                    f"{owners} touches the middle of a {net} wire at ({px:g},{py:g})")
junctions = sorted(set((round(x, 3), round(y, 3)) for x, y in junctions))

# --------------------------------------- prove the drawing IS mrp_design.NETS
# 1) no two nets share a wire endpoint
for p, nets in END.items():
    assert len(set(nets)) == 1, f"nets {set(nets)} meet at {p}"
# 2) every pin of a hand net touches a wire of that net; every hand net is one
#    connected component
import collections
for net, segs in HAND.items():
    pts = {}
    parent = {}

    def find(a):
        while parent[a] != a:
            parent[a] = parent[parent[a]]
            a = parent[a]
        return a

    def union(a, b):
        parent.setdefault(a, a); parent.setdefault(b, b)
        parent[find(a)] = find(b)

    seglist = [(x1 * G, y1 * G, x2 * G, y2 * G) for (x1, y1), (x2, y2) in segs]
    for i, (x1, y1, x2, y2) in enumerate(seglist):
        parent.setdefault(i, i)
    for i, a in enumerate(seglist):
        for j, b in enumerate(seglist):
            if i < j:
                touch = False
                for p in ((a[0], a[1]), (a[2], a[3])):
                    if p in ((b[0], b[1]), (b[2], b[3])) or on_interior(*p, *b):
                        touch = True
                for p in ((b[0], b[1]), (b[2], b[3])):
                    if on_interior(*p, *a):
                        touch = True
                if touch:
                    union(i, j)
    have_label = any(hl[0] == net for hl in HAND_LABELS) or \
        any(not ((r, n) in HANDLED or r in FLAGS) for r, n in NETPINS[net])
    comp_of_pin = set()
    for ref, num in NETPINS[net]:
        if (ref, num) not in HANDLED and ref not in FLAGS:
            continue
        if ref in FLAGS:
            px, py = (FLAG_AT[ref][0] * G, FLAG_AT[ref][1] * G)
        else:
            x, y, _ = pin_xy(ref, num)
            px, py = x * G, y * G
        hit = [i for i, s in enumerate(seglist)
               if (px, py) in ((s[0], s[1]), (s[2], s[3])) or on_interior(px, py, *s)]
        assert hit, f"{net}: {ref}.{num} at ({px/G:g},{py/G:g})G touches no wire"
        comp_of_pin.add(find(hit[0]))
    assert len(comp_of_pin) <= 1 or have_label, \
        f"{net}: hand wires form {len(comp_of_pin)} separate islands"
# 3) labels sit on wires of their own net
for net, x, y, a in HAND_LABELS:
    px, py = x * G, y * G
    ok = any(nn == net and ((px, py) in ((x1, y1), (x2, y2))
                            or on_interior(px, py, x1, y1, x2, y2))
             for x1, y1, x2, y2, nn in wires)
    assert ok, f"label {net} at ({x},{y})G is not on a {net} wire"
# 4) no wire runs across a foreign pin, no foreign wires splice
shorts = []
for x1, y1, x2, y2, net in wires:
    for (px, py), pl in PIN_PT.items():
        if on_interior(px, py, x1, y1, x2, y2):
            for r, n in pl:
                if NET_OF_PIN.get((r, n)) != net:
                    shorts.append(f"{net} wire runs across {r}.{n}")
    for a1, b1, a2, b2, other in wires:
        if other != net:
            for ex, ey in ((a1, b1), (a2, b2)):
                if on_interior(ex, ey, x1, y1, x2, y2):
                    shorts.append(f"{net} spliced by {other} at ({ex/G:g},{ey/G:g})G")
assert not shorts, "SCHEMATIC SHORT:\n  " + "\n  ".join(sorted(set(shorts))[:15])

wired = {(r, n) for v in NETPINS.values() for r, n in v}
noconn = [(r, n) for r in P if r not in FLAGS
          for n, *_ in PINS[r] if (r, n) not in wired]

# ------------------------------------------------------------------- emit
SHEET_UUID = None


def uid(*p):
    h = hashlib.md5(("mrp4:" + ":".join(map(str, p))).encode()).hexdigest()
    return f"{h[0:8]}-{h[8:12]}-{h[12:16]}-{h[16:20]}-{h[20:32]}"


SHEET_UUID = uid('sheet')
EFF = '(effects (font (size 1.27 1.27)))'


def prop(name, val, x, y, hide=False, just=None):
    j = f' (justify {just})' if just else ''
    h = ' (hide yes)' if hide else ''
    return (f'(property {q(name)} {q(val)} (at {x:g} {y:g} 0) '
            f'(effects (font (size 1.27 1.27)){j}{h}))')


def sym_block(ref, lib_id, sx, sy, rot, val, fp, tag=""):
    pins_txt = "\n\t\t".join(f'(pin {q(n)} (uuid {q(uid(tag, ref, "p", n))}))'
                             for n, *_ in ksym.pins(lib_id))
    DEFAULT_V = ('Device:R', 'Device:C', 'Device:L', 'Device:Polyfuse')
    small = lib_id.startswith(('Device:', 'Diode:', 'power:'))
    vertical = (lib_id in DEFAULT_V) == (rot in (0, 180))
    # per-part exceptions where the default text spot lands on a wire or a
    # neighbour: chosen by looking at the rendered sheet, not computed
    OVR = {'C14': 'left', 'R15': 'left', 'C16': 'below'}
    if tag or ref in FLAGS:                     # power symbols and flags
        below = lib_id.endswith(':GND') != (rot == 180)
        props = [prop("Reference", ref, sx, sy, hide=True),
                 prop("Value", val, sx, sy + (5.6 if below else -3.6),
                      hide=lib_id.endswith('PWR_FLAG'))]
    elif OVR.get(ref) == 'left':
        props = [prop("Reference", ref, sx - 2.1, sy - 1.4, just='right'),
                 prop("Value", val, sx - 2.1, sy + 1.4, just='right')]
    elif OVR.get(ref) == 'below':
        props = [prop("Reference", ref, sx, sy + 2.9),
                 prop("Value", val, sx, sy + 5.4)]
    elif small and vertical:                    # text beside the body
        props = [prop("Reference", ref, sx + 2.1, sy - 1.4, just='left'),
                 prop("Value", val, sx + 2.1, sy + 1.4, just='left')]
    elif small:                                 # horizontal: above and below
        props = [prop("Reference", ref, sx, sy - 2.7),
                 prop("Value", val, sx, sy + 2.9)]
    elif ref == 'Q2':                           # text above the rail, clear of C2/R1
        props = [prop("Reference", ref, sx + 1.5, sy - 15.2, just='left'),
                 prop("Value", val, sx + 1.5, sy - 12.7, just='left')]
    elif ref == 'Q1':
        props = [prop("Reference", ref, sx + 6.5, sy - 1.4, just='left'),
                 prop("Value", val, sx + 6.5, sy + 1.4, just='left')]
    else:                                       # ICs and connectors: on top
        top = max(p[3] for p in PINS[ref]) / G
        props = [prop("Reference", ref, sx - 4, sy - (top + 4) * G, just='left'),
                 prop("Value", val, sx + 8, sy - (top + 4) * G, just='left')]
    fp_line = f'\t\t{prop("Footprint", fp, sx, sy, hide=True)}\n' if fp else ""
    return f'''\t(symbol
\t\t(lib_id {q(lib_id)})
\t\t(at {sx:g} {sy:g} {rot})
\t\t(unit 1)
\t\t(exclude_from_sim no)
\t\t(in_bom {"no" if tag or ref in FLAGS else "yes"})
\t\t(on_board {"no" if tag or ref in FLAGS else "yes"})
\t\t(dnp no)
\t\t(uuid {q(uid(tag, ref))})
\t\t{props[0]}
\t\t{props[1]}
{fp_line}\t\t{pins_txt}
\t\t(instances
\t\t\t(project "MotoRecoPico"
\t\t\t\t(path {q("/" + SHEET_UUID)}
\t\t\t\t\t(reference {q(ref)})
\t\t\t\t\t(unit 1)
\t\t\t\t)
\t\t\t)
\t\t)
\t)'''


body = []
for ref in P:
    sx, sy, rot = P[ref]
    if ref in FLAGS:
        body.append(sym_block(ref, SYMMAP[ref], sx * G, sy * G, 0, "PWR_FLAG", ""))
        continue
    lib, name = D.footprint_of(ref)
    fp = os.path.basename(lib).replace('.pretty', '') + ":" + name
    body.append(sym_block(ref, SYMMAP[ref], sx * G, sy * G, rot,
                          D.PARTS[ref][2] or ref, fp))
for i, (lib_id, x, y, r) in enumerate(powersyms):
    body.append(sym_block(f"#PWR{i:03d}", lib_id, x, y, r,
                          lib_id.split(':')[1], "", tag="pwr"))
for i, (x1, y1, x2, y2, _n) in enumerate(wires):
    body.append(f'\t(wire (pts (xy {x1:g} {y1:g}) (xy {x2:g} {y2:g})) '
                f'(stroke (width 0) (type default)) (uuid {q(uid("w", i))}))')
for i, (x, y) in enumerate(junctions):
    body.append(f'\t(junction (at {x:g} {y:g}) (diameter 0) (color 0 0 0 0) '
                f'(uuid {q(uid("j", i))}))')
for i, (name, x, y, a, just) in enumerate(labels):
    body.append(f'\t(label {q(name)} (at {x:g} {y:g} {a}) '
                f'(effects (font (size 1.27 1.27)) (justify {just} bottom)) '
                f'(uuid {q(uid("l", i))}))')
for ref, num in noconn:
    x, y, _ = pin_xy(ref, num)
    body.append(f'\t(no_connect (at {x * G:g} {y * G:g}) (uuid {q(uid("nc", ref, num))}))')
for i, (title, x, y) in enumerate(TITLES):
    body.append(f'\t(text {q(title)} (at {x * G:g} {y * G:g} 0) '
                f'(effects (font (size 2.2 2.2) (bold yes)) (justify left)) '
                f'(uuid {q(uid("t", i))}))')
for i, (note, x, y) in enumerate(NOTES):
    body.append(f'\t(text {q(note)} (at {x * G:g} {y * G:g} 0) '
                f'(effects (font (size 1.27 1.27) (italic yes)) (justify left)) '
                f'(uuid {q(uid("n", i))}))')

libs = "\n".join("\t\t" + dump(ksym.definition(l), 2)
                 for l in sorted(set(SYMMAP.values()) | set(POWER.values())))
open(OUT, 'w', encoding='utf-8', newline='\n').write(f'''(kicad_sch
\t(version 20251024)
\t(generator "mrp")
\t(generator_version "10.0")
\t(uuid {q(uid("root"))})
\t(paper "A3")
\t(lib_symbols
{libs}
\t)
{chr(10).join(body)}
\t(sheet_instances
\t\t(path "/" (page "1"))
\t)
\t(embedded_fonts no)
)
''')
print(f"wrote {OUT}")
print(f"{len(wires)} wires, {len(junctions)} junctions, {len(labels)} labels, "
      f"{len(powersyms)} power symbols, {len(noconn)} no-connects")

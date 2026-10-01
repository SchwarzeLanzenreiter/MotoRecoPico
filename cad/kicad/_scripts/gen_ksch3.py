# -*- coding: utf-8 -*-
"""Generate MotoRecoPico.kicad_sch with the Pico at the centre.

Blocks sit either side of the MCU, on the side whose header pins they actually
land on: CAN, microSD and the status LED on the left column, the vehicle input
and GPS on the right. Support parts - decoupling, dividers, pull-ups - stand
immediately beside the device they belong to, nearest first.

Each block's wiring is confined to that block's own rectangle: a clear band just
above and below its row of symbols for sideways runs, and a trunk channel under
it. That containment is what lets blocks stack vertically without a riser from
one block spearing a pin in another.

Sheet coordinates run y-DOWN, symbol libraries y-UP: a pin at symbol-local
(px,py) on a symbol at (sx,sy) lands at (sx+px, sy-py), never rounded.
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
STUB_P, STUB_L, STUB_W = 2 * G, 4 * G, 2 * G
PITCH = 4 * G                    # clear space each side of a support part

POWER = {'GND': 'power:GND', 'V3V3': 'power:+3V3', 'V5': 'power:+5V'}
# V5 joined the list in v4: it is now driven through L1 (a passive pin, since
# the buck's power stage output is the SW node), so ERC needs the flag to know
# the rail is externally driven.
FLAGS = {'#FLG01': 'V12_SW', '#FLG02': 'VSYS', '#FLG03': 'V5'}

# (title, anchor, support parts nearest-first, column, row)
BLOCKS = [
    ("CAN INTERFACE  MCP25625", 'U2',
     ['C3', 'C4', 'C5', 'R6', 'R7', 'Y1', 'C7', 'C8', 'R8'], 'L', 0),
    ("microSD  DM3AT", 'J2', ['C11', 'C10', 'R10', 'R11', 'R12', 'R13'], 'L', 1),
    ("STATUS LED", 'D6', ['R14'], 'L', 2),
    ("MCU  Raspberry Pi Pico 2", 'U3', [], 'C', 0),
    ("VEHICLE INPUT / IG SWITCH / 5V BUCK", 'U4',
     ['C6', 'D3', 'J1', 'F1', 'D2', 'Q2', 'D1', 'C2', 'Q1', 'R1', 'R2',
      'R3', 'R4', 'C13', 'R15', 'L1', 'C14', 'C15', 'C16',
      '#FLG01', '#FLG02', '#FLG03'], 'R', 0),
    ("GPS  GT-502MGG-N", 'J3', [], 'R', 1),
]

SYMMAP = dict(M.SYMMAP)
for f in FLAGS:
    SYMMAP[f] = 'power:PWR_FLAG'
PARTS = [b[1] for b in BLOCKS] + [r for b in BLOCKS for r in b[2]]
assert sorted(PARTS) == sorted(list(D.PARTS) + list(FLAGS)), \
    f"block membership mismatch: {set(PARTS) ^ set(list(D.PARTS) + list(FLAGS))}"
PINS = {r: ksym.pins(SYMMAP[r]) for r in PARTS}
BLOCK_OF = {r: i for i, b in enumerate(BLOCKS) for r in [b[1]] + b[2]}


def uid(*p):
    h = hashlib.md5(("mrp3:" + ":".join(map(str, p))).encode()).hexdigest()
    return f"{h[0:8]}-{h[8:12]}-{h[12:16]}-{h[16:20]}-{h[20:32]}"


def extent(ref):
    xs = [p[2] for p in PINS[ref]] or [0]
    ys = [p[3] for p in PINS[ref]] or [0]
    return max(xs) - min(xs), max(ys) - min(ys), max(ys)


# ------------------------------------------------------------ net membership
NETPINS = {}
for net, members in D.NETS.items():
    seen, lst = set(), []
    for ref, epin in members:
        for num in D.pads_of(ref, epin):
            if any(n == num for n, *_ in PINS[ref]) and (ref, num) not in seen:
                seen.add((ref, num)); lst.append((ref, num))
    NETPINS[net] = lst
for f, net in FLAGS.items():
    NETPINS[net].append((f, '1'))

KIND = {net: 'power' if net in POWER else
        ('local' if len({BLOCK_OF[r] for r, _ in p}) == 1 else 'label')
        for net, p in NETPINS.items()}

# ------------------------------------------- lay each block out on its own
local, size, bands = {}, {}, {}
for bi, (title, anchor, sats, col, row) in enumerate(BLOCKS):
    aw, ah, aymax = extent(anchor)
    pos = {anchor: (round((aw / 2 + PITCH) / G) * G, aymax)}
    x = aw + 2 * PITCH
    for r in sats:
        sw, sh, symax = extent(r)
        x += round((sw / 2 + PITCH) / G) * G
        pos[r] = (round(x / G) * G, symax)
        x += round((sw / 2 + PITCH) / G) * G
    local[bi] = pos
    depth = max(extent(r)[1] for r in pos)
    nloc = sum(1 for n, k in KIND.items() if k == 'local'
               and BLOCK_OF[NETPINS[n][0][0]] == bi)
    top, bot = -6 * G, depth + 4 * G
    bands[bi] = (top, bot, bot + 4 * G)
    size[bi] = (x + PITCH, bot + 4 * G + nloc * 2 * G + 6 * G - top)

# ------------------------------------- stack the blocks into three columns
colw = {}
for c in 'LCR':
    colw[c] = max([size[i][0] for i, b in enumerate(BLOCKS) if b[3] == c] or [0])
GAPX, GAPY, MARGIN = 16 * G, 12 * G, 20.0
colx = {'L': MARGIN,
        'C': MARGIN + colw['L'] + GAPX,
        'R': MARGIN + colw['L'] + GAPX + colw['C'] + GAPX}

place, origin = {}, {}
coly = {c: MARGIN + 10 * G for c in 'LCR'}
for bi, (title, anchor, sats, col, row) in enumerate(BLOCKS):
    ox, oy = colx[col], coly[col] - bands[bi][0]
    origin[bi] = (ox, oy)
    for r, (lx, ly) in local[bi].items():
        place[r] = (ox + lx, oy + ly)
    coly[col] += size[bi][1] + GAPY
SHEET_W = colx['R'] + colw['R'] + MARGIN
SHEET_H = max(coly.values()) + MARGIN


def pin_xy(ref, num):
    sx, sy = place[ref]
    for n, _nm, px, py, rot, _et in PINS[ref]:
        if n == num:
            return sx + px, sy - py, rot
    raise KeyError(f"{ref}.{num}")


def esc(px, py, rot, length):
    a = math.radians(rot + 180)
    return px + length * round(math.cos(a)), py - length * round(math.sin(a))


# ------------------------------------------------------------------- wiring
wires, junctions, labels, powersyms, noconn = [], [], [], [], []


def add(x1, y1, x2, y2, net):
    if (x1, y1) != (x2, y2):
        wires.append((x1, y1, x2, y2, net))


for net, pins in NETPINS.items():
    if KIND[net] == 'power':
        for ref, num in pins:
            px, py, rot = pin_xy(ref, num)
            ex, ey = esc(px, py, rot, STUB_P)
            add(px, py, ex, ey, net)
            powersyms.append((POWER[net], ex, ey))
    elif KIND[net] == 'label':
        for ref, num in pins:
            px, py, rot = pin_xy(ref, num)
            ex, ey = esc(px, py, rot, STUB_L)
            add(px, py, ex, ey, net)
            labels.append((net, ex, ey, rot))

lane_used = {round(pin_xy(r, n)[0] / G): 'pin' for r in PARTS for n, *_ in PINS[r]}
for _x1, _y1, _x2, _y2, _n in wires:
    lane_used.setdefault(round(_x2 / G), 'stub')


def free_lane(near, step):
    k = round(near / G)
    while k in lane_used:
        k += step
    lane_used[k] = 'lane'
    return k * G


for bi, (title, anchor, sats, col, row) in enumerate(BLOCKS):
    ox, oy = origin[bi]
    top, bot, ch = (oy + v for v in bands[bi])
    nets = [n for n, k in KIND.items() if k == 'local'
            and BLOCK_OF[NETPINS[n][0][0]] == bi]
    for i, net in enumerate(nets):
        ty = ch + i * 2 * G
        lanes = []
        for ref, num in NETPINS[net]:
            px, py, rot = pin_xy(ref, num)
            if rot in (0, 180):
                step = -1 if rot == 0 else 1
                lx = free_lane(px + step * STUB_W, step)
                add(px, py, lx, py, net)
            else:
                cy = top if rot == 270 else bot
                lx = free_lane(px + G, 1)
                add(px, py, px, cy, net)
                add(px, cy, lx, cy, net)
                add(lx, cy, lx, py, net)
            add(lx, py, lx, ty, net)
            lanes.append(lx)
            junctions.append((lx, ty))
        add(min(lanes), ty, max(lanes), ty, net)
        labels.append((net, min(lanes), ty, 0))

wired = {(r, n) for v in NETPINS.values() for r, n in v}
noconn = [(r, n) for r in PARTS for n, *_ in PINS[r] if (r, n) not in wired]

# -------------------------------------------------- prove nothing is shorted
PIN_AT = {(round(pin_xy(r, n)[0], 4), round(pin_xy(r, n)[1], 4)): (r, n)
          for r in PARTS for n, *_ in PINS[r]}
NET_OF = {(r, n): net for net, v in NETPINS.items() for r, n in v}


def interior(x1, y1, x2, y2, px, py):
    if (px, py) in ((x1, y1), (x2, y2)):
        return False
    if abs(x1 - x2) < 1e-6:
        return abs(px - x1) < 1e-6 and min(y1, y2) < py < max(y1, y2)
    if abs(y1 - y2) < 1e-6:
        return abs(py - y1) < 1e-6 and min(x1, x2) < px < max(x1, x2)
    return False


shorts = []
for x1, y1, x2, y2, net in wires:
    for (px, py), (r, n) in PIN_AT.items():
        if interior(x1, y1, x2, y2, px, py) and NET_OF.get((r, n)) != net:
            shorts.append(f"{net} runs across {r}.{n} [{NET_OF.get((r, n))}]")
    for a1, b1, a2, b2, other in wires:
        if other != net:
            for ex, ey in ((a1, b1), (a2, b2)):
                if interior(x1, y1, x2, y2, ex, ey):
                    shorts.append(f"{net} is spliced by {other} at ({ex:g},{ey:g})")
assert not shorts, ("SCHEMATIC SHORT:\n  " + "\n  ".join(sorted(set(shorts))[:15]))

# ---------------------------------------------------------------------- emit
SHEET_UUID = uid('sheet')
EFF = '(effects (font (size 1.27 1.27)))'
body = []


def sym_block(ref, lib_id, sx, sy, val, fp, tag=""):
    pins_txt = "\n\t\t".join(f'(pin {q(n)} (uuid {q(uid(tag, ref, "p", n))}))'
                             for n, *_ in ksym.pins(lib_id))
    fp_line = (f'\t\t(property "Footprint" {q(fp)} (at {sx:g} {sy:g} 0) '
               f'(effects (font (size 1.27 1.27)) (hide yes)))\n') if fp else ""
    return f'''\t(symbol
\t\t(lib_id {q(lib_id)})
\t\t(at {sx:g} {sy:g} 0)
\t\t(unit 1)
\t\t(exclude_from_sim no)
\t\t(in_bom {"no" if tag else "yes"})
\t\t(on_board {"no" if tag else "yes"})
\t\t(dnp no)
\t\t(uuid {q(uid(tag, ref))})
\t\t(property "Reference" {q(ref)} (at {sx:g} {sy - 11.43:g} 0) {EFF})
\t\t(property "Value" {q(val)} (at {sx:g} {sy - 8.89:g} 0) {EFF})
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


for ref in PARTS:
    sx, sy = place[ref]
    if ref in FLAGS:
        body.append(sym_block(ref, SYMMAP[ref], sx, sy, "PWR_FLAG", ""))
        continue
    lib, name = D.footprint_of(ref)
    fp = os.path.basename(lib).replace('.pretty', '') + ":" + name
    body.append(sym_block(ref, SYMMAP[ref], sx, sy, D.PARTS[ref][2] or ref, fp))
for i, (lib_id, x, y) in enumerate(powersyms):
    body.append(sym_block(f"#PWR{i:03d}", lib_id, x, y, lib_id.split(':')[1],
                          "", tag="pwr"))
for i, (x1, y1, x2, y2, _n) in enumerate(wires):
    body.append(f'\t(wire (pts (xy {x1:g} {y1:g}) (xy {x2:g} {y2:g})) '
                f'(stroke (width 0) (type default)) (uuid {q(uid("w", i))}))')
for i, (x, y) in enumerate(junctions):
    body.append(f'\t(junction (at {x:g} {y:g}) (diameter 0) (color 0 0 0 0) '
                f'(uuid {q(uid("j", i))}))')
for i, (name, x, y, rot) in enumerate(labels):
    just = "right" if rot == 0 else "left"
    body.append(f'\t(label {q(name)} (at {x:g} {y:g} 0) '
                f'(effects (font (size 1.27 1.27)) (justify {just} bottom)) '
                f'(uuid {q(uid("l", i))}))')
for ref, num in noconn:
    x, y, _ = pin_xy(ref, num)
    body.append(f'\t(no_connect (at {x:g} {y:g}) (uuid {q(uid("nc", ref, num))}))')
for bi, (title, anchor, sats, col, row) in enumerate(BLOCKS):
    ox, oy = origin[bi]
    body.append(f'\t(text {q(title)} (at {ox:g} {oy + bands[bi][0] - 4 * G:g} 0) '
                f'(effects (font (size 2.2 2.2) (bold yes)) (justify left)) '
                f'(uuid {q(uid("t", bi))}))')

libs = "\n".join("\t\t" + dump(ksym.definition(l), 2)
                 for l in sorted(set(SYMMAP.values()) | set(POWER.values())))
open(OUT, 'w', encoding='utf-8', newline='\n').write(f'''(kicad_sch
\t(version 20251024)
\t(generator "mrp")
\t(generator_version "10.0")
\t(uuid {q(uid("root"))})
\t(paper "User" {SHEET_W:g} {SHEET_H:g})
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
n = {k: sum(1 for v in KIND.values() if v == k) for k in ('power', 'local', 'label')}
print(f"wrote {OUT}")
print(f"sheet {SHEET_W:.0f} x {SHEET_H:.0f} mm  (was 1205 x 237)")
print(f"nets: {n['power']} power symbols, {n['local']} wired in-block, "
      f"{n['label']} labelled between blocks")
print(f"{len(wires)} wires, {len(powersyms)} power symbols, {len(noconn)} no-connects")

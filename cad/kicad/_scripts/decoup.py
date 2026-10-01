# -*- coding: utf-8 -*-
"""How far is each bypass cap from the pin it is supposed to bypass?

A decoupling capacitor works by being close. Chip size drives placement, so this
is the number that decides whether growing every chip to 1608 is acceptable -
routability alone does not answer it.

Distance is pad-to-pad on the shared net, taking the closest pair.
"""
import sys, io, os, json, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM
TAG = sys.argv[1] if len(sys.argv) > 1 else 'current'

b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
pads = {}
for fp in b.Footprints():
    for p in fp.Pads():
        c = p.GetPosition()
        pads.setdefault(fp.GetReference(), []).append(
            (p.GetNumber(), p.GetNetname(), TOMM(c.x), TOMM(c.y), fp.IsFlipped()))

# (cap, the part it serves, the net they share, what it is for)
PAIRS = [
    ('C3',  'U2', 'V3V3', 'MCP25625 VIO'),
    ('C4',  'U2', 'V3V3', 'MCP25625 VDD'),
    ('C5',  'U2', 'V5',   'MCP25625 VDDA'),
    ('C6',  'U2', 'V5',   'MCP25625 VDDA bulk'),
    ('C10', 'J2', 'V3V3', 'microSD VDD'),
    ('C11', 'J2', 'V3V3', 'microSD bulk'),
    ('C7',  'Y1', 'XTAL1', 'crystal load'),
    ('C8',  'Y1', 'XTAL2', 'crystal load'),
    ('C12', 'U3', 'IG_DIV', 'ADC filter'),
    ('C2',  'D1', 'V12P', 'V12P bypass'),
]
rows = []
for cap, part, net, what in PAIRS:
    src = [p for p in pads.get(cap, []) if p[1] == net]
    dst = [p for p in pads.get(part, []) if p[1] == net]
    if not src or not dst:
        rows.append((cap, part, what, None, None))
        continue
    best = min(((a[2] - c[2]) ** 2 + (a[3] - c[3]) ** 2) ** 0.5
               for a in src for c in dst)
    # a cap on the far face has to reach through a via as well
    flip = src[0][4] != dst[0][4]
    rows.append((cap, part, what, best, flip))

print(f"=== {TAG} ===")
print(f"{'cap':4s} {'serves':7s} {'purpose':20s} {'distance':>9s}  face")
tot = 0.0
for cap, part, what, d, flip in rows:
    if d is None:
        print(f"{cap:4s} {part:7s} {what:20s} {'n/a':>9s}")
        continue
    tot += d
    print(f"{cap:4s} {part:7s} {what:20s} {d:7.2f}mm  {'opposite' if flip else 'same'}")
print(f"{'':33s} {tot:7.2f}mm total")

trk = sum(TOMM(t.GetLength()) for t in b.GetTracks() if t.GetClass() != 'PCB_VIA')
via = sum(1 for t in b.GetTracks() if t.GetClass() == 'PCB_VIA')
print(f"track {trk:.0f}mm, vias {via}")

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), f"decoup_{TAG}.json")
json.dump({'rows': [(c, p, w, d, f) for c, p, w, d, f in rows],
           'track': trk, 'vias': via}, open(out, 'w'))
sys.stdout.flush()
os._exit(0)

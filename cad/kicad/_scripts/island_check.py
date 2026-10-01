# -*- coding: utf-8 -*-
"""Does every poured region actually touch the ground plane?

A region with no via and no pad in it is floating copper: it looks like a plane
in the plot but connects to nothing. DRC does not flag this because no *pad* is
left unconnected. Check each region for at least one via or GND pad inside it.
"""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM
b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))

vias = [(TOMM(v.GetPosition().x), TOMM(v.GetPosition().y))
        for v in b.GetTracks() if v.GetClass() == 'PCB_VIA' and v.GetNetname() == 'GND']
pads, padshapes = [], []
for fp in b.Footprints():
    for p in fp.Pads():
        if p.GetNetname() == 'GND':
            c = p.GetPosition()
            pads.append((TOMM(c.x), TOMM(c.y)))
            padshapes.append(p)
print(f"{len(vias)} GND vias, {len(pads)} GND pads")
# A region can also hang off a pad it merely TOUCHES (a thermal-spoke fragment
# beside a header barrel, for instance) without holding the pad's centre.
# v4.2 found a 0.3mm2 sliver like that next to U3 pad 3 - connected, not floating.
MM = pcbnew.FromMM

def inside(pt, poly):
    x, y = pt
    n = len(poly)
    c = False
    for i in range(n):
        x0, y0 = poly[i]; x1, y1 = poly[(i + 1) % n]
        if (y0 > y) != (y1 > y) and x < (x1 - x0) * (y - y0) / (y1 - y0) + x0:
            c = not c
    return c

bad = 0
for z in b.Zones():
    if z.GetIsRuleArea() or z.GetNetname() != 'GND':
        continue
    L = b.GetLayerName(z.GetLayer())
    poly = z.GetFilledPolysList(z.GetLayer())
    for oi in range(poly.OutlineCount()):
        pts = [(TOMM(poly.CVertex(vi, oi, -1).x), TOMM(poly.CVertex(vi, oi, -1).y))
               for vi in range(poly.VertexCount(oi, -1))]
        xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
        a = abs(sum(pts[i][0] * pts[(i + 1) % len(pts)][1]
                    - pts[(i + 1) % len(pts)][0] * pts[i][1]
                    for i in range(len(pts)))) / 2
        nv = sum(1 for v in vias if inside(v, pts))
        np_ = sum(1 for p in pads if inside(p, pts))
        touch = 0
        if not nv + np_:
            chain = pcbnew.SHAPE_LINE_CHAIN(poly.Outline(oi))
            touch = sum(1 for p in padshapes
                        if p.IsOnLayer(z.GetLayer())
                        and p.GetEffectiveShape(z.GetLayer()).Collide(chain, MM(0.01)))
        tag = "OK" if nv + np_ else ("OK (touches %d pad)" % touch if touch else "FLOATING")
        if not (nv + np_ + touch):
            bad += 1
        print(f"  {L:8s} region {oi}: {a:7.1f}mm2  x{min(xs):5.1f}..{max(xs):5.1f} "
              f"y{min(ys):5.1f}..{max(ys):5.1f}  vias {nv:3d} pads {np_:2d}  {tag}")
print(f"floating regions: {bad}")
sys.stdout.flush()
os._exit(0)

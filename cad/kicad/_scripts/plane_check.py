# -*- coding: utf-8 -*-
"""How intact is each poured plane?

Freerouting was handed In1/In2 as planes, but nothing stops it routing on them.
This counts the filled regions per zone and their areas, so a plane that has been
sliced into pieces by signal tracks shows up as several regions instead of one.
"""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM
b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))

for z in b.Zones():
    if z.GetIsRuleArea():
        continue
    L = b.GetLayerName(z.GetLayer())
    poly = z.GetFilledPolysList(z.GetLayer())
    areas = []
    for oi in range(poly.OutlineCount()):
        pts = [(TOMM(poly.CVertex(vi, oi, -1).x), TOMM(poly.CVertex(vi, oi, -1).y))
               for vi in range(poly.VertexCount(oi, -1))]
        a = 0.0
        for i in range(len(pts)):
            x0, y0 = pts[i]; x1, y1 = pts[(i + 1) % len(pts)]
            a += x0 * y1 - x1 * y0
        areas.append(abs(a) / 2)
    areas.sort(reverse=True)
    tot = sum(areas)
    head = ', '.join(f"{a:.1f}" for a in areas[:6])
    print(f"{L:8s} {z.GetNetname():5s}: {len(areas)} region(s), "
          f"{tot:6.1f}mm2 total, largest {areas[0]:.1f} ({areas[0]/tot*100:.0f}%)  [{head}]")

# tracks per layer, to see which layers the router actually used
per = {}
for t in b.GetTracks():
    if t.GetClass() == 'PCB_VIA':
        continue
    per.setdefault(t.GetLayerName(), []).append(t.GetNetname())
for L in sorted(per):
    nets = sorted(set(per[L]))
    print(f"{L:8s}: {len(per[L])} tracks, nets: {', '.join(nets)}")
sys.stdout.flush()
os._exit(0)

# -*- coding: utf-8 -*-
"""Measure the finished board for the design document."""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM
b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
print("copper layers:", b.GetCopperLayerCount())

trk = [t for t in b.GetTracks() if t.GetClass() != 'PCB_VIA']
via = [t for t in b.GetTracks() if t.GetClass() == 'PCB_VIA']
stitch = [v for v in via if v.GetNetname() == 'GND']
print(f"tracks {len(trk)}, vias {len(via)} (GND {len(stitch)})")
per = {}
for t in trk:
    per[t.GetLayerName()] = per.get(t.GetLayerName(), 0) + TOMM(t.GetLength())
for k, v in sorted(per.items()):
    print(f"  {k}: {v:.1f}mm")

def pad(ref, num):
    for fp in b.Footprints():
        if fp.GetReference() == ref:
            for p in fp.Pads():
                if p.GetNumber() == num:
                    c = p.GetPosition()
                    return TOMM(c.x), TOMM(c.y)
    raise KeyError(ref + '.' + num)

CHAIN = [('V12_RAW', 'J1', '8', 'F1', '1'),
         ('V12_FUSED', 'F1', '2', 'D1', '2'),
         ('V12P', 'D1', '1', 'Q2', '4'),
         ('V12_SW', 'Q2', '1', 'U4', '3'),
         ('SW_5V', 'U4', '5', 'L1', '1')]
tot = 0.0
for name, ra, pa, rb, pb in CHAIN:
    ax, ay = pad(ra, pa); bx, by = pad(rb, pb)
    d = ((ax - bx) ** 2 + (ay - by) ** 2) ** 0.5
    tot += d
    print(f"{name:10s} {ra}.{pa} ({ax:5.2f},{ay:5.2f}) -> {rb}.{pb} ({bx:5.2f},{by:5.2f})  {d:5.2f}mm")
print(f"{'total':10s} {tot:.2f}mm")

for ref in ('U1', 'U2', 'J1'):
    for fp in b.Footprints():
        if fp.GetReference() == ref:
            c = fp.GetPosition()
            print(f"{ref}: ({TOMM(c.x):.2f},{TOMM(c.y):.2f}) rot {fp.GetOrientationDegrees():.0f} "
                  f"{'B.Cu' if fp.IsFlipped() else 'F.Cu'}")
sys.stdout.flush()
os._exit(0)

# -*- coding: utf-8 -*-
"""Compare footprint placement between two board files."""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
TOMM = pcbnew.ToMM

def snap(path):
    b = pcbnew.LoadBoard(path)
    out = {}
    for fp in b.Footprints():
        p = fp.GetPosition()
        out[fp.GetReference()] = (round(TOMM(p.x), 3), round(TOMM(p.y), 3),
                                  round(fp.GetOrientationDegrees()), fp.IsFlipped(),
                                  fp.GetFPID().GetLibItemName().wx_str())
    return out

a = snap(sys.argv[1])
b = snap(sys.argv[2])
print(f"A = {sys.argv[1]}\nB = {sys.argv[2]}\n")
same = 0
for ref in sorted(set(a) | set(b)):
    x, y = a.get(ref), b.get(ref)
    if x == y:
        same += 1
        continue
    print(f"{ref:5s} A={x}\n      B={y}")
print(f"\n{same} of {len(set(a) | set(b))} footprints identical")
sys.stdout.flush()
os._exit(0)

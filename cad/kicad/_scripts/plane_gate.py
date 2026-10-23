# -*- coding: utf-8 -*-
"""Seed gate: the In1 GND plane must stay essentially whole.

Freerouting 2.5 fans out every SMD pad through a via, which pokes far more
holes in the inner planes than 2.2.4 did. Small slivers (< 10mm2) next to a
via field are cosmetic - island_check proves they still tie to GND - but a
larger severed pocket means return currents detour, so those seeds go back.
Exit 1 = reject.
"""
import sys, io, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
b = pcbnew.LoadBoard(r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.kicad_pcb")
LIMIT = 10.0                                  # mm2 for any non-largest region

areas = []
for z in b.Zones():
    if z.GetIsRuleArea() or z.GetNetname() != 'GND':
        continue
    for l in z.GetLayerSet().Seq():
        if b.GetLayerName(l) != 'In1.Cu':
            continue
        poly = z.GetFilledPolysList(l)
        for i in range(poly.OutlineCount()):
            areas.append(poly.Outline(i).Area() / 1e12)
areas.sort(reverse=True)
frags = [a for a in areas[1:] if a >= LIMIT]
print(f"In1 GND: {len(areas)} region(s) " +
      ", ".join(f"{a:.1f}mm2" for a in areas[:5]) +
      (f" -> REJECT ({len(frags)} fragment(s) >= {LIMIT}mm2)" if frags else " -> ok"))
sys.exit(1 if frags else 0)

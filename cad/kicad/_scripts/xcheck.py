# -*- coding: utf-8 -*-
"""BOM and CPL must name exactly the same designators, or JLC rejects the pair."""
import csv, sys, io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad\fab"

def refs(cell):
    out = []
    for p in cell.split(','):
        p = p.strip()
        if '-' in p:
            a, b = p.split('-', 1)
            pre = ''.join(x for x in a if x.isalpha())
            out += [pre + str(i) for i in range(int(a[len(pre):]), int(b[len(pre):]) + 1)]
        elif p:
            out.append(p)
    return out

with open(D + r"\MotoRecoPico-bom.csv", encoding='utf-8') as f:
    bom = set()
    for r in list(csv.reader(f))[1:]:
        if r:
            bom |= set(refs(r[0]))
with open(D + r"\MotoRecoPico-cpl.csv", encoding='utf-8') as f:
    rows = [r for r in list(csv.reader(f))[1:] if r]
cpl = {r[0] for r in rows}
print(f"BOM {len(bom)} designators, CPL {len(cpl)}")
print("in BOM but not CPL:", sorted(bom - cpl) or "none")
print("in CPL but not BOM:", sorted(cpl - bom) or "none")

bad = [r for r in rows if not (r[1].endswith('mm') and r[2].endswith('mm'))
       or r[3] not in ('Top', 'Bottom')]
print("rows with a bad unit or layer:", bad or "none")

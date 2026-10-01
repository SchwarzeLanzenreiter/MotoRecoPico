# -*- coding: utf-8 -*-
"""Split the BOM the same way the CPL is split.

JLCPCB cross-checks designators between the two files, so a BOM line for a part
that is not in the CPL is an error on upload. The through-hole parts (J1, U1, U3,
Y1) are hand-fitted, so they come out of the SMT BOM and go into a separate list.
"""
import sys, io, os, csv, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"

b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
tht = {fp.GetReference() for fp in b.Footprints()
       if any(p.GetAttribute() == pcbnew.PAD_ATTRIB_PTH for p in fp.Pads())}
print("through-hole:", ", ".join(sorted(tht)))

src = os.path.join(D, "fab", "MotoRecoPico-bom.csv")
with open(src, newline='', encoding='utf-8') as f:
    rows = list(csv.reader(f))
head, body = rows[0], [r for r in rows[1:] if r]

def refs(cell):
    """Expand "R5-R7,R10-R13" back into individual designators."""
    out = []
    for part in cell.split(','):
        part = part.strip()
        if '-' in part:
            a, c = part.split('-', 1)
            pre = ''.join(ch for ch in a if ch.isalpha())
            out += [f"{pre}{i}" for i in range(int(a[len(pre):]), int(c[len(pre):]) + 1)]
        elif part:
            out.append(part)
    return out

def natural(ref):
    """C10 sorts after C9, not after C1."""
    pre = ''.join(c for c in ref if c.isalpha())
    num = ''.join(c for c in ref[len(pre):] if c.isdigit())
    return (pre, int(num) if num else 0)

smt, hand = [], []
for r in body:
    rr = refs(r[0])
    (hand if set(rr) & tht else smt).append(r)
    if set(rr) & tht and not set(rr) <= tht:
        print(f"  WARNING: row {r[0]!r} mixes through-hole and SMD - split it by hand")
# BOM and CPL in the same order, so the two can be read side by side
smt.sort(key=lambda r: natural(refs(r[0])[0]))
hand.sort(key=lambda r: natural(refs(r[0])[0]))

for name, data in (("MotoRecoPico-bom.csv", smt),
                   ("MotoRecoPico-bom-tht.csv", hand)):
    p = os.path.join(D, "fab", name)
    with open(p, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f, quoting=csv.QUOTE_ALL)
        w.writerow(head); w.writerows(data)
    n = sum(len(refs(r[0])) for r in data)
    print(f"{name}: {len(data)} lines, {n} parts")
sys.stdout.flush()
os._exit(0)

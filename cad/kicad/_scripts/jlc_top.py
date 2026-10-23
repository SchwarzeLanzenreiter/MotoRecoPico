# -*- coding: utf-8 -*-
"""BOM + CPL for having JLCPCB assemble the TOP face only.

Since v4.2 the top face carries every 1608 chip plus U2 and J2 - all the
fine-pitch work - while the bottom face holds the Pico socket and the larger
SOT/SOD/SMA/1206 parts that are comfortable to hand-solder. This writes the
upload pair for that split:

    fab/MotoRecoPico-bom-top.csv   rows of fab/MotoRecoPico-bom.csv on the top
    fab/MotoRecoPico-cpl-top.csv   JLC-format placements, Top rows only

and fab/MotoRecoPico-bom-bottom.csv as the hand-soldering shopping list for
the SMD parts JLC will NOT fit. Run fab_all.ps1 (or at least the kicad-cli
BOM export + jlc_bom.py) first so MotoRecoPico-bom.csv is current.
"""
import sys, io, os, csv, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM


def natural(ref):
    pre = ''.join(c for c in ref if c.isalpha())
    num = ''.join(c for c in ref[len(pre):] if c.isdigit())
    return (pre, int(num) if num else 0)


b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
top_smt, bot_smt, tht = set(), set(), set()
rows = []
for fp in sorted(b.Footprints(), key=lambda f: natural(f.GetReference())):
    ref = fp.GetReference()
    if fp.IsExcludedFromPosFiles():          # copper-only feature (J4 pads)
        continue
    if any(p.GetAttribute() == pcbnew.PAD_ATTRIB_PTH for p in fp.Pads()):
        tht.add(ref)
        continue
    if fp.IsFlipped():
        bot_smt.add(ref)
        continue
    top_smt.add(ref)
    pos = fp.GetPosition()
    rows.append([ref, f"{TOMM(pos.x):.4f}mm", f"{-TOMM(pos.y):.4f}mm",
                 "Top", f"{fp.GetOrientationDegrees() % 360:g}"])

cpl = os.path.join(D, "fab", "MotoRecoPico-cpl-top.csv")
with open(cpl, 'w', newline='', encoding='utf-8') as f:
    w = csv.writer(f)
    w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
    w.writerows(rows)
print(f"wrote {cpl}: {len(rows)} parts (all Top)")
print(f"left to hand-solder: SMD bottom {sorted(bot_smt)}")
print(f"                     through-hole {sorted(tht)}")

# split the full SMT BOM by the face each designator sits on
src = os.path.join(D, "fab", "MotoRecoPico-bom.csv")
with open(src, newline='', encoding='utf-8') as f:
    body = list(csv.reader(f))
head, body = body[0], [r for r in body[1:] if r and r[0].strip()]
unknown = [r[0] for r in body if r[0] not in top_smt | bot_smt | tht]
assert not unknown, f"BOM rows that are not on the board: {unknown} - rerun fab_all.ps1"
for name, keep in (("MotoRecoPico-bom-top.csv", top_smt),
                   ("MotoRecoPico-bom-bottom.csv", bot_smt)):
    data = [r for r in body if r[0] in keep]
    p = os.path.join(D, "fab", name)
    with open(p, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f, quoting=csv.QUOTE_ALL)
        w.writerow(head)
        w.writerows(data)
    print(f"{name}: {len(data)} parts")

# the pair must agree or the JLC upload rejects it
bom_refs = {r[0] for r in body if r[0] in top_smt}
cpl_refs = {r[0] for r in rows}
assert bom_refs == cpl_refs, \
    f"top BOM/CPL disagree: {bom_refs ^ cpl_refs}"
print("cross-check: top BOM and CPL carry the same designators")
sys.stdout.flush()
os._exit(0)

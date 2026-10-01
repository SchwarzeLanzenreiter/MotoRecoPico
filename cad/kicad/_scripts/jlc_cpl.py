# -*- coding: utf-8 -*-
"""Write the CPL in JLCPCB's own format.

kicad-cli's `pcb export pos` writes its own column names (Ref/PosX/PosY/Rot/Side).
JLCSMT_Sample_CPL1.xlsx shows what the uploader actually wants:

    Designator | Mid X | Mid Y | Layer | Rotation
    C1         | 95.0518mm | 22.6822mm | Top | 270

Differences that break the upload: the header names, lower-case top/bottom, and
the missing mm unit. Coordinates keep the same frame as the gerbers and drill
(X 0..21, Y -61..0), which is what makes the three files line up.

"Mid" means the centre of the part. KiCad exports the footprint ANCHOR, which is
the body centre for the standard libraries but sits on pin 1 for some through-hole
footprints - this reports any part where the two disagree by more than 0.1mm.
"""
import sys, io, os, csv, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
TOMM = pcbnew.ToMM

# JLCPCB SMT assembles surface-mount parts. Through-hole parts are a separate
# service and an unassemblable line in the CPL is itself an upload error.
SMT_ONLY = (len(sys.argv) > 1 and sys.argv[1] == 'smt')

def natural(ref):
    """C10 sorts after C9, not after C1 - keeps this file in step with the BOM."""
    pre = ''.join(c for c in ref if c.isalpha())
    num = ''.join(c for c in ref[len(pre):] if c.isdigit())
    return (pre, int(num) if num else 0)

b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
rows, notes, skipped = [], [], []
for fp in sorted(b.Footprints(), key=lambda f: natural(f.GetReference())):
    ref = fp.GetReference()
    pos = fp.GetPosition()
    ax, ay = TOMM(pos.x), TOMM(pos.y)

    pads = list(fp.Pads())
    tht = any(p.GetAttribute() == pcbnew.PAD_ATTRIB_PTH for p in pads)
    if pads:
        xs = [TOMM(p.GetPosition().x) for p in pads]
        ys = [TOMM(p.GetPosition().y) for p in pads]
        cx, cy = (min(xs) + max(xs)) / 2, (min(ys) + max(ys)) / 2
        off = ((ax - cx) ** 2 + (ay - cy) ** 2) ** 0.5
        if off > 0.1:
            notes.append(f"{ref:4s} anchor ({ax:.3f},{ay:.3f}) is {off:.2f}mm from the "
                         f"pad centre ({cx:.3f},{cy:.3f})")
    if SMT_ONLY and tht:
        skipped.append(f"{ref} ({fp.GetValue()})")
        continue

    rot = fp.GetOrientationDegrees() % 360
    rows.append([ref, f"{ax:.4f}mm", f"{-ay:.4f}mm",
                 "Bottom" if fp.IsFlipped() else "Top",
                 f"{rot:g}"])

out = os.path.join(D, "fab",
                   "MotoRecoPico-cpl.csv" if SMT_ONLY else "MotoRecoPico-cpl-all.csv")
with open(out, 'w', newline='', encoding='utf-8') as f:
    w = csv.writer(f)
    w.writerow(["Designator", "Mid X", "Mid Y", "Layer", "Rotation"])
    w.writerows(rows)
print(f"wrote {out}: {len(rows)} parts "
      f"(Top {sum(1 for r in rows if r[3]=='Top')}, "
      f"Bottom {sum(1 for r in rows if r[3]=='Bottom')})")
if skipped:
    print("through-hole, left out of the SMT CPL: " + ", ".join(skipped))
for n in notes:
    print("  ANCHOR: " + n)
sys.stdout.flush()
os._exit(0)

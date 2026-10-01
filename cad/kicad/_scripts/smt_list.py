# -*- coding: utf-8 -*-
"""List what is left in the SMT set, and which of it JLC is unlikely to stock.

A footprint from the project's own library means the part is not a catalogue
part, so JLC will not have a reel for it even though it is surface mount.
"""
import sys, io, os, csv, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))

with open(os.path.join(D, "fab", "MotoRecoPico-cpl.csv"), encoding='utf-8') as f:
    cpl = {r[0] for r in list(csv.reader(f))[1:] if r}

custom = []
for fp in sorted(b.Footprints(), key=lambda f: f.GetReference()):
    ref = fp.GetReference()
    if ref not in cpl:
        continue
    # GetFPIDAsString() drops the library nickname in KiCad 10; GetLibNickname()
    # off the LIB_ID is the one that still carries it.
    fid = fp.GetFPID()
    lib = fid.GetLibNickname().wx_str()
    name = fid.GetLibItemName().wx_str()
    print(f"  {ref:4s} {fp.GetValue():22s} {lib}:{name}")
    if lib == 'MotoRecoPico':
        custom.append(f"{ref} ({fp.GetValue()}) - {lib}:{name}")
print(f"CPL holds {len(cpl)} surface-mount parts")
print("of which on a project-specific footprint (no JLC catalogue part):")
for c in custom:
    print("  " + c)
sys.stdout.flush()
os._exit(0)

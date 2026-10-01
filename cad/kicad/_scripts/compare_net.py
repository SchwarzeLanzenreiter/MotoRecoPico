# -*- coding: utf-8 -*-
"""Does KiCad read the same circuit out of the schematic that we put in?

Compares the netlist exported from MotoRecoPico.kicad_sch against the intended
netlist as (part, pin) sets. This is what proves the drawn wires really connect
what they are meant to - and, just as important, that no wire connects anything
it should not.
"""
import sys, io, os
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from sexp import parse, find, first, val
import mrp_design as D
import ksym_map as M

NET = r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.net"
root = parse(open(NET, encoding='utf-8', errors='replace').read())[0]

got = {}
for n in find(first(root, 'nets'), 'net'):
    name = val(first(n, 'name')[1])
    got[name] = {(val(first(nd, 'ref')[1]), val(first(nd, 'pin')[1]))
                 for nd in find(n, 'node')}
want = {net: M.netpins(net) for net in D.NETS}

# a KiCad net name may be decorated; match on membership, not on the name
unmatched_got = dict(got)
bad = []
for net, w in sorted(want.items()):
    hit = [k for k, v in unmatched_got.items() if v == w]
    if hit:
        unmatched_got.pop(hit[0])
        continue
    near = max(unmatched_got.items(), key=lambda kv: len(kv[1] & w), default=(None, set()))
    bad.append((net, w, near))

print(f"intended nets: {len(want)}   nets found in the schematic: {len(got)}")
print(f"exact matches: {len(want) - len(bad)}")
if bad:
    print("\nMISMATCHED:")
    for net, w, (kn, kv) in bad:
        print(f"  {net}: intended {sorted(w)}")
        print(f"      closest in schematic '{kn}': {sorted(kv)}")
        print(f"      missing {sorted(w - kv)}   extra {sorted(kv - w)}")
leftover = {k: v for k, v in unmatched_got.items() if len(v) > 1}
if leftover:
    print(f"\nEXTRA nets in the schematic ({len(leftover)}):")
    for k, v in sorted(leftover.items()):
        print(f"  {k}: {sorted(v)}")
print("\nRESULT:", "netlists agree" if not bad and not leftover else "MISMATCH")

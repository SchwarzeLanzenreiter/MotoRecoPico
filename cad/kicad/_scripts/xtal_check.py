# -*- coding: utf-8 -*-
"""Guard the v3 oscillator win: XTAL nets stay OFF the inner layers.

XTAL1/XTAL2 carry the board's one permanent 16MHz aggressor. v3 moved C7/C8 to
the crystal and got both nets onto short outer-layer runs; Freerouting does not
know that matters and some seeds shove one of them through the In1 GND plane
again. Exit 1 makes the seed loop reject those.
"""
import sys, io, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
b = pcbnew.LoadBoard(r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.kicad_pcb")
TOMM = pcbnew.ToMM
bad = False
for net in ('XTAL1', 'XTAL2'):
    inner, vias, tot = 0.0, 0, 0.0
    for t in b.GetTracks():
        if t.GetNetname() != net:
            continue
        if t.GetClass() == 'PCB_VIA':
            vias += 1
        else:
            L = b.GetLayerName(t.GetLayer())
            tot += TOMM(t.GetLength())
            if L.startswith('In'):
                inner += TOMM(t.GetLength())
    ok = inner == 0 and vias <= 1
    bad |= not ok
    print(f"{net}: {tot:.1f}mm, vias {vias}, inner {inner:.1f}mm -> {'ok' if ok else 'REJECT'}")
sys.exit(1 if bad else 0)

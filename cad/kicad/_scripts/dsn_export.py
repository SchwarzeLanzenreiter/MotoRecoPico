# -*- coding: utf-8 -*-
"""Write the Specctra DSN handed to the external autorouter.

Two things the router cannot work out for itself:
  * GND is poured, not routed. Exporting the filled zones makes KiCad emit them
    as planes, so Freerouting leaves GND alone instead of spending the free lanes
    on 35 pads' worth of ground tracks.
  * Freerouting routes right up to the board outline. Handing it an outline inset
    by the edge-clearance rule keeps the real edge clear.

The board file itself is left untouched - this only writes the .dsn.
"""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
MM = pcbnew.FromMM
W, H, EDGE, CLR = 21.0, 61.0, 0.4, 0.15

PLANES = (len(sys.argv) < 2) or sys.argv[1] != 'noplanes'
INSET = float(sys.argv[2]) if len(sys.argv) > 2 else 0.3

b = pcbnew.LoadBoard(os.path.join(D, "MotoRecoPico.kicad_pcb"))
for t in list(b.GetTracks()):
    b.Remove(t)
    try:
        t.thisown = 0
    except AttributeError:
        pass

gnd = b.FindNet('GND')
# On four layers only the inner layers are handed over as planes - pouring GND on
# F and B too was tried and left the router less room on the two signal layers.
# On two layers there ARE no inner layers: the outer GND pours are the only plane
# there is, so the router gets those and has to route V3V3 as an ordinary net.
if b.GetCopperLayerCount() == 4:
    LAYERS = [(pcbnew.In1_Cu, 'GND'), (pcbnew.In2_Cu, 'V3V3')]
else:
    LAYERS = [(pcbnew.F_Cu, 'GND'), (pcbnew.B_Cu, 'GND')]
for L, netname in (LAYERS if PLANES else ()):
    z = pcbnew.ZONE(b)
    z.SetLayer(L); z.SetNet(b.FindNet(netname))
    z.SetLocalClearance(MM(CLR)); z.SetMinThickness(MM(0.2))
    o = z.Outline(); o.NewOutline()
    for x, y in ((EDGE, EDGE), (W - EDGE, EDGE), (W - EDGE, H - EDGE), (EDGE, H - EDGE)):
        o.Append(MM(x), MM(y))
    b.Add(z); z.thisown = 0
pcbnew.ZONE_FILLER(b).Fill(b.Zones())

# shrink the outline by the edge-clearance rule, for the router's benefit only
for s in b.GetDrawings():
    if s.GetLayer() != pcbnew.Edge_Cuts:
        continue
    for get, set_ in ((s.GetStart, s.SetStart), (s.GetEnd, s.SetEnd)):
        p = get()
        x, y = pcbnew.ToMM(p.x), pcbnew.ToMM(p.y)
        x = INSET if x < W / 2 else W - INSET
        y = INSET if y < H / 2 else H - INSET
        set_(pcbnew.VECTOR2I(MM(x), MM(y)))

out = os.path.join(D, "MotoRecoPico.dsn")
ok = pcbnew.ExportSpecctraDSN(b, out)
print(f"ExportSpecctraDSN -> {ok}, {os.path.getsize(out)} bytes "
      f"(GND {'planes' if PLANES else 'routed'}, outline inset {INSET}mm)")
sys.stdout.flush()
os._exit(0)

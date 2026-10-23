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

# J4's five direct-wire pads are scripted escapes (ses_import draws them
# straight from the J1 barrels, no vias), so the router must neither route
# them nor build anything through their strip. Hide the footprint and fence
# off the strip below J1's holes on B.Cu.
if any(fp.GetReference() == 'J4' for fp in b.Footprints()):
    for fp in list(b.Footprints()):
        if fp.GetReference() == 'J4':
            b.Remove(fp)
            fp.thisown = 0
    # the pad strip itself, plus the three lanes the back-row escapes
    # (CANL x15.5, CANH x11.5, +12V x3.5) drop through between the front-row
    # barrels - the front-row escapes (IGN, GND) hug their own barrels and
    # need no lane of their own
    rects = [(0, 0, W, 2.8)]
    rects += [(x - 0.45, 2.8, x + 0.45, 6.3) for x in (15.5, 11.5, 3.5)]
    for x1, y1, x2, y2 in rects:
        ka = pcbnew.ZONE(b)
        ka.SetIsRuleArea(True)
        ka.SetDoNotAllowTracks(True); ka.SetDoNotAllowVias(True)
        ka.SetLayer(pcbnew.B_Cu)
        o = ka.Outline(); o.NewOutline()
        for x, y in ((x1, y1), (x2, y1), (x2, y2), (x1, y2)):
            o.Append(MM(x), MM(y))
        b.Add(ka); ka.thisown = 0

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

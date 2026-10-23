# -*- coding: utf-8 -*-
"""Pull the autorouter's session back in, then pour the planes."""
import sys, io, os, pcbnew
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
D = r"E:\Claude\MotoRecoPico\cad\kicad"
BRD = os.path.join(D, "MotoRecoPico.kicad_pcb")
MM = pcbnew.FromMM
W, H, EDGE, CLR = 21.0, 61.0, 0.4, 0.15

b = pcbnew.LoadBoard(BRD)
ok = pcbnew.ImportSpecctraSES(b, os.path.join(D, "MotoRecoPico.ses"))
tracks = [t for t in b.GetTracks()]
vias = [t for t in tracks if t.GetClass() == 'PCB_VIA']
print(f"ImportSpecctraSES -> {ok}: {len(tracks)-len(vias)} tracks, {len(vias)} vias")

# J4 (direct-wire pads) is hidden from the autorouter by dsn_export: each pad
# sits in line of sight of the J1 barrel that feeds it (same left-to-right
# order), so the five escapes are drawn here on B.Cu as a straight drop plus
# one slanted entry - no vias. The session import above wipes all tracks,
# which is why this cannot live in mrp_brd.
j4 = next((fp for fp in b.Footprints() if fp.GetReference() == 'J4'), None)
if j4:
    j1 = next(fp for fp in b.Footprints() if fp.GetReference() == 'J1')
    src = {p.GetNetname(): p.GetPosition() for p in j1.Pads() if p.GetNetname()}

    def seg(x1, y1, x2, y2, net):
        t = pcbnew.PCB_TRACK(b)
        t.SetStart(pcbnew.VECTOR2I(MM(x1), MM(y1)))
        t.SetEnd(pcbnew.VECTOR2I(MM(x2), MM(y2)))
        t.SetWidth(MM(0.4)); t.SetLayer(pcbnew.B_Cu); t.SetNet(net)
        b.Add(t); t.thisown = 0

    for p in j4.Pads():
        s, e = src[p.GetNetname()], p.GetPosition()
        sx, sy = pcbnew.ToMM(s.x), pcbnew.ToMM(s.y)
        ex, ey = pcbnew.ToMM(e.x), pcbnew.ToMM(e.y)
        if sy > 5.0:                    # J1 back row: drop between the front-
            seg(sx, sy, sx, 2.8, p.GetNet())      # row barrels, slant into the pad
            seg(sx, 2.8, ex, ey, p.GetNet())
        else:                           # J1 front row: straight slant
            seg(sx, sy, ex, ey, p.GetNet())
    print("J4 escapes drawn: 5 pads wired from J1 on B.Cu, 0 vias")

# v2: every pad joins the pour through THERMAL SPOKES. v1 used solid joints and
# hand-soldering the boards proved them miserable - every grounded pin sinks
# straight into the plane. The v1 worry (spokes not reaching in the 15mm strip
# and stranding Pico ground pins) is checked, not assumed: the DRC unconnected
# count and island_check must both come back zero, else the build fails.
# m_MinResolvedSpokes is 1 in mrp_brd, so one landed spoke satisfies DRC.
for fp in b.Footprints():
    for p in fp.Pads():
        p.SetLocalZoneConnection(pcbnew.ZONE_CONNECTION_INHERITED)

# Stack depends on the layer count mrp_brd chose:
#   4: F.Cu signal / In1.Cu GND plane / In2.Cu 3V3 plane / B.Cu signal
#   2: GND poured on both faces, V3V3 routed - the v1 2-layer arrangement.
if b.GetCopperLayerCount() == 4:
    PLANES = [(pcbnew.F_Cu, 'GND'), (pcbnew.In1_Cu, 'GND'),
              (pcbnew.In2_Cu, 'V3V3'), (pcbnew.B_Cu, 'GND')]
else:
    PLANES = [(pcbnew.F_Cu, 'GND'), (pcbnew.B_Cu, 'GND')]

gnd = b.FindNet('GND')
for L, netname in PLANES:
    z = pcbnew.ZONE(b)
    z.SetLayer(L); z.SetNet(b.FindNet(netname))
    # 0.13mm is JLCPCB's floor. A thicker minimum makes the filler abandon narrow
    # channels between tracks, which is what split the pour into islands.
    z.SetLocalClearance(MM(CLR)); z.SetMinThickness(MM(0.13))
    z.SetPadConnection(pcbnew.ZONE_CONNECTION_THERMAL)
    # Spoke 0.4mm carries this board's worst pin current (~200mA) with ease;
    # the gap is what makes the joint hand-solderable. 0.3mm preferred; a pad
    # whose surroundings are too tight for the spoke to bridge 0.3 sometimes
    # takes a shorter one, so the gap is adjustable per run (MRP_THERMAL_GAP).
    z.SetThermalReliefSpokeWidth(MM(0.4))
    z.SetThermalReliefGap(MM(float(os.environ.get('MRP_THERMAL_GAP', '0.3'))))
    z.SetIslandRemovalMode(pcbnew.ISLAND_REMOVAL_MODE_ALWAYS)
    o = z.Outline(); o.NewOutline()
    for x, y in ((EDGE, EDGE), (W - EDGE, EDGE), (W - EDGE, H - EDGE), (EDGE, H - EDGE)):
        o.Append(MM(x), MM(y))
    b.Add(z); z.thisown = 0
pcbnew.ZONE_FILLER(b).Fill(b.Zones())
pcbnew.SaveBoard(BRD, b)
print(f"poured {len(PLANES)} plane(s) with thermal-relief pads, saved {BRD}")
sys.stdout.flush()
os._exit(0)

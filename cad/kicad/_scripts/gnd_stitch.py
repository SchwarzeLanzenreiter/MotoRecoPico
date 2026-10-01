# -*- coding: utf-8 -*-
"""Stitch the two GND pours together.

Routing fences the pour into islands, and an island with no via is one the
ground pads on it cannot use. This drops GND vias wherever one physically fits,
welding the F.Cu and B.Cu pours into a single plane.

Candidate testing is vectorised - the earlier per-candidate Python loop over
every track, pad and via never finished.
"""
import sys, io, os, time
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace',
                              line_buffering=True)
import numpy as np
import pcbnew

D = r"E:\Claude\MotoRecoPico\cad\kicad"
BRD = os.path.join(D, "MotoRecoPico.kicad_pcb")
MM, TOMM = pcbnew.FromMM, pcbnew.ToMM
W, H = 21.0, 61.0
VIA_D, VIA_DRILL, CLR = 0.6, 0.3, 0.15
KEEP = VIA_D / 2 + CLR          # via copper edge to foreign copper
EDGE = 0.3 + VIA_D / 2
PITCH = 1.0

t0 = time.time()
b = pcbnew.LoadBoard(BRD)
gnd = b.FindNet('GND')
gcode = gnd.GetNetCode()
print(f"loaded ({time.time()-t0:.1f}s)")

# Run this on a freshly imported board (mrp_brd -> ses_import -> here); it does
# not try to remove stitching from a previous pass.
#
# PCB_VIA.GetWidth() with no layer argument trips a wxWidgets assertion in
# KiCad 10 that blocks the process on a dialog - that is what hung two earlier
# attempts. GetFrontWidth() is the layer-aware accessor.
HOLE2HOLE = 0.25
NEAR_PAD = 4.0        # only stitch where the pour is certainly the main body
# A stitching via punches a clearance void through BOTH inner planes. A lattice
# of them around another net's PTH pad can strangle that pad's only thermal
# connection: after the v4.1 Pico rotation, U3's 3V3 pin (pad 36, fed by the
# In2 plane) landed inside J2's NEAR_PAD circle and the via cage killed every
# spoke - 1 unconnected on all 8 seeds. Netted non-GND PTH pads therefore keep
# a moat: 2.2mm leaves a >=0.5mm fill ring past the pad's thermal void, enough
# for a spoke from any direction. Unnetted barrels need nothing, and GND
# barrels weld the pours by themselves.
MOAT = 2.2
segs, discs, holes, gnd_pads, moat_pads = [], [], [], [], []
for t in b.GetTracks():
    if t.GetClass() == 'PCB_VIA':
        p = t.GetPosition()
        # A GND via needs no clearance from us but its DRILL still does - leaving
        # same-net vias out of the obstacle list dropped stitching straight on top
        # of routing vias.
        holes.append((TOMM(p.x), TOMM(p.y),
                      TOMM(t.GetDrillValue()) / 2 + VIA_DRILL / 2 + HOLE2HOLE))
        if t.GetNetCode() != gcode:
            discs.append((TOMM(p.x), TOMM(p.y), TOMM(t.GetFrontWidth()) / 2))
    elif t.GetNetCode() != gcode:
        s, e = t.GetStart(), t.GetEnd()
        segs.append((TOMM(s.x), TOMM(s.y), TOMM(e.x), TOMM(e.y),
                     TOMM(t.GetWidth()) / 2))
for fp in b.Footprints():
    for p in fp.Pads():
        c = p.GetPosition()
        bb = p.GetBoundingBox()
        w = TOMM(bb.GetRight()) - TOMM(bb.GetLeft())
        h = TOMM(bb.GetBottom()) - TOMM(bb.GetTop())
        # A circle is its bbox's inscribed circle; anything else has corners, so
        # take the CIRCUMscribed radius. Using max/2 for a square pad understated
        # U1's pad by 0.37mm and let vias sit 0.06mm from it.
        r = w / 2 if p.GetShape() == pcbnew.PAD_SHAPE_CIRCLE else (w * w + h * h) ** 0.5 / 2
        # Every pad is an obstacle, GND ones included: sharing a net removes the
        # clearance rule but not the hole-to-hole rule, and skipping them put
        # vias straight through the Pico's ground pins.
        discs.append((TOMM(c.x), TOMM(c.y), r))
        if p.GetAttribute() in (pcbnew.PAD_ATTRIB_PTH, pcbnew.PAD_ATTRIB_NPTH):
            holes.append((TOMM(c.x), TOMM(c.y),
                          TOMM(p.GetDrillSizeX()) / 2 + VIA_DRILL / 2 + HOLE2HOLE))
            if p.GetNetCode() not in (0, gcode):
                moat_pads.append((TOMM(c.x), TOMM(c.y)))
        if p.GetNetCode() == gcode:
            gnd_pads.append((TOMM(c.x), TOMM(c.y)))

keepouts = []
for fp in b.Footprints():
    for z in fp.Zones():
        if not z.GetIsRuleArea():
            continue
        poly = z.Outline()
        for oi in range(poly.OutlineCount()):
            pts = [(TOMM(poly.CVertex(vi, oi, -1).x), TOMM(poly.CVertex(vi, oi, -1).y))
                   for vi in range(poly.VertexCount(oi, -1))]
            xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
            keepouts.append((min(xs) - KEEP, min(ys) - KEEP,
                             max(xs) + KEEP, max(ys) + KEEP))
print(f"obstacles: {len(segs)} segments, {len(discs)} discs, "
      f"{len(keepouts)} keep-outs ({time.time()-t0:.1f}s)")

gx = np.arange(EDGE, W - EDGE + 1e-9, PITCH)
gy = np.arange(EDGE, H - EDGE + 1e-9, PITCH)
PX, PY = np.meshgrid(gx, gy)
PX, PY = PX.ravel(), PY.ravel()
ok = np.ones(PX.shape, bool)

for x0, y0, x1, y1 in keepouts:
    ok &= ~((PX >= x0) & (PX <= x1) & (PY >= y0) & (PY <= y1))
for cx, cy, r in discs:
    ok &= (PX - cx) ** 2 + (PY - cy) ** 2 >= (r + KEEP) ** 2
for cx, cy, d in holes:                       # drill-to-drill spacing
    ok &= (PX - cx) ** 2 + (PY - cy) ** 2 >= d * d
# A via dropped into a pocket of pour that nothing else reaches keeps that pocket
# alive as its own little island instead of merging anything. Staying near a GND
# pad keeps the stitching on the main body of the plane.
near = np.zeros(PX.shape, bool)
for cx, cy in gnd_pads:
    near |= (PX - cx) ** 2 + (PY - cy) ** 2 <= NEAR_PAD ** 2
ok &= near
for cx, cy in moat_pads:                      # see MOAT above
    ok &= (PX - cx) ** 2 + (PY - cy) ** 2 >= MOAT ** 2
SX = np.array([s[0] for s in segs]); SY = np.array([s[1] for s in segs])
EX = np.array([s[2] for s in segs]); EY = np.array([s[3] for s in segs])
HW = np.array([s[4] for s in segs])
for i in range(len(segs)):
    dx, dy = EX[i] - SX[i], EY[i] - SY[i]
    L = dx * dx + dy * dy
    t = 0.0 if L == 0 else np.clip(((PX - SX[i]) * dx + (PY - SY[i]) * dy) / L, 0, 1)
    d2 = (PX - (SX[i] + t * dx)) ** 2 + (PY - (SY[i] + t * dy)) ** 2
    ok &= d2 >= (HW[i] + KEEP) ** 2
print(f"{int(ok.sum())} of {ok.size} lattice points are clear "
      f"({time.time()-t0:.1f}s)")

n = 0
placed = []
for x, y in zip(PX[ok], PY[ok]):
    if any((x - a) ** 2 + (y - c) ** 2 < (VIA_D + CLR) ** 2 for a, c in placed):
        continue
    v = pcbnew.PCB_VIA(b)
    v.SetPosition(pcbnew.VECTOR2I(MM(float(x)), MM(float(y))))
    v.SetWidth(MM(VIA_D)); v.SetDrill(MM(VIA_DRILL))
    v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu); v.SetNet(gnd)
    b.Add(v); v.thisown = 0
    placed.append((x, y)); n += 1
print(f"placed {n} stitching vias ({time.time()-t0:.1f}s)")

# verify before writing: measure every placed via against every obstacle
worst = []
for x, y in placed:
    for cx, cy, r in discs:
        gap = ((x - cx) ** 2 + (y - cy) ** 2) ** 0.5 - r - VIA_D / 2
        if gap < CLR - 1e-6:
            worst.append(f"via ({x:.2f},{y:.2f}) only {gap:.3f}mm from a pad/via")
    for cx, cy, d in holes:
        if ((x - cx) ** 2 + (y - cy) ** 2) ** 0.5 < d - 1e-6:
            worst.append(f"via ({x:.2f},{y:.2f}) too close to a drilled hole")
    for sx, sy, ex, ey, hw in segs:
        dx, dy = ex - sx, ey - sy
        L = dx * dx + dy * dy
        t = 0.0 if L == 0 else max(0.0, min(1.0, ((x - sx) * dx + (y - sy) * dy) / L))
        gap = (((x - (sx + t * dx)) ** 2 + (y - (sy + t * dy)) ** 2) ** 0.5
               - hw - VIA_D / 2)
        if gap < CLR - 1e-6:
            worst.append(f"via ({x:.2f},{y:.2f}) only {gap:.3f}mm from a track")
assert not worst, ("STITCHING VIA TOO CLOSE:\n  "
                   + "\n  ".join(sorted(set(worst))[:10]))
print(f"self-check passed ({time.time()-t0:.1f}s), refilling...")

pcbnew.ZONE_FILLER(b).Fill(b.Zones())
print(f"refilled ({time.time()-t0:.1f}s), saving...")
pcbnew.SaveBoard(BRD, b)
print(f"saved ({time.time()-t0:.1f}s)")
sys.stdout.flush()
os._exit(0)

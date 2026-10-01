# -*- coding: utf-8 -*-
"""Build the five MotoRecoPico footprints KiCad's stock libraries do not cover.

KiCad footprint coordinates are Y-DOWN, so every Y here is negated relative to
the EAGLE library the geometry was originally derived from.
"""
import sys, io, os
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')
import pcbnew

PRETTY = r"E:\Claude\MotoRecoPico\cad\kicad\MotoRecoPico.pretty"
os.makedirs(os.path.dirname(PRETTY), exist_ok=True)
# A .pretty library is just a directory of .kicad_mod files, and the SWIG wrapper
# for CreateLibrary is broken in 10.0.5, so make the directory ourselves. The path
# guesser also cannot sniff an empty .pretty, so name the IO plugin explicitly.
os.makedirs(PRETTY, exist_ok=True)
IO = pcbnew.PCB_IO_MGR.FindPlugin(pcbnew.PCB_IO_MGR.KICAD_SEXP)
MM = pcbnew.FromMM


def V(x, y):
    return pcbnew.VECTOR2I(MM(x), MM(y))


def new_fp(name, descr, tags, smd):
    fp = pcbnew.FOOTPRINT(None)
    fp.SetFPIDAsString(name)
    fp.SetLibDescription(descr)
    fp.SetKeywords(tags)
    fp.SetAttributes(pcbnew.FP_SMD if smd else pcbnew.FP_THROUGH_HOLE)
    fp.Reference().SetText("REF**")
    fp.Reference().SetPosition(V(0, -3))
    fp.Reference().SetLayer(pcbnew.F_SilkS)
    fp.Value().SetText(name)
    fp.Value().SetPosition(V(0, 3))
    fp.Value().SetLayer(pcbnew.F_Fab)
    return fp


def tht(fp, name, x, y, drill, dia, rect=False):
    p = pcbnew.PAD(fp)
    p.SetNumber(name)
    p.SetAttribute(pcbnew.PAD_ATTRIB_PTH)
    p.SetShape(pcbnew.PAD_SHAPE_RECTANGLE if rect else pcbnew.PAD_SHAPE_CIRCLE)
    p.SetSize(pcbnew.VECTOR2I(MM(dia), MM(dia)))
    p.SetDrillSize(pcbnew.VECTOR2I(MM(drill), MM(drill)))
    p.SetLayerSet(p.PTHMask())
    p.SetPosition(V(x, y))
    fp.Add(p)
    return p


def smd(fp, name, x, y, dx, dy):
    p = pcbnew.PAD(fp)
    p.SetNumber(name)
    p.SetAttribute(pcbnew.PAD_ATTRIB_SMD)
    p.SetShape(pcbnew.PAD_SHAPE_RECTANGLE)
    p.SetSize(pcbnew.VECTOR2I(MM(dx), MM(dy)))
    p.SetLayerSet(p.SMDMask())
    p.SetPosition(V(x, y))
    fp.Add(p)
    return p


def line(fp, x1, y1, x2, y2, layer=pcbnew.F_SilkS, w=0.12):
    s = pcbnew.PCB_SHAPE(fp)
    s.SetShape(pcbnew.SHAPE_T_SEGMENT)
    s.SetStart(V(x1, y1)); s.SetEnd(V(x2, y2))
    s.SetLayer(layer); s.SetWidth(MM(w))
    fp.Add(s)


def rect(fp, x1, y1, x2, y2, layer=pcbnew.F_SilkS, w=0.12):
    for a, b, c, d in ((x1, y1, x2, y1), (x2, y1, x2, y2),
                       (x2, y2, x1, y2), (x1, y2, x1, y1)):
        line(fp, a, b, c, d, layer, w)


def courtyard(fp, x1, y1, x2, y2):
    rect(fp, x1, y1, x2, y2, pcbnew.F_CrtYd, 0.05)


def save(fp, name):
    IO.FootprintSave(PRETTY, fp)
    print(f"  wrote {name}  pads={fp.GetPadCount()}")


# ---------------------------------------------------------------- 1. MQS 8-way
f = new_fp("MQS-8-RA-1-967658-1",
           "TE 1-967658-1 MQS .63 automotive header, 8 pos, 2 rows, right angle THT. "
           "Origin = housing front reference line on the connector centreline; put "
           "the board edge at Y=0 so the mating face points off-board (-Y). "
           "Holes are at +Y (into the board). Drawing 967658 Rev.C3, 1.0mm min hole, "
           "1.1mm drill used. Housing 19.6mm wide, 18.1mm tall, extends 8mm over the board.",
           "MQS automotive connector TE 967658", smd=False)
for n, x, y in [("1", 7, 3.76), ("2", 3, 3.76), ("3", -1, 3.76), ("4", -5, 3.76),
                ("5", 5, 6.30), ("6", 1, 6.30), ("7", -3, 6.30), ("8", -7, 6.30)]:
    tht(f, n, x, y, 1.1, 1.8, rect=(n == "1"))
rect(f, -9.7, 0.2, 9.7, 8.0)                          # kept off the y=0 board edge
line(f, -9.8, 0, 9.8, 0, pcbnew.F_Fab, 0.2)          # board edge / datum
line(f, 8.6, 2.4, 9.4, 2.4)                           # pin-1 tick
# the housing sits on the board for its full 19.6 x 8mm - nothing else fits under it
courtyard(f, -9.9, -0.1, 9.9, 8.1)
save(f, "MQS-8-RA-1-967658-1")

# ------------------------------------------------------------- 2. GPS solder pads
f = new_fp("GPS-5P-SMD-2.54",
           "GT-502MGG-N bare-wire landing pads, single sided, no through holes. "
           "Five 2.0 x 3.0mm pads on 2.54mm pitch. "
           "1 VCC, 2 GND, 3 RXD, 4 TXD, 5 PPS (labels are MODULE side: pad 3 "
           "carries the Pico's UART TX, so the module's RXD wire lands there - "
           "v1 printed T/R the other way round and hookup proved it wrong). "
           "Strain relief is the enclosure's job - no cable-tie holes, to save area.",
           "GPS wire solder pads GT-502MGG", smd=True)
for i, lab in enumerate("VGRTP"):
    x = -5.08 + 2.54 * i
    smd(f, str(i + 1), x, 0, 2.0, 3.0)
    t = pcbnew.PCB_TEXT(f)
    t.SetText(lab); t.SetPosition(V(x, -2.4)); t.SetLayer(pcbnew.F_SilkS)
    t.SetTextSize(pcbnew.VECTOR2I(MM(0.8), MM(0.8))); t.SetTextThickness(MM(0.12))
    f.Add(t)
line(f, -6.5, 1.9, 6.5, 1.9)
courtyard(f, -6.4, -1.8, 6.4, 1.8)
save(f, "GPS-5P-SMD-2.54")

# --------------------------------------------------------------- 3. side-view LED
f = new_fp("LED-SideView-2.8x1.2",
           "Side-view chip LED, 2.8 x 1.2 x 0.8mm body. Soldering pattern common to "
           "Kodenshi LP812-010(T) and Sharp GM4ZR83200AE: two 1.4 x 0.9mm pads, 1.0mm "
           "gap, centres at X = +/-1.2mm. Pad 1 = CATHODE, pad 2 = ANODE. "
           "LIGHT LEAVES THROUGH THE +Y LONG SIDE - point +Y at the enclosure window.",
           "LED side view sidelook", smd=True)
smd(f, "1", -1.2, 0, 1.4, 0.9)
smd(f, "2", 1.2, 0, 1.4, 0.9)
rect(f, -1.4, -0.6, 1.4, 0.6, pcbnew.F_Fab, 0.1)
line(f, -2.05, -0.6, -2.05, 0.6, w=0.25)              # cathode bar, clear of pad 1
line(f, -1.6, 0.9, 1.6, 0.9, pcbnew.F_Fab, 0.3)       # emitting face
courtyard(f, -2.1, -0.85, 2.1, 1.05)
save(f, "LED-SideView-2.8x1.2")

# --------------------------------------------------------------- 4. M78AR05 SIP-3
f = new_fp("MinMax-M78AR05-SIP3",
           "MinMax M78AR05-0.5 switching regulator, SIP-3, LM78xx pin order: "
           "1 = +Vin, 2 = GND, 3 = +Vout, 2.54mm pitch. Pin section 0.70 x 0.25mm, "
           "1.0mm drill clears it. Body 11.5 x 7.55 x 10.2mm, standing off the pin row "
           "in -Y. Origin = pin 2 (GND). Tallest part after the vehicle connector.",
           "DCDC regulator MinMax M78AR05 SIP3", smd=False)
tht(f, "1", -2.54, 0, 1.0, 1.8, rect=True)
tht(f, "2", 0, 0, 1.0, 1.8)
tht(f, "3", 2.54, 0, 1.0, 1.8)
rect(f, -5.75, -5.55, 5.75, 2.0)
for lab, x in (("Vin", -4.4), ("Vout", 1.6)):
    t = pcbnew.PCB_TEXT(f)
    t.SetText(lab); t.SetPosition(V(x, 1.4)); t.SetLayer(pcbnew.F_SilkS)
    t.SetTextSize(pcbnew.VECTOR2I(MM(0.8), MM(0.8))); t.SetTextThickness(MM(0.12))
    f.Add(t)
courtyard(f, -5.9, -5.7, 5.9, 2.15)
save(f, "MinMax-M78AR05-SIP3")

# --------------------------------------------------- 5. 4030 inductor, hand-solder
# v4.2: the stock L_Abracon_ASPI-4030S land (1.1 x 3.4mm pads at +/-1.5) leaves no
# copper beyond the 4x4mm body to put an iron on. Each pad is stretched 1.0mm
# OUTWARD (inner edge stays at 0.95, outer edge 2.05 -> 3.05), so the footprint
# is 6.1mm along the pad axis. Body, silk and courtyard follow the stock part.
f = new_fp("L_4030_HandSolder",
           "SMD shielded power inductor 4x4x3mm (Abracon ASPI-4030S land), pads "
           "stretched 1.0mm outward for hand soldering: 2.1 x 3.4mm at X = +/-2.0. "
           "Datasheet https://abracon.com/Magnetics/power/ASPI-4030S.pdf",
           "inductor abracon smd shielded handsolder", smd=True)
smd(f, "1", -2.0, 0, 2.1, 3.4)
smd(f, "2", 2.0, 0, 2.1, 3.4)
line(f, -1.99, -2.12, 1.99, -2.12)
line(f, -1.99, 2.12, 1.99, 2.12)
rect(f, -2, -2, 2, 2, pcbnew.F_Fab, 0.1)
courtyard(f, -3.3, -2.25, 3.3, 2.25)
save(f, "L_4030_HandSolder")

# ------------------------------------------------------------------- verify
print("\nreloading from disk:")
for n in pcbnew.FootprintEnumerate(PRETTY):
    m = pcbnew.FootprintLoad(PRETTY, n)
    assert m is not None, n
    print(f"  {n:28s} pads={m.GetPadCount():2d} "
          f"{[p.GetNumber() for p in m.Pads()]}")

# Regenerate every manufacturing deliverable from the routed board:
#   gerbers (4 copper + mask/paste/silk/edge), PTH/NPTH drills + maps, JLC BOM
#   and CPL (SMT-only and all-parts), designator cross-check, top/bottom SVG and
#   the gerber zip. Run after try_thermal.ps1 has produced a passing board.
$d  = "E:\Claude\MotoRecoPico\cad\kicad"
$sc = "$d\_scripts"
$py = "d:\Program Files\KiCad\10.0\bin\python.exe"
$k  = "d:\Program Files\KiCad\10.0\bin\kicad-cli.exe"
$brd = "$d\MotoRecoPico.kicad_pcb"
$fab = "$d\fab"

& $k pcb export gerbers --layers "F.Cu,In1.Cu,In2.Cu,B.Cu,F.Mask,B.Mask,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,Edge.Cuts" -o $fab $brd | Out-Null
& $k pcb export drill --format excellon --excellon-separate-th --generate-map --map-format gerberx2 -o "$fab\" $brd | Out-Null
# one row per part (user request), regenerated from the schematic every time -
# jlc_bom.py splits it into SMT / through-hole. Hand edits to the csv are lost.
& $k sch export bom --fields "Reference,Value,Footprint,`${QUANTITY}" --labels "Designator,Comment,Footprint,Qty" -o "$fab\MotoRecoPico-bom.csv" "$d\MotoRecoPico.kicad_sch" | Out-Null
& $py "$sc\jlc_bom.py"
& $py "$sc\jlc_cpl.py"
& $py "$sc\xcheck.py"
& $k pcb export svg --layers "F.Cu,F.Silkscreen,F.Mask,Edge.Cuts" --page-size-mode 2 --exclude-drawing-sheet -o "$fab\top.svg" $brd 2>&1 | Select-String "プロット|Plotted" | Out-Null
& $k pcb export svg --layers "B.Cu,B.Silkscreen,B.Mask,Edge.Cuts" --page-size-mode 2 --exclude-drawing-sheet --mirror -o "$fab\bottom.svg" $brd 2>&1 | Select-String "プロット|Plotted" | Out-Null
$parts = @("*.gtl","*.gbl","*.g1","*.g2","*.gts","*.gbs","*.gtp","*.gbp","*.gto","*.gbo","*.gm1","*.drl","*.gbrjob") | ForEach-Object { "$fab\$_" }
Compress-Archive -Force -Path $parts -DestinationPath "$fab\MotoRecoPico-gerber.zip"
"fab: gerbers, drills, BOM/CPL, SVGs and zip regenerated in $fab"

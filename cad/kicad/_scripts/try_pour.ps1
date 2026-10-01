param([string]$MinThk)
$py = "d:\Program Files\KiCad\10.0\bin\python.exe"
$k = "d:\Program Files\KiCad\10.0\bin\kicad-cli.exe"
$sp = "C:\Users\root\AppData\Local\Temp\claude\E--Claude-MotoRecoPico\4b4096a9-4169-4622-a77d-e17c28653a27\scratchpad"
$d = "E:\Claude\MotoRecoPico\cad\kicad"
(Get-Content "$sp\ses_import.py") -replace 'SetMinThickness\(MM\([0-9.]+\)\)', "SetMinThickness(MM($MinThk))" | Set-Content "$sp\ses_import.py" -Encoding utf8
& $py "$sp\mrp_brd.py" 2>$null | Out-Null
& $py "$sp\ses_import.py" 2>$null | Out-Null
& $py "$sp\gnd_stitch.py" 2>$null | Out-Null
& $k pcb drc --format json --severity-error -o "$d\drc.json" "$d\MotoRecoPico.kicad_pcb" | Out-Null
$isl = (Select-String -Path "$d\MotoRecoPico.kicad_pcb" -Pattern '\(island' -AllMatches | Measure-Object).Count
$j = Get-Content "$d\drc.json" -Raw | ConvertFrom-Json
"min_thickness=$MinThk : violations=$($j.violations.Count) unconnected=$($j.unconnected_items.Count) islands=$isl"

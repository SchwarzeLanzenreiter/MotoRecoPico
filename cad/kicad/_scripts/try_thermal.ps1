# Hunt for a routing + thermal-gap combination where every GND pad gets its spoke.
#
# Each seed rebuilds the board from scratch (mrp_brd randomises net pin order via
# fresh UUIDs, which steers Freerouting differently), routes it, and then tries
# pours at successively shorter thermal gaps. ses_import must always start from
# the just-routed board - re-running it on an already-poured board stacks a second
# set of zones on top (zones_intersect), which is how the first version of this
# loop went wrong. The pristine post-route board is therefore snapshotted and
# restored before every pour attempt. DRC counting goes through python because
# ConvertFrom-Json chokes on the report's CP932 mojibake and silently yields 0/0.
param([int]$Seeds = 4, [string]$Layers = '2', [string]$Rule = '0.127')
$sc = "E:\Claude\MotoRecoPico\cad\kicad\_scripts"
$d  = "E:\Claude\MotoRecoPico\cad\kicad"
$py = "d:\Program Files\KiCad\10.0\bin\python.exe"
$k  = "d:\Program Files\KiCad\10.0\bin\kicad-cli.exe"
$fr = "E:\Claude\_tools\fr\freerouting\freerouting.exe"
$brd = "$d\MotoRecoPico.kicad_pcb"
$pristine = "$d\MotoRecoPico_routed.kicad_pcb"
$env:MRP_LAYERS = $Layers
$env:MRP_RULE = $Rule

foreach ($seed in 1..$Seeds) {
    & $py "$sc\mrp_brd.py" A 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) { "seed $seed : mrp_brd failed"; continue }
    & $py "$sc\dsn_export.py" planes 0.2 | Out-Null
    $log = & $fr -de "$d\MotoRecoPico.dsn" -do "E:\Claude\_tools\fr_try.ses" -mp 500 -oit 0.01 2>&1
    $line = ($log | Select-String "session completed").Line
    $n = 9999
    if ($line -match "\((\d+) unrouted\)") { $n = [int]$Matches[1] }
    elseif ($line -match "final score:") { $n = 0 }
    if ($n -ne 0) { "seed $seed : $n unrouted, skip"; continue }
    Copy-Item "E:\Claude\_tools\fr_try.ses" "$d\MotoRecoPico.ses" -Force
    Copy-Item $brd $pristine -Force
    foreach ($gap in @('0.3', '0.25', '0.2')) {
        Copy-Item $pristine $brd -Force
        $env:MRP_THERMAL_GAP = $gap
        & $py "$sc\ses_import.py" | Out-Null
        & $py "$sc\gnd_stitch.py" | Out-Null
        & $k pcb drc --format json --severity-error -o "$d\drc.json" $brd | Out-Null
        $c = (& $py "$sc\drc_count.py" "$d\drc.json") -split ' '
        "seed $seed gap $gap : violations=$($c[0]) unconnected=$($c[1])"
        if ([int]$c[0] -eq 0 -and [int]$c[1] -eq 0) {
            # clean DRC is not enough - the oscillator nets must stay off the
            # inner layers (xtal_check exits 1 when a seed routed them there)
            & $py "$sc\xtal_check.py"
            if ($LASTEXITCODE -ne 0) { "seed $seed : XTAL on inner layers, reject"; break }
            "SUCCESS: seed $seed, thermal gap $gap"
            Remove-Item $pristine -Force
            exit 0
        }
    }
}
"no combination reached 0 unconnected"
exit 1

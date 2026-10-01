param([int]$Runs = 20)
# Freerouting lives outside the session scratchpad - temp dirs get purged
# between sessions and took the first copy with them.
$sp = "E:\Claude\_tools"
$d = "E:\Claude\MotoRecoPico\cad\kicad"
$fr = "$sp\fr\freerouting\freerouting.exe"
$best = 9999
$bestLog = ""
for ($i = 1; $i -le $Runs; $i++) {
    $tmp = "$sp\fr_try.ses"
    $log = & $fr -de "$d\MotoRecoPico.dsn" -do $tmp -mp 500 -oit 0.01 2>&1
    $line = ($log | Select-String "session completed").Line
    # a fully routed board prints no "(N unrouted)" suffix at all
    $n = 9999
    if ($line -match "\((\d+) unrouted\)") { $n = [int]$Matches[1] }
    elseif ($line -match "final score:") { $n = 0 }
    "run $i : $n unrouted"
    if ($n -lt $best) {
        $best = $n
        Copy-Item $tmp "$d\MotoRecoPico.ses" -Force
        $bestLog = ($log | Select-String "could not be routed" -Context 0, 12) -join "`n"
    }
    if ($best -eq 0) { break }
}
"BEST = $best unrouted"
if ($best -gt 0) { $bestLog }

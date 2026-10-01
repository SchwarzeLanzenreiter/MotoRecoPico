# builds and runs the host tests with the MSVC build tools that are on this machine.
# these tests only cover the modules with no hardware in them (nmea, record, power, settings,
# wallclock), which is exactly the code that has to be right before a board exists.
# the motorecopico_tests target runs the very same tests on a Pico.
#
#   .\build.ps1              build and run the tests

$ErrorActionPreference = "Stop"

$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$src  = Resolve-Path (Join-Path $here "..\src")
$out  = Join-Path $here "build"

$vcvars = Get-ChildItem "C:\Program Files*\Microsoft Visual Studio\*\*\VC\Auxiliary\Build\vcvars64.bat" -ErrorAction SilentlyContinue | Select-Object -First 1

if (-not $vcvars) {
    throw "vcvars64.bat not found. install the Visual Studio build tools, or use the Makefile with gcc."
}

New-Item -ItemType Directory -Force -Path $out | Out-Null

# vcvars and cl have to run in the same cmd session, so the whole build goes into one batch file.
# /std:c11 for _Static_assert, /W3 because the sources are meant to be warning free
$bat = Join-Path $out "build.bat"

# note: the build runs from the output directory. a quoted /Fo ending in a backslash would have
# the backslash escape the closing quote and swallow the source file names
@"
@echo off
call "$($vcvars.FullName)" >nul
if errorlevel 1 exit /b 1
cd /d "$out"
cl /nologo /std:c11 /W3 /I"$src" /Fe:test_nmea.exe "$here\test_nmea.c" "$src\nmea.c" "$src\record.c" "$src\power.c" "$src\settings.c" "$src\wallclock.c"
if errorlevel 1 exit /b 1
"@ | Out-File -FilePath $bat -Encoding ascii

cmd /c "`"$bat`""

if ($LASTEXITCODE -ne 0) {
    throw "compilation failed"
}

& "$out\test_nmea.exe"

exit $LASTEXITCODE

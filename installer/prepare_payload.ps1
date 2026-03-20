$ErrorActionPreference = "Stop"

# Copies the best available runtime into installer\payload\
# Priority:
#   1) native C++ build: ..\cpp\mpp.exe
#   2) python-built exe: ..\dist\mpp.exe

$root = Split-Path -Parent $PSScriptRoot
$payload = Join-Path $PSScriptRoot "payload"
New-Item -ItemType Directory -Force $payload | Out-Null

$native = Join-Path $root "cpp\mpp.exe"
$pyexe  = Join-Path $root "dist\mpp.exe"

if (Test-Path $native) {
  Copy-Item $native (Join-Path $payload "mpp.exe") -Force
  Write-Host "Payload runtime: native C++ mpp.exe"
} elseif (Test-Path $pyexe) {
  Copy-Item $pyexe (Join-Path $payload "mpp.exe") -Force
  Write-Host "Payload runtime: PyInstaller mpp.exe"
} else {
  throw "No runtime found. Build one: (1) cpp\build.bat or (2) python build_exe.py"
}

Copy-Item (Join-Path $root "SPEC.md") (Join-Path $payload "SPEC.md") -Force

$ico = Join-Path $root "assets\mpp_file_icon.ico"
if (Test-Path $ico) {
  Copy-Item $ico (Join-Path $payload "mpp.ico") -Force
  Write-Host "Payload icon: mpp.ico"
} else {
  Write-Host "Payload icon: missing (assets\\mpp_file_icon.ico not found)"
}

$examplesSrc = Join-Path $root "examples"
$examplesDst = Join-Path $payload "examples"
if (Test-Path $examplesDst) { Remove-Item $examplesDst -Recurse -Force }
Copy-Item $examplesSrc $examplesDst -Recurse -Force

# Include the windowed app example
$appsSrc = Join-Path $root "apps\terminal_ui_window"
$appsDst = Join-Path $payload "apps\terminal_ui_window"
if (Test-Path $appsDst) { Remove-Item $appsDst -Recurse -Force }
if (Test-Path $appsSrc) {
  New-Item -ItemType Directory -Force (Join-Path $payload "apps") | Out-Null
  Copy-Item $appsSrc $appsDst -Recurse -Force
}

Write-Host "Done. Payload ready at: $payload"


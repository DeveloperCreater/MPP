# M++ Installer (Windows)

This folder contains a **ready-to-build Windows installer** project for M++.

## What it installs

- `mpp.exe` into `C:\Program Files\Mpp\`
- `SPEC.md` and `examples\`
- Optional: adds install folder to **PATH** (so you can run `mpp` anywhere)

## Prepare payload (pick best runtime)

Run this first:

```powershell
powershell -ExecutionPolicy Bypass -File installer\prepare_payload.ps1
```

It copies:
- `cpp\mpp.exe` if you built the native C++ runtime, otherwise
- `dist\mpp.exe` (PyInstaller build)

into `installer\payload\mpp.exe`.

## Build the installer

Install Inno Setup, then compile `installer\mpp.iss` using `ISCC.exe` (Inno Setup Compiler).

The output will be something like `Mpp-Setup.exe`.


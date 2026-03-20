@echo off
setlocal

REM Unified M++ launcher (prefers native runtime).
REM Priority:
REM  1) cpp\mpp.exe   (native C++ build, recommended)
REM  2) dist\mpp.exe  (PyInstaller-built exe)
REM  3) python -m mpp (dev fallback)

set ROOT=%~dp0

if exist "%ROOT%cpp\mpp.exe" (
  "%ROOT%cpp\mpp.exe" %*
  exit /b %ERRORLEVEL%
)

if exist "%ROOT%dist\mpp.exe" (
  "%ROOT%dist\mpp.exe" %*
  exit /b %ERRORLEVEL%
)

python -m mpp %*
exit /b %ERRORLEVEL%

@echo off
setlocal

REM Same as repo-root mpp.cmd, but in bin/ for PATH usage.
set ROOT=%~dp0..

if exist "%ROOT%\cpp\mpp.exe" (
  "%ROOT%\cpp\mpp.exe" %*
  exit /b %ERRORLEVEL%
)

if exist "%ROOT%\dist\mpp.exe" (
  "%ROOT%\dist\mpp.exe" %*
  exit /b %ERRORLEVEL%
)

python -m mpp %*
exit /b %ERRORLEVEL%

@echo off

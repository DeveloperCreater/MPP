; Inno Setup script for M++ (Windows installer)
; Requires Inno Setup to compile (ISCC.exe).
; Build flow:
;   1) Run: powershell -ExecutionPolicy Bypass -File installer\prepare_payload.ps1
;   2) Compile this .iss with Inno Setup to produce M++-Setup.exe

[Setup]
AppName=M++
AppVersion=1.1.0
DefaultDirName={pf}\Mpp
DefaultGroupName=M++
OutputBaseFilename=Mpp-Setup
Compression=lzma
SolidCompression=yes
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
SetupIconFile=payload\mpp.ico
UninstallDisplayIcon={app}\mpp.ico

[Files]
Source: "payload\mpp.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "payload\SPEC.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "payload\mpp.ico"; DestDir: "{app}"; DestName: "mpp.ico"; Flags: ignoreversion
Source: "payload\examples\*"; DestDir: "{app}\examples"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\M++ (Run Hello)"; Filename: "{app}\mpp.exe"; Parameters: "-r ""{app}\examples\hello.mpp"""
Name: "{group}\M++ Shell"; Filename: "{cmd}"; Parameters: "/k """"{app}\mpp.exe"" --help"""

[Tasks]
Name: "addtopath"; Description: "Add M++ to PATH (recommended)"; Flags: unchecked

[Registry]
; Add to PATH (current user) if task selected
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; \
    ValueData: "{olddata};{app}"; Tasks: addtopath; Check: NeedsAddPath(ExpandConstant('{app}'))

; File association (.mpp)
Root: HKCR; Subkey: ".mpp"; ValueType: string; ValueData: "Mpp.Source"; Flags: uninsdeletevalue
Root: HKCR; Subkey: "Mpp.Source"; ValueType: string; ValueData: "M++ Source File"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Mpp.Source\DefaultIcon"; ValueType: string; ValueData: "{app}\mpp.ico"
Root: HKCR; Subkey: "Mpp.Source\shell\open\command"; ValueType: string; ValueData: """{app}\mpp.exe"" -r ""%1"""

[Code]
function NeedsAddPath(Param: string): Boolean;
var
  Paths: string;
begin
  if not RegQueryStringValue(HKCU, 'Environment', 'Path', Paths) then
    Paths := '';
  Result := Pos(';' + Lowercase(Param) + ';', ';' + Lowercase(Paths) + ';') = 0;
end;


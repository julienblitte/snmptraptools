; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B7ABA127-3FBE-4118-BD12-A2B9D372D1F9}
AppName=Snmptraptools
AppVersion=3.00a
AppPublisherURL=http://marin.jb.free.fr/snmptraptools
AppSupportURL=http://marin.jb.free.fr/snmptraptools
AppUpdatesURL=http://marin.jb.free.fr/snmptraptools
DefaultDirName={pf}\Snmptraptools
DefaultGroupName=Snmptraptools
OutputBaseFilename=setup_snmptraptools_3.00a
LicenseFile=lgpl3.txt
OutputDir=..\bin
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "standalone"; Description: "Classic (recommended)"
Name: "winsnmp"; Description: "Winsnmp version"

[Components]
Name: "standalone"; Description: "classic windows service"; Types: standalone; Flags: fixed
Name: "winsnmp"; Description: "winsnmp windows service"; Types: winsnmp

[Tasks]
Name: "runservice"; Description: "Start service";
;Name: "runconfig"; Description: "Configure service at end of setup";  Flags: unchecked

[Files]
Source: "..\bin\snmptrapconfig.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\snmptrapdispatcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\snmptraphandler_standalone.exe"; DestDir: "{app}"; Components: standalone; Flags: ignoreversion; DestName: "snmptraphandler.exe"
Source: "..\bin\snmptraphandler_winsnmp.exe"; DestDir: "{app}"; Components: winsnmp; Flags: ignoreversion; DestName: "snmptraphandler.exe"
Source: "..\bin\import-mib.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "import-mib.exe"
Source: "..\tools\snmptrapemail\download sendEmail.html"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\tools\snmptrapemail\snmptrapemail.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\tools\snmptrapmessagebox\snmptrapmessagebox.exe"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Configure"; Filename: "{app}\snmptrapconfig.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,Snmptraptools}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\snmptraphandler.exe"; Parameters: "install"
Filename: "{sys}\sc.exe"; Parameters: "start snmptraptools"; Tasks: runservice

[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\snmptraptools\dispatcher\0"; ValueType: string; ValueData: "1.3.6.1.4.1"
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\snmptraptools\dispatcher\0"; ValueType: string; ValueName: "description"; ValueData: "sample messagebox"
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Services\snmptraptools\dispatcher\0"; ValueType: dword; ValueName: "plugin"; ValueData: "$4247534D"

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop snmptraptools"
Filename: "{app}\snmptraphandler.exe"; Parameters: "uninstall"

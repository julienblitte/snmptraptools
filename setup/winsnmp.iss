; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{B7ABA127-3FBE-4118-BD12-A2B9D372D1F9}
AppName=Snmptraptools
;AppVersion=2.2b
AppVerName=Snmptraptools - winsnmp version 2.2b
AppPublisherURL=http://marin.jb.free.fr/snmptraptools
AppSupportURL=http://marin.jb.free.fr/snmptraptools
AppUpdatesURL=http://marin.jb.free.fr/snmptraptools
DefaultDirName={pf}\Snmptraptools
DefaultGroupName=Snmptraptools
OutputBaseFilename=setup_winsnmp.exe
OutputDir=..\bin
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\bin\snmptrapconfig.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\snmptrapdispatcher.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\bin\snmptraphandler_winsnmp.exe"; DestDir: "{app}"; Flags: ignoreversion; DestName: "snmptraphandler.exe"
Source: "..\tools\sendemail\download sendEmail.html"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\tools\sendemail\snmptrap_email.bat"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Configure"; Filename: "{app}\snmptrapconfig.exe"; WorkingDir: "{app}"
Name: "{group}\{cm:UninstallProgram,Snmptraptools}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\snmptraphandler.exe"; Parameters: "install"
Filename: "{sys}\sc.exe"; Parameters: "start snmptraphandler"

[UninstallRun]
Filename: "{sys}\sc.exe"; Parameters: "stop snmptraphandler"
Filename: "{app}\snmptraphandler.exe"; Parameters: "uninstall"

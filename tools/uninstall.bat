@echo off

net stop snmptraphandler
snmptraphandler uninstall

cd %SystemRoot%\System32\
rm snmptraphandler.exe
rm snmptrapdispatcher.exe
rm snmptrapconfig.exe


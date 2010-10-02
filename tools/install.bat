@echo off

copy snmptrap*.exe %SystemRoot%\System32\

cd %SystemRoot%\System32\
snmptraphandler install

net start snmptraphandler


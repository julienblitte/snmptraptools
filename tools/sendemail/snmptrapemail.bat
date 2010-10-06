@echo off

set from=username@mycompany.com
set to=username@mycompany.com
set server=mymailserver
set port=25
set tls=yes
set user=username
set password=password

set body=Device %1 send trap with oid %2 - trap type is (%3, %4)
set subject=snmp trap alert - host %1

sendemail -f "%from%" -t "%to%" -u "%subject%" -m "%body%" -s "%server%:%port%" -o tls="%tls%" -xu "%user%" -xp "%password%" >> sendEmail.log


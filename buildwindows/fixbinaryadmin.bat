windeployqt.exe "%1"
ping 127.0.0.1 -n2 -w 2000 > NUL
"C:\Program Files (x86)\Windows Kits\8.1\bin\x86\mt.exe" -manifest "%2" -outputresource:"%1;1"

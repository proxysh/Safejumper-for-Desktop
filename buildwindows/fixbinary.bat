windeployqt.exe "%1"
ping 127.0.0.1 -n1 -w 2000 > NUL
"C:\Program Files\Microsoft SDKs\Windows\v7.1\bin\mt.exe" -manifest "%2" -outputresource:"%1;1"


REM Signing a file: (cross-certificate)
\\oriole-03-int\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.verisign.com/scripts/timstamp.dll /v \\oriole-03-int\treescan\installers\v.1.5.x\install-1_5_windows.exe

REM To verify the file is signed correctly:
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\treescan\installers\v.1.5.x\install-1_5_windows.exe


REM Signing a file: (cross-certificate)
\\oriole-03-int\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /v /ac \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\MSCV-VSClass3.cer /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign-2017.pfx /p "YKtNv&otBfX7" /n "Information Management Services, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll \\oriole-03-int\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe

REM To verify the file is signed correctly:
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe

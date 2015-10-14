
REM Signing a file: (cross-certificate)
\\nfsl.omni.imsweb.com\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /v /ac \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\MSCV-VSClass3.cer /f \\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p %%iaUzoA0cl!p /n "Information Management Services, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll \\nfsl.omni.imsweb.com\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe

REM To verify the file is signed correctly:
\\nfsl.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\nfsl.omni.imsweb.com\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe


REM Signing a file: (cross-certificate)
\\nfsf.omni.imsweb.com\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\nfsf.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /v /ac \\nfsf.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\MSCV-VSClass3.cer /f \\nfsf.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p J3llifi$h /n "Information Management Services, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll \\nfsf.omni.imsweb.com\treescan\installers\v.1.2.x\install-1_2_windows.exe

REM To verify the file is signed correctly:
\\nfsf.omni.imsweb.com\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\nfsf.omni.imsweb.com\treescan\installers\v.1.2.x\install-1_2_windows.exe

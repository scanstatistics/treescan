

REM Codesigning a GUI exe file.
\\oriole-03-int\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.verisign.com/scripts/timstamp.dll /v \\oriole-03-int\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe

REM Verify the GUI exe file is codesigned correctly.
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe


REM Compile the InnoSetup installer.
"C:\Program Files (x86)\Inno Setup 6\iscc.exe" \\oriole-03-int\treescan\build\treescan\installers\inno-setup\treescan.iss


REM Codesigning a installer exe file.
\\oriole-03-int\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe sign /f \\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx /p "&4L(JyhyOmwF)$Z" /t http://timestamp.verisign.com/scripts/timstamp.dll /v \\oriole-03-int\treescan\installers\v.2.0.x\install-2_0_windows.exe

REM Verify the installer exe file is codesigned correctly.
\\oriole-03-int\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe verify /pa /v \\oriole-03-int\treescan\installers\v.2.0.x\install-2_0_windows.exe
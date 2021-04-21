@echo off

REM Script which code signs TreeScan executable then builds TreeScan installer and finally code signs that installer.
REM This steps can't be executed from build environment -- currently Linux.

REM script definitions
set fileshare=\\oriole-03-int

set treescanversion=2.0
set treescanversionf=2_0
set treescanexe=%fileshare%\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe
set treescaninstaller=%fileshare%\treescan\installers\v.%treescanversion%.x\install-%treescanversionf%_windows.exe

set javajdk=%fileshare%\treescan\build\packages\java\jdk-15.0.2+7_adopt_windows_x64
set runtimeoutput=%fileshare%\treescan\build\treescan\installers\java\jre

set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\treescan\build\treescan\installers\inno-setup\treescan.iss

set signtool=%fileshare%\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe
set certificate=%fileshare%\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx
set timestamp=http://timestamp.digicert.com/
set password="&4L(JyhyOmwF)$Z"


REM Codesigning a GUI exe file.
%fileshare%\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose %signtool% sign /f %certificate% /p %password% /t %timestamp% /v %treescanexe%

REM Verify the GUI exe file is codesigned correctly.
%signtool% verify /pa /v %treescanexe%

REM Create Java runtime
if exist %runtimeoutput% rmdir %runtimeoutput% /s /q
%javajdk%\bin\jlink.exe --module-path %javajdk%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto --output %runtimeoutput% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign /f %certificate% /p %password% /t %timestamp% /v %treescaninstaller%

REM Verify the installer exe file is codesigned correctly.
%signtool% verify /pa /v %treescaninstaller%

@echo off

REM Script which code signs TreeScan executable then builds TreeScan installer and finally code signs that installer.
REM This steps can't be executed from build environment -- currently Linux.

REM script definitions
set fileshare=\\oriole-03-int

set treescanversion=2.2
set treescanversionf=2_2
set treescanexe=%fileshare%\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe
set treescaninstaller=%fileshare%\treescan\installers\v.%treescanversion%.x\install-%treescanversionf%_windows.exe

set javajdkx64=%fileshare%\treescan\build\packages\java\jdk-17.0.5+8_windows_x64
set runtimeoutputx64=%fileshare%\treescan\build\treescan\installers\java\jre_x64
set javajdkx86=%fileshare%\treescan\build\packages\java\jdk-17.0.5+8_windows_x86
set runtimeoutputx86=%fileshare%\treescan\build\treescan\installers\java\jre_x86


set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\treescan\build\treescan\installers\inno-setup\treescan.iss

set signtool=%fileshare%\imsadmin\code.sign.cert.ms.auth\signtool.exe
set certificate=%fileshare%\imsadmin\code.sign.cert.ms.auth\ims.pfx
set timestamp=http://timestamp.digicert.com/
set password="&4L(JyhyOmwF)$Z"


REM Codesigning a GUI exe file.
%fileshare%\treescan\build\treescan\installers\izpack\sign4j\sign4j.exe --verbose %signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %password% %treescanexe%

REM Verify the GUI exe file is codesigned correctly.
%signtool% verify /pa /v %treescanexe%

REM Create Java 64-bit runtime
if exist %runtimeoutputx64% rmdir %runtimeoutputx64% /s /q
%javajdkx64%\bin\jlink.exe --module-path %javajdkx64%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx64% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Create Java 32-bit runtime
if exist %runtimeoutputx86% rmdir %runtimeoutputx86% /s /q
%javajdkx86%\bin\jlink.exe --module-path %javajdkx86%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx86% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign /tr %timestamp% /td sha256 /fd sha256 /f %certificate% /p %password% %treescaninstaller%

REM Verify the installer exe file is codesigned correctly.
%signtool% verify /pa /v %treescaninstaller%

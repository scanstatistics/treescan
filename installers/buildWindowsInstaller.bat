@echo off

REM Script which code signs SaTScan executable then builds SaTScan installer and finally code signs that installer.
REM This steps can't be executed from build environment -- currently Linux.

REM script definitions
set fileshare=\\oriole-03-int

set satscanversion=10.0
set satscanversionf=10_0
set satscanexe=%fileshare%\satscan\build.area\satscan\java_application\jni_application\dist\SaTScan.exe
set satscaninstaller=%fileshare%\satscan\installers\v.%satscanversion%.x\install-%satscanversionf%_windows.exe

set javajdk=%fileshare%\satscan\installers\install.applications\java\jdk-15.0.2+7_adopt_windows_x64
set runtimeoutput=%fileshare%\satscan\build.area\satscan\installers\java\jre

set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\satscan\build.area\satscan\installers\inno-setup\satscan.iss

set signtool=%fileshare%\imsadmin\code.sign.cert.ms.auth.verisign\signtool.exe
set certificate=%fileshare%\imsadmin\code.sign.cert.ms.auth.verisign\ims-cross-sign.pfx
set timestamp=http://timestamp.digicert.com/
set password="&4L(JyhyOmwF)$Z"


REM Codesigning a GUI exe file.
%fileshare%\satscan\build.area\satscan\installers\izpack\sign4j\sign4j.exe --verbose %signtool% sign /f %certificate% /p %password% /t %timestamp% /v %satscanexe%

REM Verify the GUI exe file is codesigned correctly.
%signtool% verify /pa /v %satscanexe%

REM Create Java runtime
if exist %runtimeoutput% rmdir %runtimeoutput% /s /q
%javajdk%\bin\jlink.exe --module-path %javajdk%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto --output %runtimeoutput% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign /f %certificate% /p %password% /t %timestamp% /v %satscaninstaller%

REM Verify the installer exe file is codesigned correctly.
%signtool% verify /pa /v %satscaninstaller%

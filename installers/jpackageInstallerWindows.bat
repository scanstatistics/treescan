@echo off

REM Script which creates an installer using Java jpackage.
REM At this time we're using launch4j, creating javatime and Inno Setup for the installer.
REM The installer created by jpacket is good and we might switch to it but there are a few issues to resolve:
REM   1) License isn't presented to user -- not sure what I'm doing wrong.
REM   2) I need to test update process - installing over previous installation.
REM   3) How to properly identify beta releases vs public releases?

set javabin=c:\jdk\jdk-17.0.9+9\bin
set version=2.2
set srcdir=C:\Users\hostovic\projects\treescan.development\treescan
set bundledir=C:\Users\hostovic\projects\treescan.development\jpackage

if exist %bundledir%\TreeScan rmdir %bundledir%\TreeScan /s /q
if exist %bundledir%\bin rmdir %bundledir%\bin /s /q

REM Build TreeScan app bundle
%javabin%\jpackage.exe  --verbose --type app-image --input %srcdir%\java_application\jni_application\dist --main-jar TreeScan.jar --icon %srcdir%\installers\resources\TreeScan.ico --app-version %version% --name TreeScan --dest %bundledir% --java-options "'-Djava.library.path=$APPDIR'"

REM Add additional files to bundle - command-line executables, dlls, sample data, user guide, etc.
xcopy /E /I /Y %srcdir%\installers\examples %bundledir%\TreeScan\installers
xcopy /Y %srcdir%\installers\documents\userguide.pdf %bundledir%\TreeScan
xcopy /Y %srcdir%\installers\documents\eula.html %bundledir%\TreeScan
xcopy /Y %srcdir%\installers\documents\eula\License.txt %bundledir%\TreeScan
xcopy /Y %srcdir%\batch_application\Win32\Release\treescan32.exe %bundledir%\TreeScan
xcopy /Y %srcdir%\batch_application\x64\Release\treescan64.exe %bundledir%\TreeScan
xcopy /Y %srcdir%\shared_library\Release\treescan32.dll %bundledir%\TreeScan\app
xcopy /Y %srcdir%\shared_library\x64\Release\treescan64.dll %bundledir%\TreeScan\app

REM Sign launcher exe but first toggle off read-only flag.
attrib -r %bundledir%\TreeScan\TreeScan.exe
call %srcdir%\signbinary.bat %bundledir%\TreeScan\TreeScan.exe
REM Toggle read-only flag on again.
attrib +r %bundledir%\TreeScan\TreeScan.exe

REM  Create application installer.
%javabin%\jpackage.exe  --verbose --type msi --app-image %bundledir%\TreeScan --app-version %version% --name TreeScan --dest %bundledir%\bin --description "Software for the tree-based scan statistic" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --win-shortcut --win-dir-chooser --win-menu-group --win-upgrade-uuid\"AD0046EA-ADC2-4AD7-B623-E53C00CDAEC9" --license-file  %bundledir%\TreeScan\License.txt

REM Codesigning a installer exe but first toggle off read-only flag.
attrib -r %bundledir%\bin\TreeScan-%version%.msi
call %srcdir%\signbinary.bat %bundledir%\bin\TreeScan-%version%.msi
REM Toggle read-only flag on again.
attrib +r %bundledir%\bin\TreeScan-%version%.msi

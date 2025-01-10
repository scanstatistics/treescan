@echo off

REM Script which code signs TreeScan executable then builds TreeScan installer and finally code signs that installer.

set argCount=0
for %%x in (%*) do (
   set /A argCount+=1
)
if %argCount% NEQ 5 (
  REM See ...\imsadmin\ims.codesign\_Usage.txt
	echo need values of the following parameters, in this order: -kvu -kvi -kvt -kvs -kvc
	exit /b 1
)

REM script definitions
set fileshare=\\oriole-03-int

set treescanversion=2.3
set treescanversionf=2_3
set treescan32exe=%fileshare%\treescan\build\treescan\batch_application\Win32\Release\treescan32.exe
set treescan32dll=%fileshare%\treescan\build\treescan\library\Win32\Release\treescan32.dll
set treescan64exe=%fileshare%\treescan\build\treescan\batch_application\x64\Release\treescan64.exe
set treescan64dll=%fileshare%\treescan\build\treescan\library\x64\Release\treescan64.dll
set treescanguiexe=%fileshare%\treescan\build\treescan\java_application\jni_application\dist\TreeScan.exe
set treescaninstaller=%fileshare%\treescan\installers\v.%treescanversion%.x\install-%treescanversionf%_windows.exe

set javajdkx64=%fileshare%\treescan\build\packages\java\jdk-17.0.13+11_windows_x64
set runtimeoutputx64=%fileshare%\treescan\build\treescan\installers\java\jre_x64
set javajdkx86=%fileshare%\treescan\build\packages\java\jdk-17.0.13+11_windows_x86
set runtimeoutputx86=%fileshare%\treescan\build\treescan\installers\java\jre_x86

set innosetup="C:\Program Files (x86)\Inno Setup 6\iscc.exe"
set innoiss=%fileshare%\treescan\build\treescan\installers\inno-setup\treescan.iss

set signtool=%fileshare%\imsadmin\ims.codesign\AzureSignTool.exe
set timestamp=http://timestamp.digicert.com/
set du=https://www.treescan.org/

REM Codesigning 32-bit command-line exe.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr %timestamp% -v %treescan32exe%

REM Codesigning 32-bit dll.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr %timestamp% -v %treescan32dll%

REM Codesigning 64-bit command-line exe.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr %timestamp% -v %treescan64exe%

REM Codesigning 64-bit dll.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr %timestamp% -v %treescan64dll%

REM Codesigning the GUI exe.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr http://timestamp.digicert.com -v %treescanguiexe%

REM Create Java 64-bit jre - prompt to recreate if already exists, only need to recreate if javajdkx64 was recently updated.
set createjre="n"
if exist %runtimeoutputx64% set /p createjre=Recreate the x64 jre bundle? (Y/n): 
if exist %runtimeoutputx64% if createjre equ "Y" rmdir %runtimeoutputx64% /s /q
if not exist %runtimeoutputx64% %javajdkx64%\bin\jlink.exe --module-path %javajdkx64%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx64% --strip-debug --compress 2 --no-header-files --no-man-pages
 
REM Create Java 32-bit jre - prompt to recreate if already exists, only need to recreate if javajdkx86 was recently updated.
if exist %runtimeoutputx86% set /p createjre=Recreate the x86 jre bundle? (Y/n): 
if exist %runtimeoutputx86% if createjre equ "Y" rmdir %runtimeoutputx86% /s /q
if not exist %runtimeoutputx86% %javajdkx86%\bin\jlink.exe --module-path %javajdkx86%\jmods --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,jdk.crypto.ec,java.net.http,jdk.crypto.cryptoki,jdk.accessibility --output %runtimeoutputx86% --strip-debug --compress 2 --no-header-files --no-man-pages

REM Build InnoSetup installer.
%innosetup% %innoiss%

REM Codesign installer exe file.
%signtool% sign -du "%du%" -kvu "%1" -kvi "%2" -kvt "%3" -kvs "%4" -kvc "%5" -tr %timestamp% -v %treescaninstaller%

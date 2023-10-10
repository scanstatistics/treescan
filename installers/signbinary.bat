
REM usage: signbinary.bat <path-to-exe/dll> <password-in-double-quotes>
P:\imsadmin\code.sign.cert.ms.auth\signtool.exe sign /tr http://timestamp.digicert.com /td sha256 /fd sha256 /f P:\imsadmin\code.sign.cert.ms.auth\ims.pfx /p %2 %1
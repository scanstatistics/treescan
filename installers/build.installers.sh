#!/bin/bash

############ Script Defines #######################################################################
version="2.1"
versionf="2_1"
build="/prj/treescan/build"
installer_version="/prj/treescan/installers/v.${version}.x"
binaries="/prj/treescan/build/binaries/linux"

javajdk=$build/packages/java/jdk-15.0.2-linux_x64
launch4j=$build/packages/launch4j/launch4j-3.12
IzPack=$build/packages/IzPack/IzPack5.1.3

#### Windows ##############################################################################
# Build the Inno Setup installer for Windows. (Note that someday we might replace this process with jpackageInstallerWindows.bat)

# Build Windows TreeScan executable from java jar file ... TreeScan.jar -> TreeScan.exe.
$javajdk/bin/java -jar $launch4j/launch4j.jar $build/treescan/installers/izpack/windows/launch4j_app.xml
chmod g+w $build/treescan/java_application/jni_application/dist/TreeScan.exe

# Prompt user to codesign TreeScan.exe then build installer and codesign that file as well.
echo
echo "Run the Windows batch file ' buildWindowsInstaller.bat' now to sign TreeScan.exe then build and sign the Windows installer. Hit <enter> once done ..."
read dummy

# Build Windows command-line only archive. This is an alternative download option that is command-line only (no GUI/Java).
rm -f $installer_version/treescan.${version}_windows.zip
zip $installer_version/treescan.${version}_windows.zip -j $build/treescan/batch_application/Win32/Release/treescan32.exe
zip $installer_version/treescan.${version}_windows.zip -j $build/treescan/batch_application/x64/Release/treescan64.exe
cd $build/treescan/installers
zip $installer_version/treescan.${version}_windows.zip -j documents/*
zip $installer_version/treescan.${version}_windows.zip examples/*

#######   ############ Linux ################################################################################
# Build the IzPack Java installer for Linux. (Note that someday we might replace this process with jpackageInstallerLinux.sh)

# Build Linux installer. 
$IzPack/bin/compile $build/treescan/installers/izpack/linux/install_linux.xml -b $installer_version -o $installer_version/install-2_0_linux.jar -k standard
chmod a+x $installer_version/install-${versionf}_linux.jar

# Build batch binaries archive for Linux.
rm -f $installer_version/treescan.${version}_linux.tar.gz
cd $build/binaries/linux
tar -cf $installer_version/treescan.${version}_linux.tar treescan*
cd $build/treescan/installers
tar -rf $installer_version/treescan.${version}_linux.tar documents/*
tar -rf $installer_version/treescan.${version}_linux.tar examples/*
gzip -f $installer_version/treescan.${version}_linux.tar

# Build the linux rpm -- still a work in progress.
$build/treescan/installers/jpackageInstallerLinux.sh $version $installer_version

############ Java Application Update Archive ######################################################
# Build update archive files -- relative paths are important; must be the same as installation.
#
# Note: With the move to bundling Java into installation (Windows and Mac currently), this process
#       is not supported currently. The TreeScan application only notifies user of any update and
#       references the website for download.
#       The problem with the current update process is:
#       1) The update application is written in Java. So that application would need redesign.
#       2) I'm no longer certain how to update Mac dmg currently.

# Windows update archive
rm -f $installer_version/update_data_windows.zip

zip $installer_version/update_data_windows.zip -j $build/treescan/batch_application/Win32/Release/treescan32.exe
zip $installer_version/update_data_windows.zip -j $build/treescan/library/Win32/Release/treescan32.dll
zip $installer_version/update_data_windows.zip -j $build/treescan/batch_application/x64/Release/treescan64.exe
zip $installer_version/update_data_windows.zip -j $build/treescan/library/x64/Release/treescan64.dll
zip $installer_version/update_data_windows.zip -j $build/treescan/installers/documents/*
zip $installer_version/update_data_windows.zip -j $build/treescan/java_application/jni_application/dist/TreeScan.jar
zip $installer_version/update_data_windows.zip -j $build/treescan/java_application/jni_application/dist/TreeScan.exe
cd $build/treescan/java_application/jni_application/dist
zip $installer_version/update_data_windows.zip -r lib
cd $build/treescan/installers/java
zip $installer_version/update_data_windows.zip -r jre_x64
zip $installer_version/update_data_windows.zip -r jre_x86

# We can delete the generated Windows Java runtime now.
rm -rf $build/treescan/installers/java/jre
cd $build/treescan/installers
zip $installer_version/update_data_windows.zip -r examples

# Linux update archive
rm -f $installer_version/update_data_linux.zip

zip $installer_version/update_data_linux.zip -j $build/binaries/linux/*
zip $installer_version/update_data_linux.zip -j $build/treescan/installers/documents/*
zip $installer_version/update_data_linux.zip -j $build/treescan/java_application/jni_application/dist/TreeScan.jar
cd $build/treescan/java_application/jni_application/dist
zip $installer_version/update_data_linux.zip -r lib
cd $build/treescan/installers
zip $installer_version/update_data_linux.zip -r examples

### ############ Mac OS X #############################################################################
### Build TreeScan Mac OS X Application DMG

# Prompt user to execute Mac dmg build process.
# https://pwvault.imsweb.com/SecretServer/app/#/secret/2734/general
# Not static IP -- see 'ST-MacPublic' at https://vcenter-vdi.imsweb.com/ui/
# Should be able to ssh directly to IP and execute local file '/Users/treescan/prj/treescan.development/buildMacDMG.sh'.
#  #!/bin/bash
#  #Unlock the keychain
#  security unlock-keychain $HOME/Library/Keychains/login.keychain
#  /Users/treescan/prj/treescan.development/treescan/installers/jpackageInstallerMac.sh <- https://pwvault.imsweb.com/SecretServer/app/#/secret/25934/general ->
# Or login into 'ST-MacPublic' at https://vcenter-vdi.imsweb.com/ui/ and execute 'jpackageInstallerMac.sh'.
echo "*** Execute Mac dmg build script ... buildMacDMG.sh from Mac shell or login into VM and execute jpackageInstallerMac.sh."

#!/bin/sh

############ Script Defines #######################################################################
build="/prj/treescan/build"
installer_version="/prj/treescan/installers/v.1.1.x"

launch4j=$build/packages/launch4j/launch4j-3.0.1
IzPack=$build/packages/IzPack/IzPack4.3.5

############ Windows ##############################################################################
# Build Windows TreeScan executable from java jar file ... TreeScan.jar -> TreeScan.exe.
$launch4j/launch4j $build/treescan/installers/izpack/windows/launch4j_app.xml

# prompt user to sign the exe file created by launch4j
echo
echo "Run the Windows batch file 'signGuiApp.bat' now to sign TreeScan.exe. Hit <enter> once done ..."
read dummy

# Build the IzPack Java installer for Windows.
$IzPack/bin/compile $build/treescan/installers/izpack/windows/install_windows.xml -b $installer_version -o $installer_version/install-1_1_windows.jar -k standard

# Build Windows installer executable from Java jar file. This is needed for:
#  - UAC (User Account Control)
#  - we wanted a message to user when Java not installed
$launch4j/launch4j $build/treescan/installers/izpack/windows/launch4j_install.xml
rm $installer_version/install-1_1_windows.jar

# prompt user to sign the exe file created by launch4j
echo
echo "Run the Windows batch file 'signWindowsInstaller.bat' now to sign install-1_1_windows.exe. Hit <enter> once done ..."
read dummy

# Build Windows command-line only archive
rm -f $installer_version/treescan.1.1_windows.zip
zip $installer_version/treescan.1.1_windows.zip -j $build/treescan/batch_application/Win32/Release/treescan32.exe
zip $installer_version/treescan.1.1_windows.zip -j $build/treescan/batch_application/x64/Release/treescan64.exe
cd $build/treescan/installers
zip $installer_version/treescan.1.1_windows.zip documents/*
zip $installer_version/treescan.1.1_windows.zip examples/*

############ Linux ################################################################################
# Build the IzPack Java installer for Linux.
$IzPack/bin/compile $build/treescan/installers/izpack/linux/install_linux.xml -b $installer_version -o $installer_version/install-1_1_linux.jar -k standard
chmod a+x $installer_version/install-1_1_linux.jar

# Build batch binaries archive for Linux.
rm -f $installer_version/treescan.1.1_linux.tar.bz2
cd $build/binaries/linux
tar -cf $installer_version/treescan.1.1_linux.tar treescan*
cd $build/treescan/installers
tar -rf $installer_version/treescan.1.1_linux.tar documents/*
tar -rf $installer_version/treescan.1.1_linux.tar examples/*
bzip2 -f $installer_version/treescan.1.1_linux.tar

############ Mac OS X #############################################################################
# Build TreeScan Mac OS X Application Bundle Directory
rm -rf $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app
python $build/treescan/installers/izpack/mac/treescan2app/treescan2app.py $build/treescan/java_application/jni_application/dist/TreeScan.jar $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app
# copy jni libraries into app directory
cp $build/binaries/mac/libtreescan.jnilib $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app/Contents/Resources/Java/libtreescan.jnilib
# copy additional Java libraries into app directory
cp $build/treescan/java_application/jni_application/dist/lib/* $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app/Contents/Resources/Java/lib/

# Build the IzPack Java installer for Mac OS X.
$IzPack/bin/compile $build/treescan/installers/izpack/mac/install_mac.xml -b $installer_version -o $installer_version/install-1_1_mac.jar -k standard

# Build Mac OS X Application Bundle from IzPack Java Installer
rm -rf $installer_version/install-1_1_mac.zip
python $build/treescan/installers/izpack/mac/izpack2app/izpack2app.py $installer_version/install-1_1_mac.jar $build/treescan/installers/izpack/mac/Install.app
cd $build/treescan/installers/izpack/mac
zip $installer_version/install-1_1_mac.zip -r ./Install.app/*
rm $installer_version/install-1_1_mac.jar
rm -rf $build/treescan/installers/izpack/mac/Install.app
chmod a+x $installer_version/install-1_1_mac.zip

# Build batch binaries archive for Mac OS X.
rm -f $installer_version/treescan.1.1_mac.tar.bz2
cd $build/binaries/mac
tar -cf $installer_version/treescan.1.1_mac.tar treescan
cd $build/treescan/installers
tar -rf $installer_version/treescan.1.1_mac.tar documents/*
tar -rf $installer_version/treescan.1.1_mac.tar examples/*
bzip2 -f $installer_version/treescan.1.1_mac.tar

#rm -rf $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app

############ Java Application Update Archive ######################################################

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

# Mac update archive
rm -f $installer_version/update_data_mac.zip

cd $build/treescan/installers/izpack/mac/treescan2app
zip $installer_version/update_data_mac.zip -r TreeScan.app
zip $installer_version/update_data_mac.zip -j $build/binaries/mac/treescan
zip $installer_version/update_data_mac.zip -j $build/treescan/installers/documents/*
cd $build/treescan/installers
zip $installer_version/update_data_mac.zip -r examples

#!/bin/sh

############ Script Defines #######################################################################
build="/prj/treescan/build"
installer_version="/prj/treescan/installers/v.1.5.x"

launch4j=$build/packages/launch4j/launch4j-3.12
IzPack=$build/packages/IzPack/IzPack5.1.3

############ Windows ##############################################################################
# Build Windows TreeScan executable from java jar file ... TreeScan.jar -> TreeScan.exe.
$launch4j/launch4j $build/treescan/installers/izpack/windows/launch4j_app.xml

# prompt user to build gui exe and codesign it, then build installer and codesign that file as well.
echo
echo "Run the Windows batch file ' buildWindowsInstaller.bat' now to build and sign TreeScan.exe, then build/sign installer. Hit <enter> once done ..."
read dummy

# Build Windows command-line only archive
rm -f $installer_version/treescan.1.5_windows.zip
zip $installer_version/treescan.1.5_windows.zip -j $build/treescan/batch_application/Win32/Release/treescan32.exe
zip $installer_version/treescan.1.5_windows.zip -j $build/treescan/batch_application/x64/Release/treescan64.exe
cd $build/treescan/installers
zip $installer_version/treescan.1.5_windows.zip documents/*
zip $installer_version/treescan.1.5_windows.zip examples/*

############ Linux ################################################################################
# Build the IzPack Java installer for Linux.
$IzPack/bin/compile $build/treescan/installers/izpack/linux/install_linux.xml -b $installer_version -o $installer_version/install-1_5_linux.jar -k standard
chmod a+x $installer_version/install-1_5_linux.jar

# Build batch binaries archive for Linux.
rm -f $installer_version/treescan.1.5_linux.tar.bz2
cd $build/binaries/linux
tar -cf $installer_version/treescan.1.5_linux.tar treescan*
cd $build/treescan/installers
tar -rf $installer_version/treescan.1.5_linux.tar documents/*
tar -rf $installer_version/treescan.1.5_linux.tar examples/*
bzip2 -f $installer_version/treescan.1.5_linux.tar

############ Mac OS X #############################################################################
# Build TreeScan Mac OS X Application Bundle Directory
rm -rf $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app
python $build/treescan/installers/izpack/mac/treescan2app/treescan2app.py $build/treescan/java_application/jni_application/dist/TreeScan.jar $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app
# copy jni libraries into app directory
cp $build/binaries/mac/libtreescan.jnilib $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app/Contents/Java/libtreescan.jnilib
# copy additional Java libraries into app directory
cp $build/treescan/java_application/jni_application/dist/lib/* $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app/Contents/Java/lib/

# prompt user to sign the SaTScan.app on Mac with Developer ID certificated installed (Squish https://www.squishlist.com/ims/satscan/66329/)
echo
echo "1) Run the script .../treescan/scripts/mac/codesign_remote_appbundle.sh on TreeScan.app from Mac with Developer ID certificated installed."
echo "2) Hit <enter> once done ..."
read dummy

# Build the IzPack Java installer for Mac OS X.
$IzPack/bin/compile $build/treescan/installers/izpack/mac/install_mac.xml -b $installer_version -o $installer_version/install-1_5_mac.jar -k standard

# Build Mac OS X Application Bundle from IzPack Java Installer
rm -rf $installer_version/install-1_5_mac.zip
rm -rf $build/treescan/installers/izpack/mac/Install.app
python $build/treescan/installers/izpack/mac/izpack2app/izpack2app.py $installer_version/install-1_5_mac.jar $build/treescan/installers/izpack/mac/Install.app

# prompt user to sign the Install.app on Mac with Developer ID certificated installed (Squish https://www.squishlist.com/ims/satscan/66329/)
echo
echo "1) Run the script .../treescan/scripts/mac/codesign_remote_appbundle.sh on Install.app from Mac with Developer ID certificated installed"
echo "2) Hit <enter> once done ..."
read dummy

cd $build/treescan/installers/izpack/mac
zip $installer_version/install-1_5_mac.zip -r ./Install.app/*
rm $installer_version/install-1_5_mac.jar
rm -rf $build/treescan/installers/izpack/mac/Install.app
chmod a+x $installer_version/install-1_5_mac.zip

# Build batch binaries archive for Mac OS X.
rm -f $installer_version/treescan.1.5_mac.tar.bz2
cd $build/binaries/mac
tar -cf $installer_version/treescan.1.5_mac.tar treescan
cd $build/treescan/installers
tar -rf $installer_version/treescan.1.5_mac.tar documents/*
tar -rf $installer_version/treescan.1.5_mac.tar examples/*
bzip2 -f $installer_version/treescan.1.5_mac.tar

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
cd $build/treescan/installers/java
zip $installer_version/update_data_windows.zip -r win32-jre
zip $installer_version/update_data_windows.zip -r win64-jre
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

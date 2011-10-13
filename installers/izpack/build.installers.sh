#!/bin/sh

############ Script Defines #######################################################################
build="/prj/treescan/build"
installer_version="/prj/treescan/installers/v.1.1.x"

launch4j=$build/packages/launch4j/launch4j-3.0.1
IzPack=$build/packages/IzPack/IzPack4.3.4

############ Windows ##############################################################################
# Build Windows TreeScan executable from java jar file ... TreeScan.jar -> TreeScan.exe.
$launch4j/launch4j $build/treescan/installers/izpack/windows/launch4j_app.xml

# Build the IzPack Java installer for Windows.
$IzPack/bin/compile $build/treescan/installers/izpack/windows/install_windows.xml -b $installer_version -o $installer_version/install-1_1_windows.jar -k standard

# Build Windows installer executable from Java jar file. This is needed for:
#  - UAC (User Account Control)
#  - we wanted a message to user when Java not installed
$launch4j/launch4j $build/treescan/installers/izpack/windows/launch4j_install.xml
rm $installer_version/install-1_1_windows.jar

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
rm -rf $installer_version/install-1_1_mac.app
python $build/treescan/installers/izpack/mac/izpack2app/izpack2app.py $installer_version/install-1_1_mac.jar $build/treescan/installers/izpack/mac/Install.app
cd $build/treescan/installers/izpack/mac
zip $installer_version/install-1_1_mac.app -r ./Install.app/*
rm $installer_version/install-1_1_mac.jar
rm -rf $build/treescan/installers/izpack/mac/Install.app
chmod a+x $installer_version/install-1_1_mac.app

# Build batch binaries archive for Mac OS X.
rm -f $installer_version/treescan.1.1_mac.tar.bz2
cd $build/binaries/mac
tar -cf $installer_version/treescan.1.1_mac.tar treescan
cd $build/treescan/installers
tar -rf $installer_version/treescan.1.1_mac.tar documents/*
tar -rf $installer_version/treescan.1.1_mac.tar examples/*
bzip2 -f $installer_version/treescan.1.1_mac.tar

rm -rf $build/treescan/installers/izpack/mac/treescan2app/TreeScan.app

#######  ############ Java Application Update Archive ######################################################
#######  cd $installer_path
#######
#######  # Build update archive files -- relative paths are important; must be the same as installation
#######
#######  # Combined Windows/Linux update archive
#######  #  -- Starting with the release featuring the Mac, this archive was not needed;
#######  #     so only add Windows and Linux relevant files.
#######  rm -f $installer_path/update_data_combined.zip
#######
#######  zip $installer_path/update_data_combined.zip -j ../windows/*
#######  zip $installer_path/update_data_combined.zip -j ../linux/*
#######  zip $installer_path/update_data_combined.zip -j ../users.guide/*.pdf
#######  zip $installer_path/update_data_combined.zip -j ../java/*
#######  cd $installer_version/java
#######  zip $installer_path/update_data_combined.zip lib/*
#######  cd $installer_version
#######  zip $installer_path/update_data_combined.zip sample_data/*
#######
#######  # Windows update archive
#######  cd $installer_path
#######  rm -f $installer_path/update_data_windows.zip
#######
#######  zip $installer_path/update_data_windows.zip -j ../windows/*
#######  zip $installer_path/update_data_windows.zip -j ../users.guide/*.pdf
#######  zip $installer_path/update_data_windows.zip -j ../java/*
#######  cd $installer_version/java
#######  zip $installer_path/update_data_windows.zip lib/*
#######  cd $installer_version
#######  zip $installer_path/update_data_windows.zip sample_data/*
#######
#######  # Linux update archive
#######  cd $installer_path
#######  rm -f $installer_path/update_data_linux.zip
#######
#######  zip $installer_path/update_data_linux.zip -j ../linux/*
#######  zip $installer_path/update_data_linux.zip -j ../users.guide/*.pdf
#######  zip $installer_path/update_data_linux.zip -j ../java/*
#######  cd $installer_version/java
#######  zip $installer_path/update_data_linux.zip lib/*
#######  cd $installer_version
#######  zip $installer_path/update_data_linux.zip sample_data/*
#######
#######  # Mac update archive
#######  cd $installer_path
#######  rm -f $installer_path/update_data_mac.zip
#######  cd $installer_path/mac/treescan2app
#######  zip $installer_path/update_data_mac.zip -r TreeScan.app/*
#######  cd $installer_path
#######  zip $installer_path/update_data_mac.zip -j ../mac/treescan
#######  zip $installer_path/update_data_mac.zip -j ../users.guide/*.pdf
#######  cd $installer_version
#######  zip $installer_path/update_data_mac.zip sample_data/*
#######

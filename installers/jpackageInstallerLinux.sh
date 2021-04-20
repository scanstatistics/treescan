#!/bin/bash

# Script which creates an installer using Java jpackage.
# Unfortuntely we can use this at the moment. All the Linux servers at IMS are CentOS
# and it appears that you need to be on RedHat or Ubuntu. Right now this script works
# until it attempts to create deb or rpm file. Perhaps I just need other packages installed
# into my local java or this really needs to be done on RedHat or Ubuntu.

javajdk="/prj/satscan/installers/install.applications/java/jdk-15.0.2-linux_x64"
version="10.0"
srcdir="/prj/satscan/build.area/satscan"
bundledir="/prj/satscan/build.area/jpackage"
binaries="/prj/satscan/build.area/binaries/linux"

rm -rf $bundledir/SaTScan
rm -rf $bundledir/bin

# Build SaTScan app bundle
$javajdk/bin/jpackage --verbose --type app-image --input $srcdir/java_application/jni_application/dist --main-jar SaTScan.jar --icon $srcdir/installers/resources/SaTScan.png --app-version $version --name SaTScan --dest $bundledir --java-options "-Djava.library.path=\$APPDIR"

# Add additional files to bundle - command-line executables, so, sample data, user guide, etc.
cp -rf $srcdir/installers/sample_data $bundledir/SaTScan/sample_data
cp -f $srcdir/installers/documents/SaTScan_Users_Guide.pdf $bundledir/SaTScan
cp -f $srcdir/installers/documents/eula.html $bundledir/SaTScan
cp -f $srcdir/installers/documents/eula/License.txt $bundledir/SaTScan
cp -f $binaries/satscan_stdc++6_x86_64_64bit $bundledir/satscan
cp -f $binaries/libsatscan_stdc++6_x86_64_64bit.so $bundledir/SaTScan/lib/app/libsatscan.so

#  Create application installer.
$javajdk/bin/jpackage --verbose --type rpm --app-image $bundledir/SaTScan --app-version $version --name SaTScan --dest $bundledir/bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --linux-shortcut --linux-menu-group


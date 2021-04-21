#!/bin/bash

# Script which creates an installer using Java jpackage.
# Unfortuntely we can use this at the moment. All the Linux servers at IMS are CentOS
# and it appears that you need to be on RedHat or Ubuntu. Right now this script works
# until it attempts to create deb or rpm file. Perhaps I just need other packages installed
# into my local java or this really needs to be done on RedHat or Ubuntu.

javajdk="/prj/treescan/build/packages/java/jdk-15.0.2-linux_x64"
version="10.0"
srcdir="/prj/treescan/build/treescan"
bundledir="/prj/treescan/build/jpackage"
binaries="/prj/treescan/build/binaries/linux"

rm -rf $bundledir/TreeScan
rm -rf $bundledir/bin

# Build TreeScan app bundle
$javajdk/bin/jpackage --verbose --type app-image --input $srcdir/java_application/jni_application/dist --main-jar TreeScan.jar --icon $srcdir/installers/resources/TreeScan.png --app-version $version --name TreeScan --dest $bundledir --java-options "-Djava.library.path=\$APPDIR"

# Add additional files to bundle - command-line executables, so, sample data, user guide, etc.
cp -rf $srcdir/installers/examples $bundledir/TreeScan/examples
cp -f $srcdir/installers/documents/userguide.pdf $bundledir/TreeScan
cp -f $srcdir/installers/documents/eula.html $bundledir/TreeScan
cp -f $srcdir/installers/documents/eula/License.txt $bundledir/TreeScan
cp -f $binaries/treescan_64 $bundledir/treescan
cp -f $binaries/libtreescan_64 $bundledir/TreeScan/lib/app/libtreefsscan.so

#  Create application installer.
$javajdk/bin/jpackage --verbose --type rpm --app-image $bundledir/TreeScan --app-version $version --name TreeScan --dest $bundledir/bin --description "Software for the spatial, temporal, and space-time scan statistics" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --linux-shortcut --linux-menu-group


#!/bin/bash

# Script which creates an installer using Java jpackage.
# Still a work in progress. We can build rpm file on gen-btp-01
# and that prm is convertable to deb file on Ubuntu. Still:
# The default template.spec file might not be 100% what we want.
# I tested this on Ubuntu and it installed but:
# - permissions are only for root
# - icon still wrong
# - probably other things when looking closer

javajdk="/prj/treescan/build/packages/java/jdk-17.0.2+8-linux_x86"
version=$1
srcdir="/prj/treescan/build/treescan"
bundleinputdir="/prj/treescan/build/jpackage/treescanbundlesrc"
bundledir="/prj/treescan/build/jpackage"
binaries="/prj/treescan/build/binaries/linux"

rm -rf $bundleinputdir
rm -rf $bundledir/TreeScan
rm -rf $bundledir/bin

# copy all input files to bundle input dir
mkdir -p $bundleinputdir
cp $srcdir/java_application/jni_application/dist/TreeScan.jar $bundleinputdir
cp -rf $srcdir/java_application/jni_application/dist/lib $bundleinputdir
cp -rf $srcdir/installers/examples $bundleinputdir
cp -f $srcdir/installers/documents/userguide.pdf $bundleinputdir
cp -f $srcdir/installers/documents/eula.html $bundleinputdir
cp -f $srcdir/installers/documents/eula/License.txt $bundleinputdir
cp -f $binaries/treescan32 $bundleinputdir
cp -f $binaries/libtreescan32.so $bundleinputdir
cp -f $binaries/treescan64 $bundleinputdir
cp -f $binaries/libtreescan64.so $bundleinputdir

# Build TreeScan app bundle
$javajdk/bin/jpackage --verbose --type app-image --input $bundleinputdir \
            --main-jar TreeScan.jar --icon $srcdir/installers/resources/TreeScan.png \
            --app-version $version --name TreeScan --dest $bundledir \
            --java-options "-Djava.library.path=\$APPDIR"

#  Create application rpm
$javajdk/bin/jpackage --verbose --type rpm --app-image $bundledir/TreeScan --app-version $version \
            --name TreeScan --resource-dir $srcdir/installers/resources --dest $2 \
            --description "Software for the spatial, temporal, and space-time scan statistics" \
            --vendor "Martin Kulldorff together with Information Management Services Inc." \
            --linux-shortcut --linux-rpm-license-type "see TreeScan License Agreement @ https://www.treescan.org/techdoc.html" \
            --linux-app-release "0" --copyright "Copyright 2021, All rights reserved"

#  Create application deb
#$javajdk/bin/jpackage --verbose --type deb --app-image $bundledir/TreeScan --app-version $version \
#           --name TreeScan --resource-dir $srcdir/installers/resources --dest $2 \
#           --description "Software for the spatial, temporal, and space-time scan statistics" \
#           --vendor "Martin Kulldorff together with Information Management Services Inc." \
#           --linux-shortcut --linux-rpm-license-type "see TreeScan License Agreement @ https://www.treescan.org/techdoc.html" \
#           --linux-app-release "0" --copyright "Copyright 2021, All rights reserved" \
#           --linux-deb-maintainer techsupport@treescan.org
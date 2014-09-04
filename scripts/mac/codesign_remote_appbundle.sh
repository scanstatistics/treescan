#!/bin/sh

REQUIRED_ARGS=3
if [ $# -lt "$REQUIRED_ARGS" ]
then
echo "Usage: `basename $0` <app nfs location> <app bundlename> <project directory>"
echo "   example: `basename $0` /Users/treescan/prj/treescan.development/treescan.home/build/treecsan/installers/izpack/mac/treescan2app/ TreeScan.app /Users/treescan/prj/treescan.development/"
echo "   example: `basename $0` /Users/treescan/prj/treescan.development/treescan.home/build/treecsan/installers/izpack/mac/ Install.app /Users/treescan/prj/treescan.development/"
exit 1
fi

# copy application bundle from remote share
cp -r $1$2 $3
# codesign the local copy of application bundle
$3/treescan/scripts/mac/codesign.sh $3$2
# remove the application bundle on remote share
rm -rf $1$2
# copy codesigned application bundle back to remote share
cp -r $3$2 $1
# remove local copy of application bundle
rm -rf $3$2

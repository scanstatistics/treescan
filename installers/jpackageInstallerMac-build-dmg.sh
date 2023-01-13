#!/bin/bash

# https://pwvault.imsweb.com/SecretServer/app/#/secret/26122/general
if [[ $# -gt 0 ]]; then
  PASSWORD=$1
else
  read -p "Apple developer password (TreeScan Mac App Specific): " PASSWORD
fi

APPVERSION="2.1"
SRCDIR="/Users/treescan/prj/treescan.development/treescan"
INSTALLER_DIR="/prj/treescan/installers/v.${APPVERSION}.x"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
BUNDLEDIR="/Users/treescan/prj/treescan.development/jpackaged"
BINARIES="/Users/treescan/prj/treescan.development/binaries/mac"
#JAVAJDK="/Users/treescan/prj/java/jdk-16.0.2+7/Contents/Home" # AdoptJDK
JAVAJDK="/Users/treescan/prj/java/jdk-17.0.5+8/Contents/Home" # AdoptJDK
ENTITLEMENTS="${SRCDIR}/installers/macosentitlements.plist"
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"

rm -rf $BUNDLEDIR/bin

# Create dmg with notarized application - but codesign separately.
$JAVAJDK/bin/jpackage --type pkg  --verbose --app-image $BUNDLEDIR/TreeScan.app --app-version $APPVERSION \
                      --name TreeScan --dest $BUNDLEDIR/bin --description "Software for the tree-based scan statistic" \
					  --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved" \
					  --resource-dir $BUNDLEDIR/dmgresources --type dmg
Echo How did the dmg build go?
read APPLES_TEST

# codesign and check TreeScan.dmg
codesign --entitlements  ${ENTITLEMENTS} --options runtime -vvvv --deep --timestamp -s "${SIGN_KEY}" $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

# Notorize TreeScan.dmg
REQUEST_UUID_DMG=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.treescan" --username "meagherk@imsweb.com" --password "${PASSWORD}" --asc-provider "VF82MCMA83" --file $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg | grep RequestUUID | awk '{print $3}')

# Poll for verification completion.
while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
    echo "Verification in progress..."
    sleep 60
done

echo Results of notarization
$XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}"

Echo Any problems notarizing dmg file?
read APPLES_TEST

# staple application -- assumes notarization succeeds.
$XCRUN $STAPLER staple $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

# test notarized
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

# Build batch binaries archive for Mac OS X.
rm -f $BUNDLEDIR/treescan.${APPVERSION}_mac.tar.bz2
cd $BINARIES
tar -cf $BUNDLEDIR/treescan.${APPVERSION}_mac.tar treescan
cd $SRCDIR/installers
tar -rf $BUNDLEDIR/treescan.${APPVERSION}_mac.tar documents/*
tar -rf $BUNDLEDIR/treescan.${APPVERSION}_mac.tar examples/*
bzip2 -f $BUNDLEDIR/treescan.${APPVERSION}_mac.tar

# push over to fileshare installers directory
echo Copying dmg to fileshare
mv $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg $BUNDLEDIR/bin/TreeScan_${APPVERSION}_mac.dmg
mv $BUNDLEDIR/TreeScan.zip $BUNDLEDIR/TreeScan_${APPVERSION}_mac.zip
scp -r $BUNDLEDIR/bin/TreeScan_${APPVERSION}_mac.dmg $BUNDLEDIR/TreeScan_${APPVERSION}_mac.zip $BUNDLEDIR/treescan.${APPVERSION}_mac.tar.bz2 treescan@gen-btp-01.imsweb.com:${INSTALLER_DIR}


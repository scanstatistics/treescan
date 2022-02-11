#!/bin/bash

# https://pwvault.imsweb.com/SecretServer/app/#/secret/26122/general
if [[ $# -gt 0 ]]; then
  PASSWORD=$1
else
  read -p "Apple developer password (TreeScan Mac App Specific): " PASSWORD
fi

APPVERSION="2.0"
SRCDIR="/Users/treescan/prj/treescan.development/treescan"
INSTALLER_DIR="/prj/treescan/installers/v.${APPVERSION}.x"
SIGN_KEY="Developer ID Application: Information Management Services, Inc. (VF82MCMA83)"
BUNDLEDIR="/Users/treescan/prj/treescan.development/jpackaged"
BINARIES="/Users/treescan/prj/treescan.development/binaries/mac"
JAVAJDK="/Users/treescan/prj/java/jdk-17.0.2+8/Contents/Home" # AdoptJDK
ENTITLEMENTS="${SRCDIR}/installers/macosentitlements.plist"
XCRUN="/usr/bin/xcrun"
ALTOOL="/Applications/Xcode.app/Contents/Developer/usr/bin/altool"
STAPLER="/Applications/Xcode.app/Contents/Developer/usr/bin/stapler"

# Clean up output directory
rm -rf $BUNDLEDIR
mkdir $BUNDLEDIR

# Create collection of files that will be the application.
mkdir $BUNDLEDIR/imagesrc
# Copy TreeScan.jar from fileshare -- maybe we can build this locally at some point.
echo Copying TreeScan.jar from fileshare
scp -r treescan@gen-btp-01.imsweb.com:/prj/treescan/build/treescan/java_application/jni_application/dist/TreeScan.jar $BUNDLEDIR/imagesrc
#cp -rf $SRCDIR/java_application/jni_application/dist/TreeScan.jar $BUNDLEDIR/imagesrc
cp -rf $SRCDIR/java_application/jni_application/libs $BUNDLEDIR/imagesrc
cp -rf $SRCDIR/installers/examples $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/userguide.pdf $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/eula.html $BUNDLEDIR/imagesrc
cp -f $SRCDIR/installers/documents/eula/License.txt $BUNDLEDIR/imagesrc
cp -f $BINARIES/treescan $BUNDLEDIR/imagesrc
cp -f $BINARIES/libtreescan.jnilib $BUNDLEDIR/imagesrc

# Create overrride resources for DMG - https://docs.oracle.com/en/java/javase/15/jpackage/override-jpackage-resources.html#GUID-1B718F8B-B68D-4D46-B1DB-003D7729AAB6
mkdir $BUNDLEDIR/dmgresources
cp -f $SRCDIR/installers/resources/TreeScan.icns $BUNDLEDIR/dmgresources
cp -f $SRCDIR/installers/resources/TreeScan-volume.icns $BUNDLEDIR/dmgresources

# Ensure that our binaries/main jar are codesigned and runtime hardened. 
# This is just a safe guard to ensure they are runtime hardened since jpackage skips already signed.
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/treescan
codesign -vvv --strict $BUNDLEDIR/imagesrc/treescan
codesign --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/libtreescan.jnilib
codesign -vvv --strict $BUNDLEDIR/imagesrc/libtreescan.jnilib
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -f -v -s "${SIGN_KEY}" $BUNDLEDIR/imagesrc/TreeScan.jar
codesign -vvv --strict $BUNDLEDIR/imagesrc/TreeScan.jar

# Technically we should be able to just call the following to create the app, codesign and build dmg.
# Unfortunately the notarization fails - complaining about signatures on the launcher and dylib being invalid.
# After a lot of trial and error, I decided to try just uploading the app for notarization -- motivated by the following link:
# https://blog.frostwire.com/2019/08/27/apple-notarization-the-signature-of-the-binary-is-invalid-one-other-reason-not-explained-in-apple-developer-documentation/
# Notarizing the app alone succeeds. So my best guess, at this point, is that jpackage isn't creating the app within the dmg without mistakenly invalidating
# launcher/dylib code signatures. I tried many variations of building and code signing in pieces (similar to https://github.com/rokstrnisa/unattach/blob/master/package.sh)
# but always the notarization failed on the launcher/dylib code signatures.
#$JAVAJDK/bin/jpackage --verbose --type dmg --input $BUNDLEDIR/imagesrc --main-jar TreeScan.jar --icon $SRCDIR/installers/izpack/mac/treescan2app/Mac-App-Template/Contents/Resources/TreeScan.icns --app-version ${APPVERSION} --name TreeScan --dest $BUNDLEDIR/bin --java-options "-Djava.library.path=\$APPDIR" --mac-sign --mac-package-signing-prefix VF82MCMA83 --mac-signing-key-user-name "Information Management Services, Inc." --description "Software for the tree-based scan statistic" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved"  --resource-dir $BUNDLEDIR/dmgresources

# Create TreeScan app directory
$JAVAJDK/bin/jpackage --verbose --type app-image --input $BUNDLEDIR/imagesrc --main-jar TreeScan.jar \
                      --icon $SRCDIR/installers/izpack/mac/treescan2app/Mac-App-Template/Contents/Resources/TreeScan.icns --app-version ${APPVERSION} \
                      --name TreeScan --dest $BUNDLEDIR --java-options "-Djava.library.path=\$APPDIR" \
                      --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto,jdk.crypto.cryptoki,jdk.accessibility

## # Sign APP's runtime and itself.
codesign --sign "${SIGN_KEY}" --options runtime  --timestamp --entitlements ${ENTITLEMENTS} -vvv --force $BUNDLEDIR/TreeScan.app/Contents/runtime/Contents/MacOS/libjli.dylib
codesign -vvv --strict $BUNDLEDIR/TreeScan.app/Contents/runtime/Contents/MacOS/libjli.dylib
codesign --sign "${SIGN_KEY}" --options runtime  --timestamp --entitlements ${ENTITLEMENTS} -vvv --force $BUNDLEDIR/TreeScan.app/Contents/MacOS/TreeScan
codesign -vvv --strict $BUNDLEDIR/TreeScan.app/Contents/MacOS/TreeScan
codesign --sign "${SIGN_KEY}" --options runtime --timestamp --entitlements ${ENTITLEMENTS} -vvvv --force $BUNDLEDIR/TreeScan.app
codesign -dvvv  $BUNDLEDIR/TreeScan.app
spctl -vvv --assess --type exec $BUNDLEDIR/TreeScan.app
Echo Any problems codesigning app?
read APPLES_TEST

# Create dmg with notarized application - but codesign separately.
$JAVAJDK/bin/jpackage --type pkg --verbose --app-image $BUNDLEDIR/TreeScan.app --app-version $APPVERSION --verbose --type dmg --name TreeScan --dest $BUNDLEDIR/bin \
                      --description "Software for the tree-based scan statistic" --vendor "Information Management Services, Inc." \
                      --copyright "Copyright 2021, All rights reserved" --resource-dir $BUNDLEDIR/dmgresources                     
Echo Any problems creating dmg file?
read APPLES_TEST

# codesign and check TreeScan.dmg
# xattr -cr $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

# codesign -dvvv $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg
codesign --entitlements  ${ENTITLEMENTS} --options runtime --timestamp -vvvv --deep -s "${SIGN_KEY}" $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg
codesign -dvvv $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg
Echo Any problems codesigning dmg file?
read APPLES_TEST

# Create pkg
# $JAVAJDK/bin/jpackage  --type pkg --verbose --mac-package-identifier TreeScan --app-image $BUNDLEDIR/TreeScan.app --app-version $APPVERSION --name TreeScan --verbose --type pkg --dest $BUNDLEDIR/bin --description "Software for the tree-based scan statistic" --vendor "Information Management Services, Inc." --copyright "Copyright 2021, All rights reserved" --resource-dir $BUNDLEDIR/dmgresources
# Echo Any problems creating pkg file?
# read APPLES_TEST
## productsign --sign "${INSTALLER_SIGN_KEY}" $BUNDLEDIR/bin/TreeScan-${APPVERSION}.pkg $BUNDLEDIR/bin/TreeScan-${APPVERSION}-signed.pkg
# Echo Any problems codesigning pkg file?
# read APPLES_TEST

# Notorize TreeScan.dmg
REQUEST_UUID_DMG=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.treescan.dmg" --username "meagherk@imsweb.com" --password "${PASSWORD}" --asc-provider "VF82MCMA83" --file $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg | grep RequestUUID | awk '{print $3}')

# Poll for verification completion.
while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
    echo "Verification in progress..."
    sleep 30
done

echo Results of notarization
$XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_DMG" -u "meagherk@imsweb.com" -p "${PASSWORD}"

Echo Any problems notarizing dmg file?
read APPLES_TEST

# staple application -- assumes notarization succeeds.
$XCRUN $STAPLER staple $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

# test notarized
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dmg

## TODD: First need to create installer signing identity.
## # Notorize TreeScan.pkg
## REQUEST_UUID_PKG=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.treescan" --username "meagherk@imsweb.com" --password "${PASSWORD}" --asc-provider "VF82MCMA83" --file $BUNDLEDIR/bin/TreeScan-${APPVERSION}.pkg | grep RequestUUID | awk '{print $3}')
## 
## # Poll for verification completion.
## while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_PKG" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
##     echo "Verification in progress..."
##     sleep 30
## done
## 
## Echo Any problems notarizing pkg file?
## read APPLES_TEST
## 
## echo Results of notarization
## $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_PKG" -u "meagherk@imsweb.com" -p "${PASSWORD}"
## 
## # staple application -- assumes notarization succeeds.
## $XCRUN $STAPLER staple $BUNDLEDIR/bin/TreeScan-${APPVERSION}.pkg
## 
## # test notarized
## codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/bin/TreeScan-${APPVERSION}.dpkg
## 

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


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
#JAVAJDK="/Users/treescan/prj/java/jdk-16.0.2+7/Contents/Home" # AdoptJDK
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

# Create TreeScan app directory
$JAVAJDK/bin/jpackage --verbose --type app-image --input $BUNDLEDIR/imagesrc --main-jar TreeScan.jar \
                      --icon $SRCDIR/installers/izpack/mac/treescan2app/Mac-App-Template/Contents/Resources/TreeScan.icns \
					  --app-version ${APPVERSION} --name TreeScan --dest $BUNDLEDIR --java-options "-Djava.library.path=\$APPDIR" \
					  --add-modules java.base,java.datatransfer,java.desktop,java.logging,java.prefs,java.xml,java.xml.crypto,jdk.crypto.cryptoki,jdk.accessibility

# Sign APP's runtime and itself.
codesign -s "${SIGN_KEY}" --options runtime --entitlements ${ENTITLEMENTS} --timestamp -vvv -f $BUNDLEDIR/TreeScan.app/Contents/runtime/Contents/MacOS/libjli.dylib
codesign -vvv --strict $BUNDLEDIR/TreeScan.app/Contents/runtime/Contents/MacOS/libjli.dylib
codesign -s "${SIGN_KEY}" --options runtime --entitlements ${ENTITLEMENTS} --timestamp -vvv -f $BUNDLEDIR/TreeScan.app/Contents/MacOS/TreeScan
codesign -vvv --strict $BUNDLEDIR/TreeScan.app/Contents/MacOS/TreeScan
codesign -s "${SIGN_KEY}" --options runtime --entitlements ${ENTITLEMENTS} --timestamp -vvv -f $BUNDLEDIR/TreeScan.app
codesign -vvv --strict $BUNDLEDIR/TreeScan.app
spctl -vvv --assess --type exec $BUNDLEDIR/TreeScan.app

# Create zip file from TreeScan.app notarize application alone.
ditto -c -k --sequesterRsrc --keepParent $BUNDLEDIR/TreeScan.app $BUNDLEDIR/TreeScan.zip

# Notorize TreeScan.dmg
REQUEST_UUID_APP=$($XCRUN $ALTOOL --notarize-app --primary-bundle-id "org.treescan" --username "meagherk@imsweb.com" --password "${PASSWORD}" --asc-provider "VF82MCMA83" --file $BUNDLEDIR/TreeScan.zip | grep RequestUUID | awk '{print $3}')
# Poll for verification completion.
while $XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_APP" -u "meagherk@imsweb.com" -p "${PASSWORD}" | grep "Status: in progress" > /dev/null; do
    echo "Verification in progress..."
    sleep 60
done
echo Results of notarization
$XCRUN $ALTOOL --notarization-info "$REQUEST_UUID_APP" -u "meagherk@imsweb.com" -p "${PASSWORD}"
Echo Any problems notarizing app?
# staple application -- assumes notarization succeeds.
$XCRUN $STAPLER staple $BUNDLEDIR/TreeScan.app
# test notarized
codesign --test-requirement="=notarized" --verify --verbose $BUNDLEDIR/TreeScan.app

Echo How did the app sign go? This step usually will not have issue. Reboot Mac VM and build the dmg using the created app as source jpackageInstallerMac-build-dmg.sh.

# There has been some issue with jpackage and resources being busy during the dmg build. For now, we're juat accepting the hoops that need
# to be jumped through. Meaning, we build the app, reboot the VM then build the dmg -- this process appears to allows work.
# https://squishlist.com/ims/treescan/139/#comment-1812088


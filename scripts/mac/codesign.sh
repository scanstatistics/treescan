#!/bin/sh

REQUIRED_ARGS=1
if [ $# -lt "$REQUIRED_ARGS" ]
then
echo "Usage: `basename $0` <file>"
echo "   example: `basename $0` ./libtreescan.jnilib"
echo "   example: `basename $0` ./treescan"
echo "   example: `basename $0` ./TreeScan.app"
echo "   example: `basename $0` ./Install.app"
exit 1
fi

#security unlock-keychain $HOME/Library/Keychains/login.keychain && codesign --force -v --deep -s "Developer ID Application: Information Management Services, Inc. (VF82MCMA83)" $1
codesign --force -v --deep -s "Developer ID Application: Information Management Services, Inc. (VF82MCMA83)" $1

spctl --assess --verbose=4 --raw $1
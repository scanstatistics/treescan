#!/bin/sh

REQUIRED_ARGS=2
if [ $# -lt "$REQUIRED_ARGS" ]
then
echo "Usage: `basename $0` <nfsf username> <username>"
echo "   example: `basename $0` hostovic treescan"
exit 1
fi

# Mount nfsf.omni.imsweb.com/prj/treescan onto this Mac

echo "mounting treescan"
mount -t smbfs //OMNI\;$1@oriole-03-int/treescan /Users/$2/prj/treescan.development/treescan.home

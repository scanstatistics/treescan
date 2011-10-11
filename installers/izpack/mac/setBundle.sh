#!

# Check for minimum number of arguments.
ARGS=1 # Script requires at least 2 arguments.
if [ $# -lt "$ARGS" ]
then
  echo "Usage: `basename $0` <application directory name>"
  exit 1
fi

echo
echo " Assigning application bundle properties ... "

# Set application file to file to be a bundle
SetFile -a B $1

# change file permissions, otherwise app will fail to launch
chmod -R 755 $1

echo "Done."

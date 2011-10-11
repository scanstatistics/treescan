#!

# Check for mini,mum number of arguments.
ARGS=2 # Script requires at least 2 arguments.
if [ $# -lt "$ARGS" ]
then
  echo "Usage: `basename $0` <symbolic link name> <shared object name> <etc.> ..."
  exit 1
fi

# Iterate over arguments attempting to create symbolic link.
first=1
for arg in "$@"
  do
    if [ $first -eq "1" ]; then
      echo
      echo "Attempting to create symbolic link to TreeScan shared object file."
      echo
      first=0
    else
      if otool -L "$arg" | grep -q 'not found'
      then
        echo "Skipping shared object '$arg', missing dependencies."
        echo
      else
        ln -sf "$arg" "$1"
        echo "Symbolic link created:"
        echo "$1 -> $arg"
        exit 0
      fi
    fi
  done

# Couldn't create link, display otool output to user.
echo
echo "Unable to create symbolic link '$1'."
first=1
for arg in "$@"
  do
    if [ $first -ne "1" ]; then
      echo "Shared object file: otool -L $arg"
      otool -L "$arg"
    fi
    first=0
  done





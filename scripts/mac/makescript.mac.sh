#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=7 # Script requires 7 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <compiler> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` ../treescan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

echo building TreeScan binary ...
cd $2
make clean TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6"
make TreeScan_mac TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6" $7
strip TreeScan_mac
mv TreeScan_mac $1

echo TreeScan done

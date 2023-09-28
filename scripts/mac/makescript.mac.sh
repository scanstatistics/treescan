#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=7 # Script requires 7 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <arch> <minimum os version> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` ../treescan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

echo building zlib library ...
cd $2/zlib/zlib-1.2.7
make clean
make libz.dylib CC="$7 $8 $9" CFLAGS="-stdlib=libc++ -O3 -Wall $4 -DHAVE_HIDDEN  -DHAVE_UNISTD_H" ${11}
echo zlib done
echo

echo building TreeScan binary ...
cd $2
make clean TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 INFOPLIST_FILE="$2/scripts/mac/commandline-info.plist" CC="$6 $9 ${10}"
make TreeScan_mac TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 INFOPLIST_FILE="$2/scripts/mac/commandline-info.plist" CC="$6 $9 ${10}" ${11}
strip TreeScan_mac
mv TreeScan_mac $1

echo TreeScan done

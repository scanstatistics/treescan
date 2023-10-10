#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=10 # Script requires 10 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target> <target rename> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <processor flag> <pthread>"
  echo "   example: `basename $0` libtreescan.linux.so ./libtreescan64.so /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

echo building zlib library ...
cd $3/zlib/zlib-1.2.7
make clean
nice -n 19 make libz.a CC=$8 CFLAGS="-O3 -Wall $5 -fPIC -DHAVE_HIDDEN" $9
echo zlib done
echo

echo building TreeScan binary ...
cd $3
make clean TREESCAN=$3 BOOSTDIR=$4 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 M_CFLAGS=-fPIC
nice -n 19 make $1 TREESCAN=$3 BOOSTDIR=$4 THREAD_DEFINE=${10} COMPILATION=$5 OPTIMIZATION=$6 CC=$7 $9 M_CFLAGS=-fPIC JNI=/etc/alternatives/java_sdk/include JNI_PLAT=/etc/alternatives/java_sdk/include/linux
strip $1.x.x.0
mv $1.x.x.0 $2

echo TreeScan done

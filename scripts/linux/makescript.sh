#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=9 # Script requires 9 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <g++ compiler> <gcc compiler> <processor flag> <pthread>)"
  echo "   example: `basename $0` ./treescan64 /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

echo building zlib library ...
cd $2/zlib/zlib-1.3.1
make clean
nice -n 19 make libz.a CC=$7 CFLAGS="-O3 -Wall $4 -DHAVE_HIDDEN" $8
echo zlib done
echo

echo building TreeScan binary ...
cd $2
make clean TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC=$6
nice -n 19 make TreeScan TREESCAN=$2 BOOSTDIR=$3 THREAD_DEFINE=$9 COMPILATION=$4 OPTIMIZATION=$5 CC=$6 PLAT_DEFINES=-DBOOST_NO_INCLASS_MEMBER_INITIALIZATION $8
strip TreeScan
mv TreeScan $1

echo TreeScan done

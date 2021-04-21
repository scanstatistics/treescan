#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=8 # Script requires 8 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target> <target rename> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <compiler> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` libtreescan.linux.so ../treescan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

echo building TreeScan binary ...
cd $3
make clean TREESCAN=$3 BOOSTDIR=$4 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 M_CFLAGS=-fPIC
nice -n 19 make $1 TREESCAN=$3 BOOSTDIR=$4 THREAD_DEFINE=$9 COMPILATION=$5 OPTIMIZATION=$6 CC=$7 $8 M_CFLAGS=-fPIC JNI=/etc/alternatives/java_sdk/include JNI_PLAT=/etc/alternatives/java_sdk/include/linux
strip $1.x.x.0
mv $1.x.x.0 $2

echo TreeScan done

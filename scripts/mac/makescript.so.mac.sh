#!/bin/sh

# Check for minimum number of arguments.
REQUIRED_ARGS=7 # Script requires 7 arguments.
if [ $# -lt "$REQUIRED_ARGS" ]
then
  echo "Usage: `basename $0` <make target name> <treescan source directory> <boost source directory> <compilation flag> <optimization flag> <compiler> <processor flag> (optional)[<pthread>])"
  echo "   example: `basename $0` ../treescan_linux_8.0_gcc3.3.5_x86_64_32bit /prj/treescan/source /prj/boost/source -m32 -03 /usr/local/gcc3.3.5/bin/g++-3.3.5 -j2"
  exit 1
fi

jni="/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/JavaVM.framework/Versions/1.6.0/Headers"

echo building TreeScan binary ...
cd $2
make clean TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6" M_CFLAGS=-fPIC
make libtreescan.jnilib TREESCAN=$2 BOOSTDIR=$3 COMPILATION=$4 OPTIMIZATION=$5 CC="$6 -dynamiclib" $7 M_CFLAGS=-fPIC JNI=$jni
strip libtreescan.jnilib
mv libtreescan.jnilib $1

echo TreeScan done

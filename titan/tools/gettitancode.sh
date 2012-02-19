#!/bin/sh
KERNELDIR=$1
ROOTDIR=$2
TYPE=$3

cd "$HOME"/flashimg/source.titan/titan/tools

rm tmp
rm tmp.gz
dd if=$KERNELDIR of=tmp.gz bs=1 skip=64

if [ $TYPE == "ufs910" ];then
	lzma -d tmp.gz
else
	gzip -d tmp.gz
fi

str=`strings tmp | grep "Linux version 2.6" | sed 's/Linux version //' | sed 's/(.*)//' | sed 's/  / /'`

code=`./gettitancode "$str"`
#echo "[gettitancode.sh] $str -> $code"
echo code1: $code
cat ../titan.c | sed s/"^#define SYSCODE .*"/"#define SYSCODE $code"/ > titan.c.tmp
mv titan.c.tmp ../titan.c

code=`cat $ROOTDIR/etc/image-version | cut -d= -f2`
cat ../titan.c | sed s/"^#define TIMECODE .*"/"#define TIMECODE \"$code\""/ > titan.c.tmp
mv titan.c.tmp ../titan.c
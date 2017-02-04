#!/data/data/com.termux/files/usr/bin/sh

ME=$(readlink -f $0)
WORKDIR=$(dirname $ME)

cd $WORKDIR/.. && autoconf && autoheader

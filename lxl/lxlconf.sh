#!/data/data/com.termux/files/usr/bin/sh

ME=$(readlink -f $0)
WORKDIR=$(dirname $ME)

cd $WORKDIR/.. && ./configure --prefix=/data/data/com.termux/files/usr --disable-syslog --disable-utmp --disable-utmpx --disable-wtmp

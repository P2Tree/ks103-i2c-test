#!/bin/sh
target='ks103'
address='192.168.1.32'
path='/root/ks103'
user='root'
passwd='root'

make

/usr/bin/ftp -n << !
open ${address}
user ${user} ${passwd}
cd ${path}
put ${target}
bye
!

echo ftp down.

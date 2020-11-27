#! /bin/bash

make all
mount ../../tizen-image/rootfs.img ../../mount
cp rotd ../../mount/root
cp selector ../../mount/root
cp trial ../../mount/root
umount ../../mount


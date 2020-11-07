#! /bin/bash


make all
mount ../../tizen-image/rootfs.img ../../mount
cp trial ../../mount/root/test
cp load_balance ../../mount/root/test
sudo umount ../../mount


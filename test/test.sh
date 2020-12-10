#! /bin/bash

make all
sudo mount ../../tizen-image/rootfs.img ../../mount
cp gpsupdate ../../mount/root/
cp writetest ../../mount/root/
cp file_loc ../../mount/root/
sudo umount ../../mount

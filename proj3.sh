#!/bin/bash

./build-rpi3-arm64.sh
./scripts/mkbootimg_rpi3.sh

rm ../tizen-image/boot.img
rm ../tizen-image/modules.img

cp boot.img ../tizen-image
cp modules.img ../tizen-image

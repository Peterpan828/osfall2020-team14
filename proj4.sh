#!/bin/bash

./build-rpi3-arm64.sh
./scripts/mkbootimg_rpi3.sh

cp boot.img ../tizen-image
cp modules.img ../tizen-image


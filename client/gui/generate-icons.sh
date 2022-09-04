#!/usr/bin/env bash

#
# Copyright 2022 Anysphere, Inc.
# SPDX-License-Identifier: MIT
#

INPUT_FILE="$1"
echo "input file (should be 1024x1024): $INPUT_FILE"

rm -rf assets/icons
rm -rf assets/icons.iconset
mkdir -p assets/icons.iconset

sips -z 16 16 "$INPUT_FILE" --out "assets/icons.iconset/icon_16x16.png"
sips -z 32 32 "$INPUT_FILE" --out "assets/icons.iconset/icon_16x16@2x.png"
sips -z 32 32 "$INPUT_FILE" --out "assets/icons.iconset/icon_32x32.png"
sips -z 64 64 "$INPUT_FILE" --out "assets/icons.iconset/icon_32x32@2x.png"
sips -z 128 128 "$INPUT_FILE" --out "assets/icons.iconset/icon_128x128.png"
sips -z 256 256 "$INPUT_FILE" --out "assets/icons.iconset/icon_128x128@2x.png"
sips -z 256 256 "$INPUT_FILE" --out "assets/icons.iconset/icon_256x256.png"
sips -z 512 512 "$INPUT_FILE" --out "assets/icons.iconset/icon_256x256@2x.png"
sips -z 512 512 "$INPUT_FILE" --out "assets/icons.iconset/icon_512x512.png"

iconutil -c icns -o assets/icon.icns assets/icons.iconset

mv assets/icons.iconset assets/icons

cp assets/icons/icon_256x256.png assets/icon.png
cp assets/icons/icon_256x256.png assets/icon.ico

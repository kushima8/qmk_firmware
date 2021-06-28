#!/bin/sh

set -eu

version=1.0.0-rc.2

for keymap in default \
  test test_Left test_Both \
  via via_Left via_Both
do
  make -j8 keyball/rev1/ball:${keymap} keyball/rev1/noball:${keymap}
  zip -9 tmp/keyball_${keymap}_${version}.zip keyball_rev1_{ball,noball}_${keymap}.hex
done

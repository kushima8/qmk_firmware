#!/bin/sh

set -eu

version=1.1.0

keymaps=()
keymaps+=(default)
keymaps+=(test test_Left test_Both)
keymaps+=(via via_Left via_Both)

targets=()
for keymap in "${keymaps[@]}" ; do
  targets+=(keyball/rev1/ball:${keymap} keyball/rev1/noball:${keymap})
done

make -j8 SKIP_GIT=true "${targets[@]}"

for keymap in "${keymaps[@]}" ; do
  zip -9 tmp/keyball_${keymap}_${version}.zip keyball_rev1_{ball,noball}_${keymap}.hex
done

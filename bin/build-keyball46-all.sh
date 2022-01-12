#!/bin/sh

set -eu

version=1.1.0

keymaps=()
keymaps+=(default)
keymaps+=(test test_Left test_Both)
keymaps+=(via via_Left via_Both)

targets=()
for keymap in "${keymaps[@]}" ; do
  targets+=(keyball/keyball46/ball:${keymap} keyball/keyball46/noball:${keymap})
done

make -j8 SKIP_GIT=yes "${targets[@]}"

for keymap in "${keymaps[@]}" ; do
  zip -9 tmp/keyball_${keymap}_${version}.zip keyball_keyball46_{ball,noball}_${keymap}.hex
done

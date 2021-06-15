#!/bin/sh

set -eu

for km in \
  crkbd/rev1/common:kaoriya \
  crkbd/rev1/legacy:kaoriya \
  dztech/dz60rgb/v2:kaoriya \
  re64:kaoriya \
  uzu42:kaoriya \
; do
  make $km:clean
  make -j8 $km
done

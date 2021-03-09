#!/bin/sh

set -eu

make -j8 crkbd/rev1/common:kaoriya
make -j8 crkbd/rev1/legacy:kaoriya
make -j8 dztech/dz60rgb/v2:kaoriya
make -j8 re64:kaoriya
make -j8 uzu42:kaoriya

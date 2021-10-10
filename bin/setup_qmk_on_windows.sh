#!/bin/sh

# How to setup QMK on Windows with MSYS2 (MinGW64)

set -eu

export MSYSTEM=MINGW64

PYTHONUTF8=1 python3 -m pip install qmk==1.0.0

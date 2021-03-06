#!/bin/sh

set -eu

make -j8 handwired/yuiop/yuiop27:default
make -j8 handwired/yuiop/yuiop47:default
make -j8 handwired/yuiop/yuiop50d:default
make -j8 handwired/yuiop/yuiop50d:test
make -j8 handwired/yuiop/yuiop22rs:default
make -j8 handwired/yuiop/yuiop63hhs:default

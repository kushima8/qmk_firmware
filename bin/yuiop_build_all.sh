#!/bin/sh

set -eu

make -j8 handwired/yuiop/yuiop27:default
make -j8 handwired/yuiop/yuiop47:default
make -j8 handwired/yuiop/yuiop50d:default
make -j8 handwired/yuiop/yuiop22rs:default
make -j8 handwired/yuiop/yuiop63hhs:default
make -j8 handwired/yuiop/yuiop60hh4:default
make -j8 handwired/yuiop/yuiop31rs:test

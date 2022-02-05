#!/bin/sh

set -eu

make -j8 SKIP_GIT=yes handwired/yuiop/yuiop47:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop50d:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop22rs:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop60hh4:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop31rs:test
make -j8 SKIP_GIT=yes \
  handwired/yuiop/yuiop60hh5:default \
  handwired/yuiop/yuiop60hh5:test

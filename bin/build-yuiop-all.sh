#!/bin/sh

set -eu

#keyboards=()
##keyboards+=(handwired/yuiop/yuiop27) # deprecated
#keyboards+=(handwired/yuiop/yuiop47)
#keyboards+=(handwired/yuiop/yuiop50d)
#keyboards+=(handwired/yuiop/yuiop22rs)
##keyboards+=(handwired/yuiop/yuiop63hhs) # deprecated
#keyboards+=(handwired/yuiop/yuiop60hh4)
#keyboards+=(handwired/yuiop/yuiop31rs)

make -j8 SKIP_GIT=yes handwired/yuiop/yuiop47:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop50d:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop22rs:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop60hh4:default
make -j8 SKIP_GIT=yes handwired/yuiop/yuiop31rs:test

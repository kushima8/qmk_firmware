#!/bin/sh

set -eu

#make handwired/yuiop/yuiop27:default:clean
#make handwired/yuiop/yuiop47:default:clean
#make handwired/yuiop/yuiop50d:default:clean
#make handwired/yuiop/yuiop22rs:default:clean
#make handwired/yuiop/yuiop63hhs:default:clean
#make handwired/yuiop/yuiop60hh4:default:clean
#make handwired/yuiop/yuiop31rs:test:clean

#make -j8 handwired/yuiop/yuiop27:default # deprecated
make -j8 handwired/yuiop/yuiop47:default
make -j8 handwired/yuiop/yuiop50d:default
make -j8 handwired/yuiop/yuiop22rs:default
#make -j8 handwired/yuiop/yuiop63hhs:default # deprecated
make -j8 handwired/yuiop/yuiop60hh4:default
make -j8 handwired/yuiop/yuiop31rs:test

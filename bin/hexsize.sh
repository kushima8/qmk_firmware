#!/bin/sh

set -eu

for f in "$@" ; do
  echo -n "$f	"
  cut -c 2,3 "$f" | awk '{s+=strtonum("0x"$1)}END{print s}'
done

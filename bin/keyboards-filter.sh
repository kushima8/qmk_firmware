#!/bin/sh

set -eu

exec sed \
  -e '\!^crkbd!p' \
  -e '\!^dztech/dz60rgb/v2!p' \
  -e '\!^yuiop!p' \
  -e '\!^keyball!p' \
  -e '\!^re64!p' \
  -e '\!^silverbullet44!p' \
  -e '\!^uzu42!p' \
  -e 'd' "$@"

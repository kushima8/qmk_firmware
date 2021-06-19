#!/bin/sh

set -eu

cp -Rp ../../Yowkees/keyball/qmk_firmware/keyboards/keyball keyboards/
find keyboards/keyball -type f -regex '.*\.\(c\|h\|json\|md\)' | xargs unix2dos -q

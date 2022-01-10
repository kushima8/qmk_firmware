#!/bin/sh

set -eu

$(dirname "$0")/../util/list_keyboards.sh -f | $(dirname "$0")/keyboards-filter.sh

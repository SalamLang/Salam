#!/bin/sh
# Builds gui.ui page tests for the web and replays their .script files in a
# headless browser, so the web build keeps answering like a native window.
#
# Usage:
#   tools/bash/test-ui-web.sh <salam> <en|fa>
#
# Needs playwright-core on NODE_PATH. CHROME picks the browser, otherwise the
# installed Google Chrome or Chromium is used.

set -eu

[ $# -eq 2 ] || {
    echo "usage: $0 <salam> <en|fa>" >&2
    exit 2
}

SALAM=$1
LANG_CODE=$2
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DIR=$(mktemp -d)
trap 'rm -rf "$DIR"' EXIT

status=0
for name in bind component_pub_file dir_component element_keep element_page \
    event_each_param events_bubble events_callback events_rows grid_table \
    input_clear_bound input_push_reset input_value list loop_key_use \
    scope_components scope_counter scope_keyed scope_list scope_state; do
    src="$ROOT/tests/$LANG_CODE/ui/$name.salam"
    if "$SALAM" build "$src" --platform=web --output="$DIR/$name.html" > "$DIR/build.log" 2>&1; then
        node "$ROOT/tools/js/test-ui-web.js" "$DIR/$name.html" "$src" || status=1
    else
        echo "FAIL $src (build)"
        cat "$DIR/build.log"
        status=1
    fi
done
exit "$status"

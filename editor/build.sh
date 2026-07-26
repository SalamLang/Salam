#!/bin/sh
set -e
cd "$(dirname "$0")/.."
./bin/salam web editor/src/page.salam --output=editor/index.html

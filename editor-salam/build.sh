#!/bin/sh
set -e
cd "$(dirname "$0")/.."
./salam web editor-salam/src/page.salam --output=editor-salam/index.html

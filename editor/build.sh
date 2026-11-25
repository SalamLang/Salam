#!/bin/sh
set -e
cd "$(dirname "$0")/.."
./salam build editor/page.salam --platform=web --output=editor/index.html

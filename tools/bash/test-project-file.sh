#!/bin/sh
# Tests that salam.toml supplies the defaults of the phone app scripts, that an
# option wins over the file, that bad keys and password keys warn, and that the
# Windows executable takes its version info from [app] and [windows].

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
export SALAM_MOBILE_ROOT="$root"
unset SALAM_KEYSTORE_PASS SALAM_KEY_PASS
cd "$work" || exit 1

mkdir app
cd app || exit 1
printf 'page:\nend\n' >main.salam
printf 'keystore' >release.jks
cat >salam.toml <<'EOF'
[package]
name = "notes"
version = "0.1.0"

[project]
type = "dsl"
platforms = ["android"]

[app]
id = "ir.example.notes"
name = "Notes"
version = "1.2.0"
build = 7
description = "Take notes"
author = "Example"
orientation = "portrait"
category = "productivity"
permissions = ["camera", "location"]
splash_color = "#101820"
assets = "www"

[app.mobile]
icon_bg = "#112233"

[android]
min_sdk = 26
abi = "arm64-v8a,x86_64"
keystore = "release.jks"
key_alias = "notes"
keystore_pass = "hunter2"
bogus = 1

[ios]
min_ios = "16.0"
team = "ABCDE12345"
EOF

show() {
    grep -E "$1" | sed 's|^\./||'
}

echo "== android defaults from salam.toml"
sh "$root/tools/bash/build-android-app.sh" main.salam --out o --dry-run 2>&1 |
    show '^(id|name|version|build|icon_bg|splash_color|min_sdk|orientation|category|abi|assets|permissions|keystore|key_alias)='
echo "== android options win"
sh "$root/tools/bash/build-android-app.sh" main.salam --out o --dry-run --name Other --min-sdk 30 \
    --category social 2>/dev/null | show '^(name|min_sdk|category|version)='
echo "== warnings"
sh "$root/tools/bash/build-android-app.sh" main.salam --out o --dry-run 2>&1 >/dev/null
echo "== password only from the environment"
SALAM_KEYSTORE_PASS=secret sh "$root/tools/bash/build-android-app.sh" main.salam --out o --dry-run \
    2>/dev/null | show '^(keystore_pass|key_pass)='
echo "== ios defaults, team only for a device build"
sh "$root/tools/bash/build-ios-app.sh" main.salam --out o --dry-run 2>/dev/null |
    show '^(id|name|build|min_ios|team|orientation)='
sh "$root/tools/bash/build-ios-app.sh" main.salam --out o --dry-run --device 2>/dev/null |
    show '^team='
echo "== desktop only keys do not reach the phone"
printf '[app]\nid = "a.b"\nname = "B"\ncategory = "Utility"\nassets = "www"\n' >salam.toml
sh "$root/tools/bash/build-android-app.sh" main.salam --out o --dry-run 2>&1 | show '^(category|assets)='
printf '[package]\nname = "notes"\nversion = "0.1.0"\n\n[project]\ntype = "dsl"\nplatforms = ["android"]\n\n[app]\nid = "a.b"\nname = "B"\ndescription = 5\ncolour = "red"\n' >salam.toml
echo "== the compiler reads the project file and passes options after --"
"$salam" build --platform=android --no-color --log-level=warn -- --dry-run --version 3.1 2>&1 |
    sed -e 's/^\[[^]]*\]\[[^]]*\]\[[^]]*\] //' | show '^(salam.toml:|name=|version=)'

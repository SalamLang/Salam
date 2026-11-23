#!/bin/sh
# Shared by build-android-app.sh and build-ios-app.sh: checks and scales one PNG.

icon_resize() {
    if command -v sips >/dev/null 2>&1; then
        sips -s format png -z "$2" "$2" "$1" --out "$3" >/dev/null 2>&1 && return 0
    fi
    if command -v magick >/dev/null 2>&1; then
        magick "$1" -resize "${2}x${2}!" "$3" >/dev/null 2>&1 && return 0
    fi
    if command -v convert >/dev/null 2>&1; then
        convert "$1" -resize "${2}x${2}!" "$3" >/dev/null 2>&1 && return 0
    fi
    cp "$1" "$3"
}

icon_check() {
    [ -f "$1" ] || {
        echo "$2: no such image: $1" >&2
        return 1
    }
    case $(head -c 8 "$1" | od -An -tx1 | tr -d ' \n') in
    89504e470d0a1a0a) return 0 ;;
    esac
    echo "$2: the image must be a PNG: $1" >&2
    return 1
}

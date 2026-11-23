#!/bin/sh
# Shared by build-android-app.sh and build-ios-app.sh: reads the [app] tables
# of salam.toml and checks the identity options.
#
# [app] id, name, version and icon are shared with the desktop bundle.
# [app.mobile] and then [app.android] or [app.ios] hold everything else, and
# a later table wins over an earlier one.

APP_CFG=
APP_CFG_DIR=

app_cfg_load() {
    APP_CFG_DIR=$(dirname "$1")
    APP_CFG=$(awk -v platform="$2" '
        function trim(s) { gsub(/^[ \t]+|[ \t]+$/, "", s); return s }
        /^[ \t]*\[/ {
            name = trim($0)
            gsub(/^\[|\][ \t]*(#.*)?$/, "", name)
            name = trim(name)
            prio = 0
            if (name == "app") prio = 1
            if (name == "app.mobile") prio = 2
            if (name == "app." platform) prio = 3
            next
        }
        prio > 0 && /^[ \t]*[A-Za-z_][A-Za-z0-9_-]*[ \t]*=/ {
            eq = index($0, "=")
            key = trim(substr($0, 1, eq - 1))
            raw = trim(substr($0, eq + 1))
            if (prio == 1 && key != "id" && key != "name" && key != "version" && key != "icon") next
            if (substr(raw, 1, 1) == "\"") {
                rest = substr(raw, 2)
                val = substr(rest, 1, index(rest, "\"") - 1)
            } else if (substr(raw, 1, 1) == "[") {
                val = ""
                rest = raw
                while ((q = index(rest, "\"")) > 0) {
                    rest = substr(rest, q + 1)
                    e = index(rest, "\"")
                    val = val (val == "" ? "" : " ") substr(rest, 1, e - 1)
                    rest = substr(rest, e + 1)
                }
            } else {
                sub(/[ \t]*#.*$/, "", raw)
                val = raw
            }
            if (!(key in have) || prio >= have[key]) {
                have[key] = prio
                out[key] = val
            }
        }
        END { for (k in out) printf "%s\t%s\n", k, out[k] }
    ' "$1")
}

app_cfg_get() {
    printf '%s\n' "$APP_CFG" | awk -F'\t' -v k="$1" '$1 == k { print substr($0, length(k) + 2); exit }'
}

app_cfg_path() {
    v=$(app_cfg_get "$1")
    case $v in
    "") ;;
    /*) printf '%s\n' "$v" ;;
    *) printf '%s/%s\n' "$APP_CFG_DIR" "$v" ;;
    esac
}

app_check_int() {
    case $2 in
    "" | *[!0-9]*)
        echo "$3: $1 wants a whole number, not '$2'" >&2
        return 1
        ;;
    esac
}

app_check_orientation() {
    case $1 in
    "" | portrait | landscape | auto) return 0 ;;
    esac
    echo "$2: --orientation is portrait, landscape or auto, not '$1'" >&2
    return 1
}

APP_CATEGORIES="game social productivity utilities education finance health lifestyle music news photo travel weather business"
APP_CATEGORIES="$APP_CATEGORIES entertainment sports books food maps video reference"

app_category_known() {
    [ -n "$1" ] || return 0
    for c in $APP_CATEGORIES; do
        [ "$c" = "$1" ] && return 0
    done
    echo "$2: unknown category '$1' ($APP_CATEGORIES)" >&2
    return 1
}

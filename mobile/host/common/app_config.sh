#!/bin/sh
# Shared by build-android-app.sh and build-ios-app.sh: reads the [app] tables
# of salam.toml and checks the identity options.
#
# [app] holds the keys every platform shares (id, name, version, build, icon,
# description, author, orientation, category, permissions, ...). [app.mobile],
# then [app.android] or [app.ios], then the plain [android] or [ios] table
# override it, and a later table wins over an earlier one. docs/PROJECT_FILE.md
# lists every key. Passwords are never read from the file.

APP_CFG=
APP_CFG_DIR=

app_cfg_load() {
    APP_CFG_DIR=$(dirname "$1")
    app_cfg_check "$1" "$2"
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
            if (name == platform) prio = 4
            next
        }
        prio > 0 && /^[ \t]*[A-Za-z_][A-Za-z0-9_-]*[ \t]*=/ {
            eq = index($0, "=")
            key = trim(substr($0, 1, eq - 1))
            raw = trim(substr($0, eq + 1))
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

app_cfg_check() {
    awk -v platform="$2" -v file="$1" '
        function trim(s) { gsub(/^[ \t]+|[ \t]+$/, "", s); return s }
        BEGIN {
            n = split("id name version build description author publisher icon icon_bg splash_color splash_icon orientation category permissions permission main assets engine runtime copyright fonts scheme link_host min_sdk target_sdk abi keystore key_alias min_ios team profile sign_identity ipa", k, " ")
            for (i = 1; i <= n; i++) known[k[i]] = 1
            split("keystore_pass keystore_password key_pass key_password password storepass", sec, " ")
            for (i in sec) secret[sec[i]] = 1
        }
        /^[ \t]*\[/ {
            name = trim($0)
            gsub(/^\[|\][ \t]*(#.*)?$/, "", name)
            name = trim(name)
            use = (name == "app" || name == "app.mobile" || name == "app." platform || name == platform)
            next
        }
        use && /^[ \t]*[A-Za-z_][A-Za-z0-9_-]*[ \t]*=/ {
            key = trim(substr($0, 1, index($0, "=") - 1))
            if (key in secret) {
                printf "%s: warning: %s is ignored, secrets do not belong in salam.toml (use SALAM_KEYSTORE_PASS and SALAM_KEY_PASS)\n", file, key | "cat 1>&2"
            } else if (!(key in known)) {
                printf "%s: warning: unknown key %s in [%s]\n", file, key, name | "cat 1>&2"
            }
        }
    ' "$1"
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

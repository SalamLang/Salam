#!/bin/sh
# Salam installer for Linux, macOS and the BSDs.
#
# Usage:
#   curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh | sh
#   curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh | sh -s -- --dir ./bin
#   ./install.sh --version 0.3.5
#
# Options:
#   -d, --dir DIR         install into DIR (default: $HOME/.salam/bin)
#   -v, --version X.Y.Z   install this exact release instead of the newest
#   -p, --platform NAME   force a release asset (linux, mac, linux-aarch64, ...)
#       --no-modify-path  never touch your shell startup files
#       --no-color        plain output, no colors
#   -h, --help            show this help
#
# Environment variables:
#   SALAM_INSTALL_DIR     same as --dir
#   SALAM_VERSION         same as --version
#   SALAM_PLATFORM        same as --platform
#   SALAM_NO_MODIFY_PATH  set to 1 for --no-modify-path
#   NO_COLOR              set to anything for --no-color
#   GITHUB_TOKEN, GH_TOKEN  used when talking to api.github.com
#
# Release assets this understands:
#   linux           Linux x86_64          mac    macOS
#   linux-i686      Linux 32-bit x86
#   linux-aarch64   Linux ARM64
#   linux-armhf     Linux 32-bit ARM (hard float)
#
# Names are probed against the actual release, so a build published later
# (freebsd, mac-aarch64, ...) is picked up by this script unchanged.
#
# Plain POSIX sh on purpose: this has to run unmodified under dash, under
# busybox ash, under ksh/pdksh on the BSDs, and under the ancient bash
# macOS still ships. No arrays, no [[ ]], no local, no GNU-only flags.

set -eu

REPO="SalamLang/Salam"
RELEASES_URL="https://github.com/$REPO/releases"
ISSUES_URL="https://github.com/$REPO/issues"
RAW_URL="https://raw.githubusercontent.com/$REPO/refs/heads/main"
USER_AGENT="SalamInstaller/2.0 (sh)"

INSTALL_DIR="${SALAM_INSTALL_DIR:-}"
VERSION="${SALAM_VERSION:-}"
PLATFORM="${SALAM_PLATFORM:-}"
PLATFORM_FORCED=0

if [ -n "$PLATFORM" ]; then
    PLATFORM_FORCED=1
fi

API_TOKEN="${GITHUB_TOKEN:-${GH_TOKEN:-}}"

MODIFY_PATH=1
if [ -n "${SALAM_NO_MODIFY_PATH:-}" ]; then
    MODIFY_PATH=0
fi

STEP=0
STEPS=7
BAR_SLOTS=24
MAX_TAGS=8
DL_PID=""
WORKDIR=""

# Some init systems and cron setups start a shell without HOME.
if [ -z "${HOME:-}" ]; then
    HOME=$(cd ~ 2>/dev/null && pwd || printf '/tmp')
    export HOME
fi

# ---------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------

# Everything the installer says goes to stderr, so `... | sh` keeps
# working and so a caller can still capture what salam itself prints.
IS_TTY=0
if [ -t 2 ]; then
    IS_TTY=1
fi

USE_COLOR=$IS_TTY
if [ -n "${NO_COLOR:-}" ] || [ "${TERM:-dumb}" = "dumb" ]; then
    USE_COLOR=0
fi

set_colors() {
    C_OFF=""
    C_STEP=""
    C_GOOD=""
    C_WARN=""
    C_ERR=""
    C_DIM=""
    C_BOLD=""

    if [ "$USE_COLOR" = 1 ]; then
        C_OFF=$(printf '\033[0m')
        C_STEP=$(printf '\033[1;36m')
        C_GOOD=$(printf '\033[1;32m')
        C_WARN=$(printf '\033[1;33m')
        C_ERR=$(printf '\033[1;31m')
        C_DIM=$(printf '\033[2m')
        C_BOLD=$(printf '\033[1m')
    fi
}
set_colors

# Width is only used to wipe the tail of a repainted line, so a wrong
# guess costs nothing worse than a stray character.
term_width() {
    _w="${COLUMNS:-}"

    if [ -z "$_w" ] && command -v tput >/dev/null 2>&1; then
        _w=$(tput cols 2>/dev/null || true)
    fi

    if [ -z "$_w" ] && command -v stty >/dev/null 2>&1; then
        _w=$(stty size 2>/dev/null | cut -d' ' -f2 || true)
    fi

    case "$_w" in
    '' | *[!0-9]*) _w=80 ;;
    esac

    if [ "$_w" -lt 40 ] || [ "$_w" -gt 200 ]; then
        _w=80
    fi

    printf '%s' "$_w"
}
WIDTH=$(term_width)

step() {
    STEP=$((STEP + 1))
    printf '\n%s[%d/%d]%s %s\n' "$C_STEP" "$STEP" "$STEPS" "$C_OFF" "$*" >&2
}

info() {
    printf '      %s\n' "$*" >&2
}

field() {
    printf '      %s%-11s%s %s\n' "$C_DIM" "$1" "$C_OFF" "$2" >&2
}

warn() {
    printf '      %swarning:%s %s\n' "$C_WARN" "$C_OFF" "$*" >&2
}

die() {
    if [ "$IS_TTY" = 1 ]; then
        printf '\r' >&2
    fi

    printf '\n%serror:%s %s\n' "$C_ERR" "$C_OFF" "$1" >&2
    shift

    for _line in "$@"; do
        if [ -z "$_line" ]; then
            printf '\n' >&2
        else
            printf '       %s\n' "$_line" >&2
        fi
    done

    printf '\n' >&2
    exit 1
}

spaces() {
    _n=$1
    _s=''

    while [ "$_n" -gt 0 ]; do
        _s="$_s "
        _n=$((_n - 1))
    done

    printf '%s' "$_s"
}

# One line, repainted in place. Without a terminal there is no cursor to
# move, so callers print milestones instead of animating.
status() {
    if [ "$IS_TTY" != 1 ]; then
        return 0
    fi

    _pad=$((WIDTH - ${#1} - 1))

    if [ "$_pad" -lt 0 ]; then
        _pad=0
    fi

    printf '\r%s%s' "$1" "$(spaces $_pad)" >&2
}

status_end() {
    if [ "$IS_TTY" = 1 ]; then
        printf '\n' >&2
    fi
}

usage() {
    cat >&2 <<'USAGE'

Salam installer for Linux, macOS and the BSDs

Usage:
  install.sh [--dir DIR] [--version X.Y.Z] [--platform NAME]
             [--no-modify-path] [--no-color]

Options:
  -d, --dir DIR         install into DIR (default $HOME/.salam/bin)
  -v, --version X.Y.Z   install this exact release instead of the newest
  -p, --platform NAME   force a release asset (linux, linux-i686,
                        linux-aarch64, linux-armhf, mac)
      --no-modify-path  never touch your shell startup files
      --no-color        plain output, no colors
  -h, --help            show this help

Environment:
  SALAM_INSTALL_DIR, SALAM_VERSION, SALAM_PLATFORM,
  SALAM_NO_MODIFY_PATH, NO_COLOR, GITHUB_TOKEN

USAGE
}

# ---------------------------------------------------------------------
# Formatting
# ---------------------------------------------------------------------

fmt_size() {
    _n=${1:-0}

    case "$_n" in
    '' | *[!0-9]*) _n=0 ;;
    esac

    if [ "$_n" -ge 1073741824 ]; then
        printf '%d.%d GB' "$((_n / 1073741824))" "$(((_n % 1073741824) * 10 / 1073741824))"
    elif [ "$_n" -ge 1048576 ]; then
        printf '%d.%d MB' "$((_n / 1048576))" "$(((_n % 1048576) * 10 / 1048576))"
    elif [ "$_n" -ge 1024 ]; then
        printf '%d KB' "$((_n / 1024))"
    else
        printf '%d B' "$_n"
    fi
}

fmt_duration() {
    _t=${1:-0}

    case "$_t" in
    '' | *[!0-9]*) _t=0 ;;
    esac

    # A stalled transfer produces nonsense estimates; say so rather than
    # promise the user four days.
    if [ "$_t" -gt 359999 ]; then
        printf -- '--:--'
        return 0
    fi

    if [ "$_t" -ge 3600 ]; then
        printf '%d:%02d:%02d' "$((_t / 3600))" "$(((_t % 3600) / 60))" "$((_t % 60))"
    else
        printf '%02d:%02d' "$((_t / 60))" "$((_t % 60))"
    fi
}

now_seconds() {
    _s=$(date +%s 2>/dev/null || true)

    case "$_s" in
    '' | *[!0-9]*) _s=0 ;;
    esac

    printf '%s' "$_s"
}

file_size() {
    _b=0

    if [ -f "$1" ]; then
        _b=$(wc -c <"$1" 2>/dev/null | tr -d ' \011\012' || true)
    fi

    case "$_b" in
    '' | *[!0-9]*) _b=0 ;;
    esac

    printf '%s' "$_b"
}

count_files() {
    _c=$(find "$1" -type f 2>/dev/null | wc -l | tr -d ' \011\012' || true)

    case "$_c" in
    '' | *[!0-9]*) _c=0 ;;
    esac

    printf '%s' "$_c"
}

make_bar() {
    _filled=$1
    _i=0
    _bar=''

    while [ "$_i" -lt "$BAR_SLOTS" ]; do
        if [ "$_i" -lt "$_filled" ]; then
            _bar="$_bar#"
        else
            _bar="$_bar."
        fi

        _i=$((_i + 1))
    done

    printf '%s' "$_bar"
}

progress_line() {
    _done=${1:-0}
    _total=${2:-0}
    _tenths=${3:-0}

    _rate='--'
    _speed=0

    if [ "$_tenths" -ge 1 ]; then
        # Divide before multiplying: a 32-bit shell would overflow on
        # (bytes * 10) once an archive passes 200 MB.
        # shellcheck disable=SC2017  # the lost precision is under a byte/s
        _speed=$((_done / _tenths * 10))
        _rate="$(fmt_size $_speed)/s"
    fi

    if [ "$_total" -le 0 ]; then
        printf '      %s  %s' "$(fmt_size $_done)" "$_rate"
        return 0
    fi

    _pct=$((_done * 100 / _total))

    if [ "$_pct" -gt 100 ]; then
        _pct=100
    fi

    _eta='--:--'

    if [ "$_speed" -gt 0 ] && [ "$_done" -le "$_total" ]; then
        _eta=$(fmt_duration $(((_total - _done) / _speed)))
    fi

    printf '      [%s] %3d%%  %s / %s  %s  ETA %s' \
        "$(make_bar $((_pct * BAR_SLOTS / 100)))" "$_pct" \
        "$(fmt_size $_done)" "$(fmt_size $_total)" "$_rate" "$_eta"
}

# ---------------------------------------------------------------------
# Cleanup
# ---------------------------------------------------------------------

# shellcheck disable=SC2329  # invoked from the traps below
cleanup() {
    if [ -n "$DL_PID" ]; then
        kill "$DL_PID" 2>/dev/null || true
        DL_PID=""
    fi

    if [ -n "$WORKDIR" ] && [ -d "$WORKDIR" ]; then
        rm -rf "$WORKDIR" || true
    fi
}

trap cleanup EXIT
trap 'status_end; cleanup; exit 130' INT
trap 'status_end; cleanup; exit 143' TERM
trap 'status_end; cleanup; exit 129' HUP

# ---------------------------------------------------------------------
# Arguments
# ---------------------------------------------------------------------

while [ $# -gt 0 ]; do
    case "$1" in
    --dir | -d)
        if [ $# -lt 2 ]; then
            die "$1 requires a directory"
        fi
        INSTALL_DIR="$2"
        shift 2
        ;;
    --dir=*)
        INSTALL_DIR="${1#--dir=}"
        shift
        ;;
    --version | -v)
        if [ $# -lt 2 ]; then
            die "$1 requires a version"
        fi
        VERSION="$2"
        shift 2
        ;;
    --version=*)
        VERSION="${1#--version=}"
        shift
        ;;
    --platform | -p)
        if [ $# -lt 2 ]; then
            die "$1 requires a platform name"
        fi
        PLATFORM="$2"
        PLATFORM_FORCED=1
        shift 2
        ;;
    --platform=*)
        PLATFORM="${1#--platform=}"
        PLATFORM_FORCED=1
        shift
        ;;
    --no-modify-path)
        MODIFY_PATH=0
        shift
        ;;
    --no-color)
        USE_COLOR=0
        set_colors
        shift
        ;;
    -h | --help | help)
        usage
        exit 0
        ;;
    *)
        usage
        die "unknown option: $1" "run with --help to see what this accepts"
        ;;
    esac
done

VERSION="${VERSION#v}"

# ---------------------------------------------------------------------
# Downloading
# ---------------------------------------------------------------------

DL_TOOL=""
UNAME_S=$(uname -s 2>/dev/null || printf 'unknown')

if command -v curl >/dev/null 2>&1; then
    DL_TOOL="curl"
elif command -v wget >/dev/null 2>&1; then
    DL_TOOL="wget"
elif [ "$UNAME_S" = "FreeBSD" ] && command -v fetch >/dev/null 2>&1; then
    # FreeBSD installs neither curl nor wget by default; fetch(1) is in
    # the base system and is what the OS fetches its own packages with.
    DL_TOOL="fetch"
elif command -v ftp >/dev/null 2>&1; then
    case "$UNAME_S" in
    OpenBSD | NetBSD | DragonFly)
        # Same story on the other BSDs, where ftp(1) speaks HTTPS. The
        # Linux client of that name does not, hence the guard.
        DL_TOOL="ftp"
        ;;
    esac
fi

WGET_FLAGS="--tries=3 --timeout=20"

if [ "$DL_TOOL" = "wget" ]; then
    case "$(wget --help 2>&1 | head -n 5 || true)" in
    *BusyBox* | *busybox*) WGET_FLAGS="-T 20" ;;
    esac
fi

fetch_to_stdout() {
    _auth=""

    case "$1" in
    https://api.github.com/*)
        # Anonymous API calls are rate limited per IP, which a busy
        # office or a CI runner burns through fast.
        if [ -n "$API_TOKEN" ]; then
            _auth="Authorization: Bearer $API_TOKEN"
        fi
        ;;
    esac

    case "$DL_TOOL" in
    curl)
        if [ -n "$_auth" ]; then
            curl -fsSL --retry 3 --retry-delay 2 --connect-timeout 20 \
                -A "$USER_AGENT" -H "$_auth" "$1"
        else
            curl -fsSL --retry 3 --retry-delay 2 --connect-timeout 20 \
                -A "$USER_AGENT" "$1"
        fi
        ;;
    wget)
        if [ -n "$_auth" ]; then
            wget $WGET_FLAGS -qO- -U "$USER_AGENT" \
                --header="$_auth" "$1"
        else
            wget $WGET_FLAGS -qO- -U "$USER_AGENT" "$1"
        fi
        ;;
    fetch)
        fetch -q -o - "$1" 2>/dev/null
        ;;
    *)
        ftp -V -o - "$1" 2>/dev/null
        ;;
    esac
}

fetch_to_file() {
    case "$DL_TOOL" in
    curl)
        curl -fL --retry 3 --retry-delay 2 --connect-timeout 20 \
            -A "$USER_AGENT" -o "$2" "$1"
        ;;
    wget)
        wget $WGET_FLAGS -q -U "$USER_AGENT" -O "$2" "$1"
        ;;
    fetch)
        fetch -q -o "$2" "$1"
        ;;
    *)
        ftp -V -o "$2" "$1"
        ;;
    esac
}

# Headers only: it settles whether a release publishes this platform's
# asset without pulling a hundred megabytes to find out, and hands back
# the size the progress bar needs.
PROBE_SIZE=0

probe_url() {
    PROBE_SIZE=0
    _headers=""

    case "$DL_TOOL" in
    curl)
        # No -f here: a 404 has to come back with its headers so the
        # status check below can see it. GitHub answers a missing asset
        # with a 404 that still carries a content-length, so trusting
        # that header alone would call every wrong name a hit.
        _headers=$(curl -sSL -I --connect-timeout 20 --max-time 60 \
            -A "$USER_AGENT" "$1" 2>/dev/null || true)
        ;;
    wget)
        _headers=$(wget $WGET_FLAGS --spider -S -U "$USER_AGENT" "$1" 2>&1 || true)
        ;;
    fetch)
        # -s prints the size and downloads nothing, which is exactly the
        # question being asked here.
        _len=$(fetch -s "$1" 2>/dev/null || true)

        case "$_len" in
        '' | *[!0-9]*) return 1 ;;
        esac

        PROBE_SIZE=$_len
        return 0
        ;;
    *)
        # ftp(1) has no HEAD mode; 2 tells the caller to settle it with a
        # plain GET rather than to treat this as a missing asset.
        return 2
        ;;
    esac

    if [ -z "$_headers" ]; then
        return 1
    fi

    _headers=$(printf '%s\n' "$_headers" | tr -d '\015')

    # The last status line is the one that settles it; the earlier ones
    # belong to the redirect hops.
    _code=$(printf '%s\n' "$_headers" |
        sed -n 's#^ *HTTP/[0-9.]* *\([0-9][0-9][0-9]\).*#\1#p' | tail -n 1)

    case "$_code" in
    2??) ;;
    *) return 1 ;;
    esac

    # The last Content-Length wins, for the same reason.
    _len=$(printf '%s\n' "$_headers" |
        sed -n 's/^ *[Cc]ontent-[Ll]ength: *\([0-9][0-9]*\).*/\1/p' | tail -n 1)

    case "$_len" in
    '' | *[!0-9]*) _len=0 ;;
    esac

    PROBE_SIZE=$_len
    return 0
}

NAP="1"
NAP_TENTHS=10

pick_nap() {
    # Sub-second sleeps are not POSIX, but every system worth animating
    # for has them. One doomed call settles it.
    if sleep 0.25 2>/dev/null; then
        NAP="0.25"
        NAP_TENTHS=2
    else
        NAP="1"
        NAP_TENTHS=10
    fi
}

nap() {
    sleep "$NAP" 2>/dev/null || sleep 1
}

# $1 = start second, $2 = tenths counted from our own sleeps. The wall
# clock wins when the loop body ran slower than the sleeps promised.
elapsed_tenths() {
    _wall=$((($(now_seconds) - $1) * 10))

    if [ "$_wall" -gt "$2" ]; then
        printf '%s' "$_wall"
    else
        printf '%s' "$2"
    fi
}

download_file() {
    _url=$1
    _dest=$2
    _total=${3:-0}

    rm -f "$_dest"

    _start=$(now_seconds)
    _tenths=0

    fetch_to_file "$_url" "$_dest" >/dev/null 2>&1 &
    DL_PID=$!

    if [ "$IS_TTY" = 1 ]; then
        while kill -0 "$DL_PID" 2>/dev/null; do
            status "$(progress_line "$(file_size "$_dest")" "$_total" \
                "$(elapsed_tenths "$_start" "$_tenths")")"
            nap
            _tenths=$((_tenths + NAP_TENTHS))
        done
    fi

    if wait "$DL_PID"; then
        _rc=0
    else
        _rc=1
    fi

    DL_PID=""

    if [ "$_rc" != 0 ] || [ ! -s "$_dest" ]; then
        status_end
        rm -f "$_dest"
        return 1
    fi

    _got=$(file_size "$_dest")

    if [ "$IS_TTY" = 1 ]; then
        status "$(progress_line "$_got" "$_total" "$(elapsed_tenths "$_start" "$_tenths")")"
        status_end
    else
        info "downloaded $(fmt_size "$_got")"
    fi

    return 0
}

# Watches a background job and keeps the line moving, so a long unpack
# never looks like a hang.
spin_wait() {
    _pid=$1
    _label=$2
    _dir=${3:-}
    _start=$(now_seconds)
    _frame=0

    if [ "$IS_TTY" = 1 ]; then
        while kill -0 "$_pid" 2>/dev/null; do
            case $((_frame % 4)) in
            0) _mark='-' ;;
            1) _mark="\\" ;;
            2) _mark='|' ;;
            *) _mark='/' ;;
            esac

            _detail=""

            if [ -n "$_dir" ] && [ -d "$_dir" ]; then
                _detail=", $(count_files "$_dir") files"
            fi

            status "      $_mark $_label ($(fmt_duration $(($(now_seconds) - _start)))$_detail)"

            _frame=$((_frame + 1))
            nap
        done

        status_end
    fi

    if wait "$_pid"; then
        return 0
    fi

    return 1
}

# ---------------------------------------------------------------------
# Release lookup
# ---------------------------------------------------------------------

latest_tag_via_redirect() {
    # /releases/latest answers with a redirect to the real tag and costs
    # no API quota, unlike api.github.com which rate limits per IP.
    _location=""

    case "$DL_TOOL" in
    curl)
        _location=$(curl -fsS -o /dev/null -A "$USER_AGENT" \
            -w '%{redirect_url}' --connect-timeout 20 --max-time 30 \
            "$RELEASES_URL/latest" 2>/dev/null || true)
        ;;
    wget)
        _location=$(wget $WGET_FLAGS --max-redirect=0 -S --spider \
            -U "$USER_AGENT" "$RELEASES_URL/latest" 2>&1 |
            tr -d '\015' | sed -n 's/^ *[Ll]ocation: *//p' | tail -n 1 || true)
        ;;
    esac

    # The class has to exclude whitespace: GNU wget writes the header as
    # "Location: <url> [following]", and everything after the tag would
    # otherwise be swallowed into it.
    printf '%s\n' "$_location" |
        sed -n 's#.*/releases/tag/\([^/?#[:space:]]*\).*#\1#p'
}

tags_via_atom() {
    # Served by github.com itself, so it carries no API rate limit.
    fetch_to_stdout "$RELEASES_URL.atom" 2>/dev/null |
        tr '<' '\012' |
        sed -n 's#.*/releases/tag/\([^"]*\)".*#\1#p' || true
}

tags_via_api() {
    fetch_to_stdout "https://api.github.com/repos/$REPO/releases?per_page=10" 2>/dev/null |
        tr ',' '\012' |
        sed -n 's/.*"tag_name": *"\([^"]*\)".*/\1/p' || true
}

candidate_tags() {
    _tags=$({
        latest_tag_via_redirect
        tags_via_atom
    } 2>/dev/null || true)

    if [ -z "$(printf '%s' "$_tags" | tr -d ' \011\012')" ]; then
        _tags=$(tags_via_api || true)
    fi

    # Nightlies name their assets after the plain version (salam-0.2.9-linux.zip
    # under a v0.2.9-nightly-<date> tag), so a name built from the tag can
    # never match one - skipping them here avoids a guaranteed 404.
    printf '%s\n' "$_tags" |
        tr -d ' \011\015' |
        grep -v 'nightly' |
        grep '.' |
        awk '!seen[$0]++' |
        head -n "$MAX_TAGS" || true
}

asset_url() {
    printf '%s/download/v%s/salam-%s-%s.zip' "$RELEASES_URL" "$1" "$1" "$2"
}

# ---------------------------------------------------------------------
# Header
# ---------------------------------------------------------------------

printf '\n' >&2
printf '  %sSalam Programming Language installer%s\n' "$C_GOOD" "$C_OFF" >&2
printf '  https://github.com/%s\n' "$REPO" >&2
printf '  ------------------------------------\n' >&2

# ---------------------------------------------------------------------
# 1. This machine
# ---------------------------------------------------------------------

step "Checking this machine"

if [ -z "$DL_TOOL" ]; then
    die "neither curl nor wget is installed, and the installer needs one of them" \
        "  Debian/Ubuntu : apt install curl" \
        "  Fedora/RHEL   : dnf install curl" \
        "  Alpine        : apk add curl" \
        "  FreeBSD       : pkg install curl" \
        "  macOS         : curl is already there; check your PATH" \
        "or download a release archive by hand from $RELEASES_URL"
fi

OS_NAME=$(uname -s 2>/dev/null || printf 'unknown')
ARCH=$(uname -m 2>/dev/null || printf 'unknown')
OS_LABEL="$OS_NAME"
LIBC=""

case "$OS_NAME" in
Linux)
    KERNEL="linux"

    if [ -r /etc/os-release ]; then
        OS_LABEL=$(
            . /etc/os-release 2>/dev/null || true
            printf '%s' "${PRETTY_NAME:-${NAME:-Linux}}"
        )
    elif [ -r /etc/alpine-release ]; then
        OS_LABEL="Alpine Linux $(cat /etc/alpine-release 2>/dev/null || true)"
    fi

    if [ -z "$OS_LABEL" ]; then
        OS_LABEL="Linux"
    fi

    # Which libc the machine actually runs decides whether the published,
    # glibc-linked builds can start at all. Ask the dynamic loader
    # rather than looking for files: a glibc box with musl-tools
    # installed has an ld-musl-* sitting right there, and is still glibc.
    LIBC=""

    if command -v ldd >/dev/null 2>&1; then
        case "$(ldd --version 2>&1 | head -n 1 || true)" in
        *musl*) LIBC="musl" ;;
        *GLIBC* | *GNU* | *glibc*) LIBC="glibc" ;;
        esac
    fi

    if [ -z "$LIBC" ] && [ -r /etc/alpine-release ]; then
        LIBC="musl"
    fi

    if [ -z "$LIBC" ]; then
        for _loader in /lib64/ld-linux-*.so.* /lib/ld-linux*.so.*; do
            if [ -e "$_loader" ]; then
                LIBC="glibc"
                break
            fi
        done
    fi

    if [ -z "$LIBC" ]; then
        for _loader in /lib/ld-musl-*.so.*; do
            if [ -e "$_loader" ]; then
                LIBC="musl"
                break
            fi
        done
    fi

    case "$(uname -o 2>/dev/null || true)" in
    Android) LIBC="bionic" ;;
    esac

    if [ -n "${TERMUX_VERSION:-}" ]; then
        LIBC="bionic"
        OS_LABEL="Android (Termux ${TERMUX_VERSION})"
    fi

    if [ -z "$LIBC" ]; then
        LIBC="unknown"
    fi
    ;;
Darwin)
    KERNEL="mac"

    if command -v sw_vers >/dev/null 2>&1; then
        OS_LABEL="macOS $(sw_vers -productVersion 2>/dev/null || true)"
    else
        OS_LABEL="macOS $(uname -r 2>/dev/null || true)"
    fi
    ;;
FreeBSD)
    KERNEL="bsd"
    BSD_KIND="freebsd"

    if command -v freebsd-version >/dev/null 2>&1; then
        OS_LABEL="FreeBSD $(freebsd-version 2>/dev/null || true)"
    else
        OS_LABEL="FreeBSD $(uname -r 2>/dev/null || true)"
    fi
    ;;
OpenBSD | NetBSD | DragonFly)
    KERNEL="bsd"
    BSD_KIND=$(printf '%s' "$OS_NAME" | tr 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' 'abcdefghijklmnopqrstuvwxyz')
    OS_LABEL="$OS_NAME $(uname -r 2>/dev/null || true)"
    ;;
SunOS | AIX | Haiku | GNU | *)
    case "$OS_NAME" in
    MINGW* | MSYS* | CYGWIN* | Windows_NT)
        die "this is a Windows shell, and Windows has its own installer" \
            "run install.bat, or from PowerShell:" \
            "  iwr -useb $RAW_URL/install.ps1 | iex"
        ;;
    esac

    KERNEL="other"
    OS_LABEL="$OS_NAME $(uname -r 2>/dev/null || true)"
    ;;
esac

# Asset names to try, best first. They are probed against the real
# release, so a build published later - freebsd, mac-aarch64 - starts
# working here with no change to this script.
if [ -n "$PLATFORM" ]; then
    PLATFORMS="$PLATFORM"
else
    case "$KERNEL" in
    linux)
        case "$ARCH" in
        x86_64 | amd64) PLATFORMS="linux linux-x86_64" ;;
        i386 | i486 | i586 | i686 | x86) PLATFORMS="linux-i686" ;;
        aarch64 | arm64 | armv8*) PLATFORMS="linux-aarch64 linux-arm64" ;;
        armv7* | armv6* | arm) PLATFORMS="linux-armhf linux-arm" ;;
        riscv64) PLATFORMS="linux-riscv64" ;;
        ppc64le) PLATFORMS="linux-ppc64le" ;;
        s390x) PLATFORMS="linux-s390x" ;;
        *) PLATFORMS="" ;;
        esac
        ;;
    mac)
        case "$ARCH" in
        arm64 | aarch64) PLATFORMS="mac mac-aarch64 mac-arm64" ;;
        *) PLATFORMS="mac mac-x86_64" ;;
        esac
        ;;
    bsd)
        case "$ARCH" in
        amd64 | x86_64) PLATFORMS="$BSD_KIND $BSD_KIND-x86_64" ;;
        aarch64 | arm64) PLATFORMS="$BSD_KIND-aarch64 $BSD_KIND-arm64" ;;
        i386 | i686) PLATFORMS="$BSD_KIND-i686 $BSD_KIND-i386" ;;
        *) PLATFORMS="$BSD_KIND" ;;
        esac
        ;;
    *)
        PLATFORMS=""
        ;;
    esac
fi

field "system" "$OS_LABEL"
field "machine" "$ARCH"

if [ -n "$LIBC" ]; then
    field "libc" "$LIBC"
fi

field "downloader" "$DL_TOOL"

if [ -z "$PLATFORMS" ]; then
    die "no Salam build is published for $OS_NAME on $ARCH" \
        "if a build exists under another name, point at it directly:" \
        "  install.sh --platform NAME" \
        "otherwise please open an issue so we know it is wanted: $ISSUES_URL"
fi

field "asset" "$(printf '%s' "$PLATFORMS" | cut -d' ' -f1)"

if [ "$KERNEL" = "bsd" ] && [ "$PLATFORM_FORCED" = 0 ]; then
    warn "no native $OS_NAME build is published yet"
    info "checking anyway, in case one has been added since"
fi

if [ "$KERNEL" = "linux" ] && [ "$LIBC" = "musl" ]; then
    warn "the published Linux builds link against glibc, and this system uses musl"
    info "if salam will not start, install a glibc shim (apk add gcompat) or"
    info "use the Docker image: docker run --rm -it salamlang/salam"
fi

if [ "$KERNEL" = "linux" ] && [ "$LIBC" = "bionic" ]; then
    warn "this looks like Android/Termux, whose libc is bionic, not glibc"
    info "the published Linux builds may refuse to start here"
fi

if [ -z "$INSTALL_DIR" ]; then
    INSTALL_DIR="$HOME/.salam/bin"
fi

# Relative paths resolve against the caller's directory, which is what
# someone typing --dir ./bin means.
TILDE='~'

# Two passes, because a single one cannot rewrite the overlapping
# components of a path like /a/././b.
tidy_path() {
    printf '%s' "$1" | sed \
        -e 's#//*#/#g' -e 's#/\./#/#g' \
        -e 's#//*#/#g' -e 's#/\./#/#g' \
        -e 's#/\.$##' -e 's#\(.\)/*$#\1#'
}

case "$INSTALL_DIR" in
/*) ;;
"$TILDE") INSTALL_DIR="$HOME" ;;
"$TILDE"/*) INSTALL_DIR="$HOME/${INSTALL_DIR#"$TILDE"/}" ;;
*) INSTALL_DIR="$PWD/$INSTALL_DIR" ;;
esac

INSTALL_DIR=$(tidy_path "$INSTALL_DIR")
DEFAULT_DIR=$(tidy_path "$HOME/.salam/bin")

field "install to" "$INSTALL_DIR"

PREVIOUS=""

if [ -x "$INSTALL_DIR/salam" ]; then
    PREVIOUS=$("$INSTALL_DIR/salam" version --short 2>/dev/null | head -n 1 | tr -d '\015' || true)
fi

if [ -n "$PREVIOUS" ]; then
    field "installed" "$PREVIOUS (will be replaced)"
fi

if [ "$IS_TTY" = 1 ]; then
    pick_nap
fi

WORKDIR=$(mktemp -d 2>/dev/null || mktemp -d -t salam 2>/dev/null || true)

if [ -z "$WORKDIR" ]; then
    # Solaris 10 and a few embedded userlands have no usable mktemp.
    WORKDIR="${TMPDIR:-/tmp}/salam-install-$$"
    mkdir -p "$WORKDIR" || die "could not create a temporary directory in ${TMPDIR:-/tmp}"
fi

ARCHIVE="$WORKDIR/salam.zip"
EXTRACT_DIR="$WORKDIR/extracted"

# ---------------------------------------------------------------------
# 2. Which release
# ---------------------------------------------------------------------

step "Choosing the release to install"

TAG=""
ASSET=""
URL=""
TOTAL=0
PROBE_BLOCKED=0

try_platforms() {
    # $1 = version without the leading v. Sets URL/ASSET/TOTAL on a hit.
    for _plat in $PLATFORMS; do
        _try=$(asset_url "$1" "$_plat")

        if probe_url "$_try"; then
            PLATFORM="$_plat"
            ASSET="salam-$1-$_plat.zip"
            URL="$_try"
            TOTAL=$PROBE_SIZE
            return 0
        else
            # Reading $? here, as the first command of the else branch,
            # still gets the condition's own status.
            if [ "$?" = 2 ]; then
                PROBE_BLOCKED=1
            fi
        fi
    done

    return 1
}

if [ -n "$VERSION" ]; then
    info "requested version: $VERSION"

    if ! try_platforms "$VERSION"; then
        # A downloader without a HEAD mode, or a proxy that refuses one,
        # looks exactly like a missing asset. Let the GET decide.
        PLATFORM=$(printf '%s' "$PLATFORMS" | cut -d' ' -f1)
        ASSET="salam-$VERSION-$PLATFORM.zip"
        URL=$(asset_url "$VERSION" "$PLATFORM")
        TOTAL=0
    fi

    TAG="v$VERSION"
else
    info "asking GitHub for the newest release with a matching asset"

    TAGS=$(candidate_tags)

    if [ -z "$TAGS" ]; then
        die "could not reach GitHub to list releases" \
            "check your internet connection and any proxy settings," \
            "or pass --version X.Y.Z to skip the lookup entirely"
    fi

    SHOWN=$(printf '%s\n' "$TAGS" | head -n 4 | tr '\012' ' ')

    if [ "$(printf '%s\n' "$TAGS" | wc -l | tr -d ' \011\012')" -gt 4 ]; then
        SHOWN="$SHOWN..."
    fi

    info "recent releases: $SHOWN"

    for _tag in $TAGS; do
        _version="${_tag#v}"

        status "      checking $_tag..."

        if try_platforms "$_version"; then
            status_end
            info "$_tag publishes $ASSET"

            TAG="$_tag"
            VERSION="$_version"
            break
        fi

        status_end

        if [ "$PROBE_BLOCKED" = 1 ]; then
            break
        fi

        info "$_tag has nothing for this platform, trying the one before it"
    done

    if [ -z "$URL" ] && [ "$PLATFORM_FORCED" = 0 ] && [ "$PROBE_BLOCKED" = 0 ]; then
        if [ "$KERNEL" = "bsd" ] || [ "$KERNEL" = "other" ]; then
            NO_BUILD="no release publishes a build for $OS_NAME"
            PUBLISHED="Salam publishes Linux, macOS and Windows builds today."

            case "${BSD_KIND:-}" in
            freebsd)
                die "$NO_BUILD" \
                    "$PUBLISHED" \
                    "" \
                    "FreeBSD can run the Linux build through its compatibility layer:" \
                    "  1. enable it once, as root:  sysrc linux_enable=YES && service linux start" \
                    "  2. add a Linux userland:     pkg install linux_base-rl9  (or linux_base-c7)" \
                    "  3. run this installer again: sh install.sh --platform linux" \
                    "" \
                    "Want a native build? Say so at $ISSUES_URL"
                ;;
            netbsd | dragonfly)
                die "$NO_BUILD" \
                    "$PUBLISHED" \
                    "" \
                    "With COMPAT_LINUX in your kernel and a Linux userland installed," \
                    "the Linux build works here: sh install.sh --platform linux" \
                    "" \
                    "Want a native build? Say so at $ISSUES_URL"
                ;;
            openbsd)
                die "$NO_BUILD" \
                    "$PUBLISHED" \
                    "" \
                    "OpenBSD dropped Linux emulation in 5.8, so there is nothing to" \
                    "fall back to here - a native build is the only way." \
                    "" \
                    "Want one? Say so at $ISSUES_URL"
                ;;
            *)
                die "$NO_BUILD" \
                    "$PUBLISHED" \
                    "if this system can run Linux binaries: sh install.sh --platform linux" \
                    "otherwise please open an issue so we know it is wanted: $ISSUES_URL"
                ;;
            esac
        fi
    fi

    if [ -z "$URL" ]; then
        # Header requests get refused by some proxies, and ftp(1) cannot
        # make one at all - both look exactly like a missing asset. Give
        # the newest tag one honest GET before giving up.
        TAG=$(printf '%s\n' "$TAGS" | head -n 1)
        VERSION="${TAG#v}"
        PLATFORM=$(printf '%s' "$PLATFORMS" | cut -d' ' -f1)
        ASSET="salam-$VERSION-$PLATFORM.zip"
        URL=$(asset_url "$VERSION" "$PLATFORM")
        TOTAL=0

        if [ "$PROBE_BLOCKED" = 1 ]; then
            info "$DL_TOOL cannot ask for headers; trying $TAG directly"
        else
            info "no recent release advertised one; trying $TAG directly anyway"
        fi
    fi
fi

# ---------------------------------------------------------------------
# 3. Download
# ---------------------------------------------------------------------

step "Downloading $ASSET"

info "$URL"

if [ "$TOTAL" -gt 0 ]; then
    info "size: $(fmt_size "$TOTAL")"
fi

if ! download_file "$URL" "$ARCHIVE" "$TOTAL"; then
    die "could not download $ASSET" \
        "$URL" \
        "either that release publishes no $PLATFORM asset, or the transfer was refused" \
        "browse what is available at $RELEASES_URL"
fi

SIZE=$(file_size "$ARCHIVE")

if [ "$SIZE" -lt 1024 ]; then
    die "the download produced only $(fmt_size "$SIZE"), which is no release archive" \
        "try again, or download it by hand from $RELEASES_URL"
fi

# "PK". An HTML error page saved under a .zip name fails here instead of
# turning into a baffling extraction error later.
MAGIC=$(dd if="$ARCHIVE" bs=1 count=2 2>/dev/null || true)

if [ "$MAGIC" != "PK" ]; then
    die "the downloaded file is not a zip archive" \
        "something between here and GitHub replaced it - usually a proxy or a captive portal" \
        "download it by hand from $RELEASES_URL"
fi

# ---------------------------------------------------------------------
# 4. Extract
# ---------------------------------------------------------------------

step "Extracting $ASSET"

info "archive size: $(fmt_size "$SIZE")"

mkdir -p "$EXTRACT_DIR"

UNPACK=""

for _tool in unzip bsdtar tar python3 python; do
    if command -v "$_tool" >/dev/null 2>&1; then
        # BSD tar reads zip archives; GNU tar does not, and answers with
        # "This does not look like a tar archive" if handed one.
        if [ "$_tool" = "tar" ]; then
            case "$(tar --version 2>&1 | head -n 1 || true)" in
            *bsdtar* | *libarchive*) ;;
            *) continue ;;
            esac
        fi

        UNPACK="$_tool"
        break
    fi
done

if [ -z "$UNPACK" ]; then
    die "no tool on this machine can unpack a zip archive" \
        "  Debian/Ubuntu : apt install unzip" \
        "  Fedora/RHEL   : dnf install unzip" \
        "  Alpine        : apk add unzip" \
        "  FreeBSD       : pkg install unzip   (or use the base bsdtar)" \
        "the archive is a plain zip and is waiting at $RELEASES_URL"
fi

info "using $UNPACK"

extract_archive() {
    case "$UNPACK" in
    unzip) unzip -q -o "$ARCHIVE" -d "$EXTRACT_DIR" ;;
    bsdtar | tar) "$UNPACK" -xf "$ARCHIVE" -C "$EXTRACT_DIR" ;;
    *) "$UNPACK" -c 'import sys,zipfile; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])' "$ARCHIVE" "$EXTRACT_DIR" ;;
    esac
}

extract_archive >/dev/null 2>&1 &
EXTRACT_PID=$!

if ! spin_wait "$EXTRACT_PID" "unpacking" "$EXTRACT_DIR"; then
    die "could not unpack $ASSET with $UNPACK" \
        "the download may have been truncated; run the installer again"
fi

info "unpacked $(count_files "$EXTRACT_DIR") files"

BUNDLE="$EXTRACT_DIR/salam-$PLATFORM"

if [ ! -f "$BUNDLE/salam" ]; then
    FOUND=$(find "$EXTRACT_DIR" -type f -name salam 2>/dev/null | head -n 1 || true)

    if [ -z "$FOUND" ]; then
        die "no 'salam' binary inside $ASSET" \
            "this release archive looks broken; please report it at $ISSUES_URL"
    fi

    BUNDLE=$(dirname "$FOUND")
fi

# ---------------------------------------------------------------------
# 5. Install
# ---------------------------------------------------------------------

step "Installing into $INSTALL_DIR"

if ! mkdir -p "$INSTALL_DIR" 2>/dev/null; then
    die "cannot create $INSTALL_DIR" \
        "choose a directory you own with --dir DIR, or re-run this under sudo"
fi

if [ ! -w "$INSTALL_DIR" ]; then
    die "cannot write to $INSTALL_DIR" \
        "choose a directory you own with --dir DIR, or re-run this under sudo"
fi

# Everything ships side by side on purpose: salam finds its std/ library
# relative to its own path, so installing the bare binary leaves every
# "import os" failing with "standard library package not found". Copying
# the whole bundle also keeps this working when a release starts
# shipping something new next to the compiler.
for _entry in "$BUNDLE"/*; do
    if [ ! -e "$_entry" ]; then
        continue
    fi

    _name=$(basename "$_entry")
    _target="$INSTALL_DIR/$_name"

    if [ -d "$_entry" ]; then
        info "copying $_name/"

        # Replace wholesale: an upgrade must not leave a previous
        # version's std/ modules lying around.
        rm -rf "$_target"

        if ! cp -R "$_entry" "$_target" 2>/dev/null; then
            warn "could not copy $_name/"
        fi
    else
        info "copying $_name"

        # Unlink first: a running program keeps its executable busy, and
        # overwriting it in place fails with ETXTBSY.
        rm -f "$_target"

        if ! cp "$_entry" "$_target" 2>/dev/null; then
            if [ "$_name" = "salam" ]; then
                die "could not write $_target" \
                    "close anything still running salam and try again"
            fi

            warn "could not copy $_name"
        fi
    fi
done

chmod +x "$INSTALL_DIR/salam" 2>/dev/null || true

# Gatekeeper quarantines anything a browser or curl brought in, and the
# dialog it shows says "cannot be opened because the developer cannot be
# verified" rather than anything about quarantine.
if [ "$KERNEL" = "mac" ] && command -v xattr >/dev/null 2>&1; then
    xattr -dr com.apple.quarantine "$INSTALL_DIR" >/dev/null 2>&1 || true
fi

if [ -d "$INSTALL_DIR/std" ]; then
    info "standard library: $INSTALL_DIR/std"
else
    warn "no std/ directory in this release - imports will not resolve"
fi

INSTALLED_SIZE=$(du -sh "$INSTALL_DIR" 2>/dev/null | cut -f1 || true)

if [ -n "$INSTALLED_SIZE" ]; then
    info "installed size: $INSTALLED_SIZE"
fi

# ---------------------------------------------------------------------
# 6. PATH
# ---------------------------------------------------------------------

on_path() {
    case ":$PATH:" in
    *":$1:"*) return 0 ;;
    esac

    return 1
}

PATH_CHANGED=0
PATH_RC=""
PATH_LINE="export PATH=\"$INSTALL_DIR:\$PATH\""

if [ "$MODIFY_PATH" != 1 ]; then
    step "Leaving your PATH alone"
    info "--no-modify-path was given"
    info "run salam as $INSTALL_DIR/salam, or add that directory to PATH yourself"
elif on_path "$INSTALL_DIR"; then
    step "Checking your PATH"
    info "$INSTALL_DIR is already on it"
elif [ "$INSTALL_DIR" != "$DEFAULT_DIR" ]; then
    step "Leaving your PATH alone"
    info "PATH is only updated for the default directory, $DEFAULT_DIR"
    info "to use salam by name from here, add this to your shell startup file:"
    info "  $PATH_LINE"
else
    step "Updating your PATH"

    case "${SHELL:-}" in
    */zsh)
        PATH_RC="${ZDOTDIR:-$HOME}/.zshrc"
        ;;
    */bash)
        # macOS Terminal starts login shells, which read .bash_profile
        # and never .bashrc; most Linux terminals do the opposite.
        if [ -f "$HOME/.bash_profile" ]; then
            PATH_RC="$HOME/.bash_profile"
        elif [ "$KERNEL" = "mac" ]; then
            PATH_RC="$HOME/.bash_profile"
        else
            PATH_RC="$HOME/.bashrc"
        fi
        ;;
    */fish)
        PATH_RC="${XDG_CONFIG_HOME:-$HOME/.config}/fish/config.fish"
        PATH_LINE="fish_add_path \"$INSTALL_DIR\""
        mkdir -p "$(dirname "$PATH_RC")" 2>/dev/null || true
        ;;
    */ksh | */ksh93 | */mksh | */pdksh)
        PATH_RC="$HOME/.kshrc"
        ;;
    */tcsh | */csh)
        PATH_RC="$HOME/.cshrc"
        PATH_LINE="setenv PATH \"$INSTALL_DIR:\$PATH\""
        ;;
    *)
        PATH_RC="$HOME/.profile"
        ;;
    esac

    if [ -f "$PATH_RC" ] && grep -F "$INSTALL_DIR" "$PATH_RC" >/dev/null 2>&1; then
        info "$PATH_RC already mentions $INSTALL_DIR"
        PATH_CHANGED=1
    elif printf '\n# added by the Salam installer\n%s\n' "$PATH_LINE" >>"$PATH_RC" 2>/dev/null; then
        info "added it to $PATH_RC:"
        info "  $PATH_LINE"
        PATH_CHANGED=1
    else
        warn "could not write to $PATH_RC"
        info "add this line to your shell startup file by hand:"
        info "  $PATH_LINE"
    fi
fi

SHADOW=$(command -v salam 2>/dev/null || true)

if [ -n "$SHADOW" ] && [ "$SHADOW" != "$INSTALL_DIR/salam" ]; then
    warn "another salam sits earlier on your PATH: $SHADOW"
    info "that one keeps winning until you remove it or reorder PATH"
fi

# ---------------------------------------------------------------------
# 7. Verify
# ---------------------------------------------------------------------

step "Verifying the installation"

if [ ! -x "$INSTALL_DIR/salam" ]; then
    die "salam is not executable at $INSTALL_DIR/salam after installing"
fi

info "$INSTALL_DIR/salam"

VERIFY_OK=1

if VERIFY_OUT=$("$INSTALL_DIR/salam" version 2>&1); then
    printf '%s\n' "$VERIFY_OUT" | sed 's/^/      /' >&2
else
    VERIFY_OK=0
    warn "the installed binary did not run"
    printf '%s\n' "$VERIFY_OUT" | head -n 5 | sed 's/^/      /' >&2

    MISSING_LIB=$(printf '%s\n' "$VERIFY_OUT" |
        sed -n 's/.*error while loading shared libraries: \([^:]*\):.*/\1/p' |
        head -n 1)

    case "$VERIFY_OUT" in
    *"cannot execute binary file"* | *"Exec format error"*)
        info "this archive is built for a different architecture than $ARCH"
        info "pick another one with --platform NAME; see $RELEASES_URL"
        ;;
    *"Permission denied"*)
        info "the filesystem may be mounted noexec; install elsewhere with --dir DIR"
        ;;
    *)
        if [ "$LIBC" = "musl" ] || [ "$LIBC" = "bionic" ]; then
            info "these builds are linked against glibc, and this system uses $LIBC"
            info "on Alpine a shim usually does it: apk add gcompat"
        elif [ -n "$MISSING_LIB" ]; then
            info "one shared library is missing: $MISSING_LIB"

            case "$MISSING_LIB" in
            libxml2*)
                info "  Debian/Ubuntu : apt install libxml2"
                info "  Fedora/RHEL   : dnf install libxml2"
                info "  Arch          : pacman -S libxml2"
                ;;
            libtinfo* | libncurses*)
                info "  Debian/Ubuntu : apt install libtinfo6"
                info "  Fedora/RHEL   : dnf install ncurses-libs"
                info "  Arch          : pacman -S ncurses"
                ;;
            *)
                info "install it with your package manager, then run salam again"
                ;;
            esac
        else
            info "please report this at $ISSUES_URL, with the message above"
        fi
        ;;
    esac
fi

# ---------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------

SALAM_CMD="salam"

if [ "$PATH_CHANGED" = 0 ] && ! on_path "$INSTALL_DIR"; then
    # Nothing put this directory on PATH, so the bare name would not
    # find it - show the path that does.
    SALAM_CMD="$INSTALL_DIR/salam"
fi

printf '\n' >&2

if [ "$VERIFY_OK" = 1 ]; then
    printf '  %sSalam %s is installed.%s\n' "$C_GOOD" "$VERSION" "$C_OFF" >&2
else
    printf '  %sSalam %s is in place, but it did not run here.%s\n' \
        "$C_WARN" "$VERSION" "$C_OFF" >&2
fi

printf '\n' >&2

if [ "$VERIFY_OK" = 1 ] && [ "$PATH_CHANGED" = 1 ] && ! on_path "$INSTALL_DIR"; then
    printf '  Start a new shell (or run %ssource %s%s), then try:\n' \
        "$C_BOLD" "$PATH_RC" "$C_OFF" >&2
elif [ "$VERIFY_OK" = 1 ]; then
    printf '  Try it:\n' >&2
else
    printf '  Once that is sorted:\n' >&2
fi

printf '\n' >&2
printf '      %s version\n' "$SALAM_CMD" >&2
printf '      %s new hello\n' "$SALAM_CMD" >&2
printf '      %s run hello/salam.salam\n' "$SALAM_CMD" >&2
printf '\n' >&2
printf '  Docs and examples: https://github.com/%s\n' "$REPO" >&2
printf '\n' >&2

exit 0

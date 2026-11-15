#!/bin/sh
# Publishes both language sites to the Salam server in one run.
#
#   sh website/deploy.sh                 # build HEAD, publish fa + en
#   sh website/deploy.sh --ref 1c022491d # publish an older commit
#   sh website/deploy.sh --dry-run       # build only, upload nothing
#
# It exports the ref (the working tree is never touched), builds both languages
# the way the CI build job does, gives the English tree site.base=/en, then
# mirrors
#
#   dist/fa -> /hosts/salamlang.ir     https://www.salamlang.ir/
#   dist/en -> /hosts/salamlang.ir-en  https://www.salamlang.ir/en/
#
# and finally checks that both URLs answer. The /en/ nginx location is added on
# the server the first time it is missing, so a fresh server only needs the
# host, the user and the password.
#
# Settings come from the environment, or from website/.deploy.env (git-ignored,
# see the file for the keys); a variable given on the command line wins.
#
#   SITE_HOST       server to deploy to            (default 85.17.142.241)
#   SITE_USER       ssh user                       (default root)
#   SITE_PORT       ssh port                       (default 22)
#   SITE_PASS       ssh password; empty uses an ssh key
#   SITE_FA_PATH    server directory for Persian   (default /hosts/salamlang.ir)
#   SITE_EN_PATH    server directory for English   (default /hosts/salamlang.ir-en)
#   SITE_FA_URL     public URL of the Persian site (default https://www.salamlang.ir)
#   SITE_EN_URL     public URL of the English site (default https://www.salamlang.ir/en)
#   SALAM           compiler used to build         (default salam on PATH, then ./salam)
#   SITE_SKIP_CHECKS=1  publish even when a content gate fails
#
# Examples:
#   sh website/deploy.sh                       # the everyday command
#   SITE_PASS=... sh website/deploy.sh         # no .deploy.env yet
#   sh website/deploy.sh --skip-checks         # while a gate is red on main

set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
ref=HEAD
dry_run=0
verify=1

usage() {
    sed -n '2,40p' "$0" | sed 's/^# \{0,1\}//'
    exit "${1:-0}"
}

while [ $# -gt 0 ]; do
    case $1 in
    --ref=*) ref=${1#*=} ;;
    --ref)
        [ $# -ge 2 ] || {
            echo "deploy: --ref needs a value" >&2
            exit 2
        }
        ref=$2
        shift
        ;;
    --dry-run) dry_run=1 ;;
    --skip-checks) SITE_SKIP_CHECKS=1 ;;
    --no-verify) verify=0 ;;
    -h | --help) usage 0 ;;
    *)
        echo "deploy: unknown argument '$1' (try --help)" >&2
        exit 2
        ;;
    esac
    shift
done

# A variable given on the command line must survive sourcing the env file.
want_host=${SITE_HOST-}
want_user=${SITE_USER-}
want_port=${SITE_PORT-}
want_pass=${SITE_PASS-}
env_file=${SITE_ENV_FILE:-$root/website/.deploy.env}
if [ -f "$env_file" ]; then
    set -a
    # shellcheck disable=SC1090 # the path is configurable on purpose
    . "$env_file"
    set +a
fi
SITE_HOST=${want_host:-${SITE_HOST:-85.17.142.241}}
SITE_USER=${want_user:-${SITE_USER:-root}}
SITE_PORT=${want_port:-${SITE_PORT:-22}}
SITE_PASS=${want_pass:-${SITE_PASS:-}}
SITE_FA_PATH=${SITE_FA_PATH:-/hosts/salamlang.ir}
SITE_EN_PATH=${SITE_EN_PATH:-/hosts/salamlang.ir-en}
SITE_FA_URL=${SITE_FA_URL:-https://www.salamlang.ir}
SITE_EN_URL=${SITE_EN_URL:-https://www.salamlang.ir/en}

die() {
    echo "deploy: $1" >&2
    exit 1
}

say() {
    printf '==> %s\n' "$1"
}

# ---------------------------------------------------------------- build ----
command -v git >/dev/null 2>&1 || die "git is not on PATH"
git -C "$root" rev-parse --git-dir >/dev/null 2>&1 || die "$root is not a git checkout"
have_ref=$(git -C "$root" rev-parse --verify "$ref^{commit}" 2>/dev/null) || die "no commit for ref '$ref'"

if [ -z "${SALAM-}" ]; then
    if command -v salam >/dev/null 2>&1; then
        SALAM=$(command -v salam)
    elif [ -x "$root/salam" ]; then
        SALAM=$root/salam
    fi
fi
[ -n "${SALAM-}" ] || die "no compiler: install salam, or set SALAM=/path/to/salam"
case $SALAM in
/*) ;;
*) SALAM=$(cd "$(dirname "$SALAM")" && pwd)/$(basename "$SALAM") ;;
esac
[ -x "$SALAM" ] || die "'$SALAM' is not executable"

work=$(mktemp -d "${TMPDIR:-/tmp}/salam-deploy.XXXXXX")
trap 'rm -rf "$work"' EXIT HUP INT TERM

say "exporting $ref ($(git -C "$root" log -1 --format='%h %s' "$have_ref"))"
git -C "$root" archive "$have_ref" | tar -x -C "$work"

# The generator keeps the API JSON and the page previews here; keep it between
# runs so a deploy only renders pages instead of documenting every package.
mkdir -p "$root/website/.cache"
ln -s "$root/website/.cache" "$work/website/.cache"

# The English site is published under /en/ on the same host, which is the
# sibling layout the generator already assumes for hreflang links.
en_site=$work/website/content/en/site.txt
sed -e 's|^site\.base = /Salam$|site.base = /en|' \
    -e 's|^site\.url = https://salamlang\.github\.io/Salam$|site.url = https://www.salamlang.ir/en|' \
    "$en_site" >"$work/en-site.txt"
mv "$work/en-site.txt" "$en_site"
if ! grep -qx 'site.base = /en' "$en_site"; then
    die "website/content/en/site.txt no longer sets 'site.base = /Salam'; update website/deploy.sh"
fi

skip_checks=${SITE_SKIP_CHECKS:-0}
if [ "$skip_checks" = 1 ]; then
    say "skipping the Persian-name content gate (SITE_SKIP_CHECKS=1)"
    sed '/check-fa-names\.salam/d' "$work/website/build.sh" >"$work/build.sh"
    chmod +x "$work/build.sh"
    mv "$work/build.sh" "$work/website/build.sh"
fi

say "building both languages with $SALAM ($("$SALAM" version --short 2>/dev/null || echo unknown))"
build_log=$work/build.log
build_rc=0
(cd "$work" && WEBSITE_EXAMPLES=${WEBSITE_EXAMPLES:-skip} SALAM="$SALAM" sh website/build.sh) >"$build_log" 2>&1 || build_rc=$?

fa_tree=$work/website/dist/fa
en_tree=$work/website/dist/en
if [ "$build_rc" -ne 0 ]; then
    if [ "$skip_checks" = 1 ] && [ -f "$fa_tree/index.html" ] && [ -f "$en_tree/index.html" ]; then
        echo "deploy: the build exited $build_rc after rendering; publishing anyway (SITE_SKIP_CHECKS=1)" >&2
        tail -4 "$build_log" >&2
    else
        tail -40 "$build_log" >&2
        die "the site build failed; fix it, or pass --skip-checks to publish what was rendered"
    fi
fi
grep -E '^built [0-9]+ pages' "$build_log" || die "the build produced no page counts"
[ -f "$fa_tree/index.html" ] || die "no Persian tree at $fa_tree"
[ -f "$en_tree/index.html" ] || die "no English tree at $en_tree"
grep -q '"/en/assets/' "$en_tree/index.html" ||
    die "the English tree was not built for /en/; check content/en/site.txt"

if [ "$dry_run" = 1 ]; then
    say "dry run: would mirror $fa_tree -> $SITE_FA_PATH and $en_tree -> $SITE_EN_PATH on $SITE_USER@$SITE_HOST"
    exit 0
fi

# ------------------------------------------------------------ transport ----
if [ -n "$SITE_PASS" ]; then
    command -v sshpass >/dev/null 2>&1 || die "a password is set but sshpass is not installed (brew install sshpass)"
    SSHPASS=$SITE_PASS
    export SSHPASS
    ssh_prefix="sshpass -e"
else
    ssh_prefix=""
fi
remote=$SITE_USER@$SITE_HOST
ssh_opts="-p $SITE_PORT -o StrictHostKeyChecking=accept-new -o ServerAliveInterval=15 -o ConnectTimeout=20"
[ -n "$SITE_PASS" ] || ssh_opts="$ssh_opts -o BatchMode=yes"

run_ssh() {
    # $1 is a command string; the rest of the arguments are its arguments.
    # The command is assembled locally on purpose: the remote shell runs it.
    cmd=$1
    shift
    if [ -n "$ssh_prefix" ]; then
        # shellcheck disable=SC2029
        sshpass -e ssh $ssh_opts "$remote" "$cmd" "$@"
    else
        # shellcheck disable=SC2029
        ssh $ssh_opts "$remote" "$cmd" "$@"
    fi
}

say "checking $remote"
run_ssh "command -v rsync >/dev/null" || die "$remote has no rsync"
run_ssh "mkdir -p '$SITE_FA_PATH' '$SITE_EN_PATH'"

# --------------------------------------------------------------- nginx ----
# The Persian vhost also serves /en/. Add the locations when they are missing,
# so a server that has never hosted the English site needs no manual step.
en_conf=/opt/nginx/conf.d/salamlang.ir
en_block=$(cat <<'BLOCK'

    # English site: built with site.base=/en, served from /hosts/salamlang.ir-en.
    location = /en {
        return 301 /en/;
    }

    location ^~ /en/assets/ {
        alias /hosts/salamlang.ir-en/assets/;
        add_header Cache-Control "public, max-age=2592000";
    }

    location ^~ /en/ {
        alias /hosts/salamlang.ir-en/;
        index index.html;
        charset utf-8;
        error_page 404 /en/404.html;
        add_header Cache-Control "no-cache";

        location ~ /\. {
            deny all;
            access_log off;
            log_not_found off;
        }
    }
BLOCK
)
en_block_b64=$(printf '%s' "$en_block" | base64 | tr -d '\n')

if run_ssh "test -f '$en_conf'"; then
    if run_ssh "grep -q '/hosts/salamlang.ir-en' '$en_conf'"; then
        say "nginx already serves /en/"
    else
        say "adding the /en/ location to $en_conf"
        run_ssh "python3 - '$en_block_b64' '$en_conf'" <<'PY'
import base64
import sys

block = base64.b64decode(sys.argv[1]).decode()
path = sys.argv[2]
src = open(path).read()
if "/hosts/salamlang.ir-en" in src:
    print("nginx: already configured")
    raise SystemExit(0)
idx = src.rstrip().rfind("}")
if idx < 0:
    raise SystemExit("nginx: no closing brace in " + path)
open(path + ".bak-deploy", "w").write(src)
open(path, "w").write(src[:idx] + block + src[idx:])
print("nginx: added the /en/ location to " + path)
PY
        run_ssh "docker exec nginx nginx -t 2>&1 | tail -1 && docker exec nginx nginx -s reload" ||
            die "nginx rejected the change on $remote; the backup is $en_conf.bak-deploy"
    fi
else
    say "warning: $en_conf not found on $remote; /en/ needs a location block by hand"
fi

# -------------------------------------------------------------- upload ----
mirror() {
    # $1 local tree, $2 server directory
    say "uploading $2"
    if [ -n "$ssh_prefix" ]; then
        rsync -rlz --delete --delay-updates \
            --exclude='.well-known/' --exclude='.htaccess' \
            -e "sshpass -e ssh $ssh_opts" "$1/" "$remote:$2/"
    else
        rsync -rlz --delete --delay-updates \
            --exclude='.well-known/' --exclude='.htaccess' \
            -e "ssh $ssh_opts" "$1/" "$remote:$2/"
    fi
}

mirror "$fa_tree" "$SITE_FA_PATH"
mirror "$en_tree" "$SITE_EN_PATH"

# -------------------------------------------------------------- verify ----
[ "$verify" = 1 ] || {
    say "done: skipped the URL checks (--no-verify)"
    exit 0
}

fa_host=${SITE_FA_URL#*://}
fa_host=${fa_host%%/*}
status=0
check() {
    # $1 url, $2 expected status, $3 label
    code=$(curl -sS -o /dev/null -w '%{http_code}' --max-time 20 \
        --resolve "$fa_host:443:$SITE_HOST" "$1" 2>/dev/null) || code=000
    if [ "$code" = "$2" ]; then
        printf '    ok   %-38s %s\n' "$3" "$code"
    else
        printf '    FAIL %-38s %s (expected %s)\n' "$3" "$code" "$2"
        status=1
    fi
}

say "checking the live URLs through $SITE_HOST"
check "$SITE_FA_URL/" 200 "persian home"
check "$SITE_EN_URL/" 200 "english home"
check "$SITE_EN_URL/learn/" 200 "english learn index"
check "$SITE_EN_URL/assets/css/site.css" 200 "english css"
check "$SITE_FA_URL/assets/css/site.css" 200 "persian css"
check "$SITE_EN_URL/no-such-page/" 404 "english 404"

[ "$status" = 0 ] || die "the deploy uploaded but a URL check failed"
say "done: $SITE_FA_URL/ and $SITE_EN_URL/ are live"

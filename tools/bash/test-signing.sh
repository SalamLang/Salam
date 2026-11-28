#!/bin/sh
# Tests code signing with stand-in tools on PATH: the argument vectors, that a
# password never reaches an argument list or the log, that file names are not
# read by a shell, and that every failure fails the command. No real signing
# tool, certificate or network is used.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
unset SALAM_SIGN_PASSWORD SALAM_SIGN_PASSWORD_FILE TEMP TMP
mkdir "$work/tmp" "$work/fake" "$work/empty" "$work/app"
export FAKE_LOG="$work/calls"
cd "$work/app" || exit 1

cat >"$work/fake/lib" <<'END_LIB'
le32() {
    for shift in 0 8 16 24; do
        printf "\\$(printf %o $((($1 >> shift) & 255)))"
    done
}
sign_pe() {
    pad=$(((8 - $(wc -c <"$1") % 8) % 8))
    head -c "$pad" /dev/zero >>"$1"
    at=$(($(wc -c <"$1")))
    printf 'FAKECERTTABLE...' >>"$1"
    pe=$(od -An -tu4 -j60 -N4 "$1" | tr -d ' ')
    {
        le32 "$at"
        le32 16
    } | dd of="$1" bs=1 seek=$((pe + 168)) conv=notrunc 2>/dev/null
}
record() {
    printf '%s' "$(basename "$0")" >>"$FAKE_LOG"
    for a in "$@"; do printf ' [%s]' "$a" >>"$FAKE_LOG"; done
    printf '\n' >>"$FAKE_LOG"
}
secret() {
    printf 'secret file mode %s holds [%s]\n' "$(ls -l "$1" | cut -c1-10)" "$(cat "$1")" >>"$FAKE_LOG"
}
END_LIB

fake() {
    {
        printf '#!/bin/sh\n. "%s/fake/lib"\nrecord "$@"\n' "$work"
        cat
    } >"$work/fake/$1"
    chmod +x "$work/fake/$1"
}

fake osslsigncode <<'END_TOOL'
while [ $# -gt 0 ]; do
    case $1 in
    -readpass) secret "$2" ;;
    -in) in=$2 ;;
    -out) out=$2 ;;
    esac
    shift
done
if [ -n "${FAKE_FAIL:-}" ]; then
    echo "bad password ${SALAM_SIGN_PASSWORD:-}" >&2
    exit 3
fi
cp "$in" "$out"
sign_pe "$out"
END_TOOL

fake signtool <<'END_TOOL'
if [ -n "${FAKE_FAIL:-}" ]; then
    echo "SignTool Error with $*"
    exit 1
fi
for last in "$@"; do :; done
sign_pe "$last"
END_TOOL

fake mysigner <<'END_TOOL'
if [ -n "${FAKE_PIN:-}" ]; then
    read -r pin
    echo "pin from the terminal input: $pin" >>"$FAKE_LOG"
fi
echo "environment password: ${SALAM_SIGN_PASSWORD:+present}" >>"$FAKE_LOG"
[ -n "${FAKE_NOOP:-}" ] || sign_pe "$2"
END_TOOL

fake codesign </dev/null

fake ditto <<'END_TOOL'
for last in "$@"; do :; done
: >"$last"
END_TOOL

fake xcrun <<'END_TOOL'
[ "$1" = notarytool ] && printf '{"id":"1","status": "%s"}\n' "${FAKE_STATUS:-Accepted}"
exit 0
END_TOOL

fake gpg <<'END_TOOL'
while [ $# -gt 0 ]; do
    case $1 in
    --passphrase-file) secret "$2" ;;
    --output) out=$2 ;;
    esac
    shift
done
[ -n "${FAKE_FAIL:-}" ] && exit 2
echo signature >"$out"
END_TOOL

make_pe() {
    {
        printf 'MZ'
        head -c 58 /dev/zero
        printf '\100\000\000\000'
        printf 'PE\000\000'
        head -c 20 /dev/zero
        printf '\013\002'
        head -c 238 /dev/zero
    } >"$1"
}

tidy() {
    sed -e "s|$work/tmp/salam-sign-[0-9-]*|SECRET|g" \
        -e "s|$work/tmp/salam-sign-[0-9-]*\.zip|ZIP|g" -e "s|SECRET\.zip|ZIP|g" \
        -e "s|$work|WORK|g" -e 's/^\[[0-9: -]*\]\[DRIVER\]//'
}

run() {
    : >"$FAKE_LOG"
    TMPDIR="$work/tmp" PATH="$work/fake:$PATH" "$salam" "$@" --no-color --log-level=info \
        >"$work/out" 2>&1
    code=$?
    grep -E '^(\[[0-9: -]*\]\[DRIVER\]\[(INFO|WARN|ERROR)\] (sign|salam)|salam|bad password|SignTool)' \
        "$work/out" | tidy
    echo "exit $code"
    tidy <"$FAKE_LOG"
}

left_over() {
    echo "temporary files left: $(find "$work/tmp" -name 'salam-sign-*' | wc -l | tr -d ' ')"
}

printf 'certificate' >cert.pfx
cat >salam.toml <<'END_TOML'
[app]
id = "ir.example.notes"
name = "Notes"

[windows]
sign_cert = "cert.pfx"
sign_password = "never-read"
timestamp_url = "http://ts.example/rfc3161"
END_TOML

echo "== osslsigncode, password from the environment"
make_pe "my app.exe"
SALAM_SIGN_PASSWORD='s3cret pw' run sign "my app.exe" --sign-tool=osslsigncode
left_over
echo "== osslsigncode, password file with a newline, no timestamp"
printf 'filepw\n' >"$work/pw"
make_pe app.exe
run sign app.exe --sign-tool=osslsigncode "--sign-password-file=$work/pw" --no-timestamp
left_over
echo "== a failing tool, password scrubbed from its output"
make_pe app.exe
FAKE_FAIL=1 SALAM_SIGN_PASSWORD=s3cret run sign app.exe --sign-tool=osslsigncode
left_over
ls app.exe*
echo "== signtool with a thumbprint"
run sign app.exe --sign-tool=signtool --sign-thumbprint=00AA11BB
echo "== signtool with a .pfx, password redacted"
make_pe app.exe
SALAM_SIGN_PASSWORD=s3cret FAKE_FAIL=1 run sign app.exe --sign-tool=signtool
echo "== custom command, hostile file name"
cat >salam.toml <<'END_TOML'
[windows]
sign_command = ["mysigner", "--in", "{file}", "--ts={timestamp_url}"]
END_TOML
name='a;touch pwned $(touch pwned2) &.exe'
make_pe "$name"
SALAM_SIGN_PASSWORD=s3cret run sign "$name"
echo "files made by the name: $(find . -name 'pwned*' | wc -l | tr -d ' ')"
echo "== custom command reads a PIN from standard input"
make_pe app.exe
printf '1234\n' | FAKE_PIN=1 run sign app.exe
echo "== custom command that signs nothing"
make_pe app.exe
FAKE_NOOP=1 run sign app.exe
echo "== configuration errors"
printf '[windows]\nsign_command = "mysigner {file}"\n' >salam.toml
run sign app.exe
printf '[windows]\n' >salam.toml
run sign app.exe
run sign app.exe --sign-cert=missing.pfx --sign-tool=osslsigncode
run sign app.exe --sign-cert=cert.pfx --timestamp-url=-bad --sign-tool=osslsigncode
run sign app.exe --sign-cert=cert.pfx --sign-password-file=nowhere --sign-tool=osslsigncode
echo "== the tool is missing"
PATH="$work/empty" "$salam" sign app.exe --sign-cert=cert.pfx --sign-tool=osslsigncode --no-color \
    >"$work/out" 2>&1
echo "exit $?"
tidy <"$work/out"

cat >salam.toml <<'END_TOML'
[macos]
sign_identity = "Developer ID Application: Example (ABCDE12345)"
notary_profile = "ci"
END_TOML
mkdir -p Notes.app/Contents/MacOS
echo "== codesign an application and notarize it"
run sign Notes.app --notarize
echo "== a disk image"
: >Notes.dmg
run sign Notes.dmg --notarize
echo "== ad-hoc"
run sign Notes.app --sign-identity=-
echo "== Apple says no"
FAKE_STATUS=Invalid run sign Notes.dmg --notarize

printf 'func main:\n    println "hi"\nend\n' >main.salam
printf '[linux]\ngpg_key = "ABCD"\n' >salam.toml
echo "== build --sign"
SALAM_SIGN_PASSWORD=s3cret run build main.salam --sign -o hi
ls hi hi.asc
echo "== a second build --sign signs again"
run build main.salam --sign -o hi | grep -c '^gpg '
echo "== a signing failure fails the build"
FAKE_FAIL=1 run build main.salam --sign -o hi
ls hi.asc 2>/dev/null
echo "== signing is checked before building"
rm -f hi hi.asc
printf '[linux]\n' >salam.toml
run build main.salam --sign -o hi
echo "programs built: $(find . -name hi | wc -l | tr -d ' ')"
echo "== signing keys of the project file, and passwords in it"
cat >salam.toml <<'END_TOML'
[windows]
sign_cert = "cert.pfx"
sign_command = ["mysigner", "{file}"]
pfx_password = "never-read"

[macos]
sign_identity = "x"
notarize = "yes"

[linux]
gpg_key = "ABCD"
sign_password = "never-read"
nonsense = 1
END_TOML
run build . --sign -o hi | grep -v '^gpg '
echo "== sign options need --sign"
run build main.salam --gpg-key=ABCD -o hi
run build main.salam --sign --platform=web

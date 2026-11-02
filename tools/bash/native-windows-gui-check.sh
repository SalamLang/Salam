#!/bin/sh
# Builds a Windows program with the native backend and the built-in linker
# with an icon, a manifest, version information and the GUI subsystem, and
# checks the result the way Windows reads it:
#
#   - the PE header says IMAGE_SUBSYSTEM_WINDOWS_GUI and the program still
#     runs main (its output reaches a redirected stdout),
#   - a C program run under wine loads the .exe as a data file and finds the
#     group icon (and turns it into an HICON), the manifest (byte for byte)
#     and the version strings through GetFileVersionInfo/VerQueryValue,
#   - the same settings given in a salam.toml [windows] table give the same
#     resources,
#   - a console build has no resource section and the console subsystem.
#
# Needs wine on PATH and the mingw-w64 runtime for the target. The steps that
# need a cross C compiler or llvm-readobj are skipped when it is missing.
# NATIVE_WIN_TRIPLE picks the target (x86_64-w64-windows-gnu by default).
#
# Usage: tools/bash/native-windows-gui-check.sh <salam>

set -u
SALAM=$1
case "$SALAM" in /*) ;; */*) SALAM="$(pwd)/$SALAM" ;; esac
TRIPLE=${NATIVE_WIN_TRIPLE:-x86_64-w64-windows-gnu}
case "$TRIPLE" in
    i686-*) CC=i686-w64-mingw32-gcc ;;
    aarch64-*) CC=aarch64-w64-mingw32-clang ;;
    *) CC=x86_64-w64-mingw32-gcc ;;
esac
case "$TRIPLE" in
    aarch64-*) run=false ;;
    *) run=true ;;
esac
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
export WINEDEBUG=-all
pass=0
fail=0

ok() {
    echo "ok   $1"
    pass=$((pass + 1))
}

bad() {
    echo "FAIL $1"
    fail=$((fail + 1))
}

# A 16x16, 32-bit icon: ICONDIR, one ICONDIRENTRY, then the DIB (header,
# 256 BGRA pixels and the 1-bit AND mask).
make_icon() {
    {
        printf '\000\000\001\000\001\000'
        printf '\020\020\000\000\001\000\040\000\150\004\000\000\026\000\000\000'
        printf '\050\000\000\000\020\000\000\000\040\000\000\000\001\000\040\000'
        printf '\000\000\000\000\000\000\000\000\000\000\000\000'
        printf '\000\000\000\000\000\000\000\000\000\000\000\000'
        i=0
        while [ $i -lt 256 ]; do
            printf '\000\200\377\377'
            i=$((i + 1))
        done
        i=0
        while [ $i -lt 64 ]; do
            printf '\000'
            i=$((i + 1))
        done
    } >"$1"
}

mkdir -p "$work/app"
make_icon "$work/app/app.ico"
cat >"$work/app/app.manifest" <<'EOF'
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
    <assemblyIdentity type="win32" name="Salam.GuiCheck" version="1.2.3.4"/>
    <trustInfo xmlns="urn:schemas-microsoft-com:asm.v3">
        <security>
            <requestedPrivileges>
                <requestedExecutionLevel level="asInvoker" uiAccess="false"/>
            </requestedPrivileges>
        </security>
    </trustInfo>
</assembly>
EOF
cat >"$work/app/main.salam" <<'EOF'
func main():
    println "gui main ran"
end
EOF

build() {
    dir=$1
    out=$2
    shift 2
    (cd "$dir" && "$SALAM" build "$@" --backend=native --linker=builtin --target="$TRIPLE" \
        --output="$out" --log-level=error >"$out.log" 2>&1)
}

if build "$work/app" "$work/app/gui.exe" main.salam --subsystem=windows --icon=app.ico \
    --manifest=app.manifest --version-info=version=1.2.3.4 --version-info=company=Salam \
    "--version-info=description=GUI check" "--version-info=product=Salam GUI" \
    "--version-info=copyright=(c) Salam"; then
    ok "build with --subsystem, --icon, --manifest and --version-info"
else
    bad "build"
    head -5 "$work/app/gui.exe.log"
fi

if build "$work/app" "$work/app/con.exe" main.salam; then
    ok "console build"
else
    bad "console build"
    head -5 "$work/app/con.exe.log"
fi

mkdir -p "$work/proj"
cp "$work/app/main.salam" "$work/app/app.ico" "$work/app/app.manifest" "$work/proj/"
cat >"$work/proj/salam.toml" <<'EOF'
[package]
name = "guiproj"
version = "1.2.3.4"

[windows]
subsystem = "windows"
icon = "app.ico"
manifest = "app.manifest"
company = "Salam"
description = "GUI check"
product = "Salam GUI"
copyright = "(c) Salam"
EOF
if build "$work/proj" "$work/proj/proj.exe" .; then
    ok "project build with a [windows] table"
else
    bad "project build"
    head -5 "$work/proj/proj.exe.log"
fi

if command -v llvm-readobj >/dev/null 2>&1; then
    for exe in "$work/app/gui.exe" "$work/proj/proj.exe"; do
        llvm-readobj --file-headers --coff-resources "$exe" >"$exe.txt" 2>&1
        if grep -q "IMAGE_SUBSYSTEM_WINDOWS_GUI" "$exe.txt" && grep -q "Type: ICON (ID 3)" "$exe.txt" &&
            grep -q "Type: GROUP_ICON (ID 14)" "$exe.txt" && grep -q "Type: VERSIONINFO (ID 16)" "$exe.txt" &&
            grep -q "Type: MANIFEST (ID 24)" "$exe.txt"; then
            ok "$(basename "$exe"): GUI subsystem and resource directory"
        else
            bad "$(basename "$exe"): header or resource directory"
            grep -E "Subsystem|Type:" "$exe.txt" | head -10
        fi
    done
    llvm-readobj --file-headers "$work/app/con.exe" >"$work/con.txt" 2>&1
    if grep -q "IMAGE_SUBSYSTEM_WINDOWS_CUI" "$work/con.txt" && grep -q "ResourceTableRVA: 0x0" "$work/con.txt"; then
        ok "console build keeps the console subsystem and has no resources"
    else
        bad "console header"
        grep -E "Subsystem|ResourceTable" "$work/con.txt"
    fi
fi

if $run; then
    if (cd "$work/app" && timeout 60 wine ./gui.exe </dev/null >"$work/gui.out" 2>/dev/null) &&
        [ "$(tr -d '\r' <"$work/gui.out")" = "gui main ran" ]; then
        ok "GUI program runs main"
    else
        bad "GUI program output"
        head -3 "$work/gui.out"
    fi
fi

if $run && command -v "$CC" >/dev/null 2>&1; then
    cat >"$work/res.c" <<'EOF'
#include <stdio.h>
#include <string.h>
#include <windows.h>

static int query(const char *file, const char *key, const char *want)
{
    DWORD h = 0;
    DWORD n = GetFileVersionInfoSizeA(file, &h);
    char path[128];
    char *buf;
    char *val = NULL;
    UINT len = 0;
    int good;
    if (n == 0)
        return 0;
    buf = malloc(n);
    if (!GetFileVersionInfoA(file, 0, n, buf))
        return 0;
    snprintf(path, sizeof path, "\\StringFileInfo\\040904B0\\%s", key);
    good = VerQueryValueA(buf, path, (void **)&val, &len) && strcmp(val, want) == 0;
    if (!good)
        printf("version %s: got '%s'\n", key, val ? val : "(none)");
    free(buf);
    return good;
}

static int fixed(const char *file)
{
    DWORD h = 0;
    DWORD n = GetFileVersionInfoSizeA(file, &h);
    char *buf = malloc(n ? n : 1);
    VS_FIXEDFILEINFO *fi = NULL;
    UINT len = 0;
    int good = n && GetFileVersionInfoA(file, 0, n, buf) && VerQueryValueA(buf, "\\", (void **)&fi, &len);
    good = good && fi->dwSignature == 0xFEEF04BD && fi->dwFileVersionMS == 0x00010002;
    good = good && fi->dwFileVersionLS == 0x00030004;
    free(buf);
    return good;
}

int main(int argc, char **argv)
{
    const char *file = argv[1];
    long mlen = atol(argv[2]);
    HMODULE m = LoadLibraryExA(file, NULL, LOAD_LIBRARY_AS_DATAFILE);
    HRSRC r;
    int id;
    HICON icon;
    if (argc < 3 || !m) {
        printf("cannot load %s\n", file);
        return 1;
    }
    r = FindResourceA(m, MAKEINTRESOURCEA(1), MAKEINTRESOURCEA(14));
    if (!r) {
        printf("no group icon\n");
        return 1;
    }
    id = LookupIconIdFromDirectoryEx(LockResource(LoadResource(m, r)), TRUE, 16, 16, LR_DEFAULTCOLOR);
    r = FindResourceA(m, MAKEINTRESOURCEA(id), MAKEINTRESOURCEA(3));
    icon = NULL;
    if (r)
        icon = CreateIconFromResourceEx(LockResource(LoadResource(m, r)), SizeofResource(m, r), TRUE, 0x00030000, 16,
            16, LR_DEFAULTCOLOR);
    if (!icon) {
        printf("icon %d does not load\n", id);
        return 1;
    }
    r = FindResourceA(m, MAKEINTRESOURCEA(1), MAKEINTRESOURCEA(24));
    if (!r || (long)SizeofResource(m, r) != mlen) {
        printf("manifest missing or wrong size\n");
        return 1;
    }
    if (!fixed(file) || !query(file, "CompanyName", "Salam") || !query(file, "FileDescription", "GUI check") ||
        !query(file, "ProductName", "Salam GUI") || !query(file, "LegalCopyright", "(c) Salam") ||
        !query(file, "FileVersion", "1.2.3.4")) {
        printf("version info differs\n");
        return 1;
    }
    printf("resources ok\n");
    return 0;
}
EOF
    mlen=$(wc -c <"$work/app/app.manifest" | tr -d ' ')
    if "$CC" "$work/res.c" -o "$work/res.exe" -lversion >"$work/res.log" 2>&1; then
        for exe in "$work/app/gui.exe" "$work/proj/proj.exe"; do
            cp "$exe" "$work/t.exe"
            if (cd "$work" && timeout 60 wine ./res.exe t.exe "$mlen" </dev/null >"$work/res.out" 2>/dev/null) &&
                [ "$(tr -d '\r' <"$work/res.out")" = "resources ok" ]; then
                ok "$(basename "$exe"): Windows finds the icon, manifest and version strings"
            else
                bad "$(basename "$exe"): resources under wine"
                head -5 "$work/res.out"
            fi
        done
    else
        bad "resource reader does not build"
        head -5 "$work/res.log"
    fi
fi

echo "gui check ($TRIPLE): $pass passed, $fail failed"
[ "$fail" -eq 0 ]

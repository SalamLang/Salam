#!/bin/sh
# Builds the Salam plugin for JetBrains IDEs with javac: no Gradle. It
# compiles against IntelliJ IDEA Community (downloaded once into a cache) and
# LSP4IJ, and writes build/salam-jetbrains-<version>.zip, which installs
# through Settings | Plugins | Install Plugin from Disk.
#
# Usage: extensions/jetbrains/plugin/build.sh [--smoke path/to/salam] [--verify]
#
# --smoke   also starts that salam through the plugin's own classes and
#           LSP4IJ's process launcher, sends initialize, and checks that the
#           TextMate bundle is found inside the unpacked plugin
# --verify  also runs JetBrains' Plugin Verifier against the IDE build, which
#           reports any API the plugin uses that the IDE does not have
#
# Env: SALAM_JB_CACHE  where downloads are kept (default ~/.cache/salam-jetbrains)

set -eu

SMOKE=""
VERIFY=0
while [ $# -gt 0 ]; do
    case $1 in
    --smoke)
        SMOKE=$2
        shift 2
        ;;
    --verify)
        VERIFY=1
        shift
        ;;
    *)
        echo "build.sh: unknown option $1" >&2
        exit 2
        ;;
    esac
done

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../.." && pwd)
CACHE=${SALAM_JB_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/salam-jetbrains}
OUT=$HERE/build
VERSION=$(tr -d ' \n\r' <"$ROOT/VERSION")

IDEA=2024.2.6
LSP4IJ_UPDATE=1157565
VERIFIER=1.381

fetch() {
    [ -s "$2" ] && return 0
    echo "downloading $(basename "$2")"
    curl -fsSL -o "$2.part" "$1"
    mv "$2.part" "$2"
}

mkdir -p "$CACHE"
fetch "https://download.jetbrains.com/idea/ideaIC-$IDEA.tar.gz" "$CACHE/ideaIC-$IDEA.tar.gz"
if [ ! -d "$CACHE/ideaIC-$IDEA" ]; then
    mkdir -p "$CACHE/ideaIC-$IDEA.part"
    tar xzf "$CACHE/ideaIC-$IDEA.tar.gz" -C "$CACHE/ideaIC-$IDEA.part" --strip-components=1
    mv "$CACHE/ideaIC-$IDEA.part" "$CACHE/ideaIC-$IDEA"
fi
IDE=$CACHE/ideaIC-$IDEA
fetch "https://plugins.jetbrains.com/plugin/download?updateId=$LSP4IJ_UPDATE" "$CACHE/lsp4ij-$LSP4IJ_UPDATE.zip"
if [ ! -d "$CACHE/lsp4ij-$LSP4IJ_UPDATE" ]; then
    (cd "$CACHE" && unzip -q -o "lsp4ij-$LSP4IJ_UPDATE.zip" -d "lsp4ij-$LSP4IJ_UPDATE")
fi

CP=""
for j in "$IDE"/lib/*.jar "$IDE"/plugins/textmate/lib/*.jar "$CACHE/lsp4ij-$LSP4IJ_UPDATE"/*/lib/*.jar; do
    CP="$CP${CP:+:}$j"
done

rm -rf "$OUT"
mkdir -p "$OUT/classes/META-INF" "$OUT/classes/icons" "$OUT/Salam/lib" "$OUT/Salam/textmate/salam/syntaxes"
find "$HERE/src/main/java" -name '*.java' >"$OUT/sources.txt"
javac --release 17 -proc:none -Xlint:all -Werror -d "$OUT/classes" -cp "$CP" @"$OUT/sources.txt"
sed "s/@VERSION@/$VERSION/" "$HERE/src/main/resources/META-INF/plugin.xml" >"$OUT/classes/META-INF/plugin.xml"
cp "$ROOT/extensions/vscode/icons/salam-file.svg" "$OUT/classes/META-INF/pluginIcon.svg"
cp "$ROOT/extensions/vscode/icons/salam-file.svg" "$OUT/classes/icons/salam.svg"
(cd "$OUT/classes" && jar --create --file "$OUT/Salam/lib/salam-jetbrains.jar" .)

cp "$ROOT/extensions/vscode/syntaxes/salam.tmLanguage.json" "$OUT/Salam/textmate/salam/syntaxes/"
cp "$ROOT/extensions/vscode/language-configuration.json" "$OUT/Salam/textmate/salam/"
cat >"$OUT/Salam/textmate/salam/package.json" <<JSON
{
    "name": "salam",
    "version": "$VERSION",
    "contributes": {
        "languages": [{ "id": "salam", "aliases": ["Salam"], "extensions": [".salam"], "configuration": "./language-configuration.json" }],
        "grammars": [{ "language": "salam", "scopeName": "source.salam", "path": "./syntaxes/salam.tmLanguage.json" }]
    }
}
JSON

ZIP=$OUT/salam-jetbrains-$VERSION.zip
(cd "$OUT" && rm -f "$ZIP" && zip -q -r "$ZIP" Salam)
echo "built $ZIP"

if [ -n "$SMOKE" ]; then
    RUN=$OUT/smoke
    mkdir -p "$RUN"
    (cd "$RUN" && unzip -q -o "$ZIP")
    RCP="$CP:$RUN/Salam/lib/salam-jetbrains.jar"
    javac -nowarn -proc:none -d "$RUN" -cp "$RCP" "$HERE/smoke/PluginSmoke.java" 2>/dev/null
    SALAM_PATH=$SMOKE java -cp "$RCP:$RUN" PluginSmoke "$RUN" | tee "$RUN/result.txt"
    if ! grep -q "initialize ok" "$RUN/result.txt" || ! grep -q "textmate/salam" "$RUN/result.txt"; then
        echo "build.sh: smoke test failed" >&2
        exit 1
    fi
    mkdir -p "$RUN/tests"
    printf 'func test_good():\n    testing.AssertEqInt(1 + 1, 2)\nend\n\nfunc test_bad():\n    testing.AssertEqStr("a", "b")\nend\n' >"$RUN/tests/smoke_test.salam"
    javac -nowarn -proc:none -d "$RUN" -cp "$RCP" "$HERE/smoke/TeamCitySmoke.java" 2>/dev/null
    (cd "$RUN/tests" && SALAM_STD="${SALAM_STD:-$ROOT/std}" "$SMOKE" test --format=teamcity 2>/dev/null) |
        java -cp "$RCP:$RUN" TeamCitySmoke | tee "$RUN/teamcity.txt"
    if ! grep -q "2 started, 2 finished, 1 failed: strings differ" "$RUN/teamcity.txt"; then
        echo "build.sh: salam test output does not read as a test tree" >&2
        exit 1
    fi
    echo "smoke test passed"
fi

if [ "$VERIFY" = 1 ]; then
    REPO=https://packages.jetbrains.team/maven/p/intellij-plugin-verifier/intellij-plugin-verifier
    fetch "$REPO/org/jetbrains/intellij/plugins/verifier-cli/$VERIFIER/verifier-cli-$VERIFIER-all.jar" \
        "$CACHE/verifier-cli-$VERIFIER-all.jar"
    mkdir -p "$OUT/verifier"
    java -jar "$CACHE/verifier-cli-$VERIFIER-all.jar" check-plugin "$ZIP" "$IDE" \
        -plugins-to-check-all-builds "$CACHE/lsp4ij-$LSP4IJ_UPDATE.zip" -verification-reports-dir "$OUT/verifier" | tee "$OUT/verifier.txt"
    if grep -qE "Compatibility problems|[1-9][0-9]* compatibility problem" "$OUT/verifier.txt"; then
        echo "build.sh: the Plugin Verifier found compatibility problems" >&2
        exit 1
    fi
    echo "plugin verifier passed"
fi

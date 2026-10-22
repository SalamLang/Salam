#!/bin/sh
# Builds the Salam plugin for JetBrains IDEs with javac alone: no Gradle and
# no IDE download. It fetches the three jars the sources compile against
# (about 20 MB) into a cache, compiles, and writes
# build/salam-jetbrains-<version>.zip, which installs through
# Settings | Plugins | Install Plugin from Disk.
#
# Usage: extensions/jetbrains/plugin/build.sh [--smoke path/to/salam]
#
# --smoke also starts that salam through the plugin's own classes and
# LSP4IJ's process launcher, sends initialize, and checks that the TextMate
# bundle is found inside the unpacked plugin. It needs two more jars.
# Env:   SALAM_JB_CACHE  where the downloaded jars are kept (default ~/.cache/salam-jetbrains)

set -eu

SMOKE=""
if [ "${1:-}" = "--smoke" ]; then
    SMOKE=$2
fi

HERE=$(cd "$(dirname "$0")" && pwd)
ROOT=$(cd "$HERE/../../.." && pwd)
CACHE=${SALAM_JB_CACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/salam-jetbrains}
OUT=$HERE/build
VERSION=$(tr -d ' \n\r' <"$ROOT/VERSION")

PLATFORM=242.26775.26
TEXTMATE=242.26775.22
LSP4IJ_UPDATE=1157565
KOTLIN=1.9.24
REPO=https://cache-redirector.jetbrains.com/intellij-repository/releases

fetch() {
    [ -s "$2" ] && return 0
    echo "downloading $(basename "$2")"
    curl -fsSL -o "$2.part" "$1"
    mv "$2.part" "$2"
}

mkdir -p "$CACHE"
fetch "$REPO/com/jetbrains/intellij/platform/core/$PLATFORM/core-$PLATFORM.jar" "$CACHE/core-$PLATFORM.jar"
fetch "$REPO/com/jetbrains/intellij/platform/extensions/$PLATFORM/extensions-$PLATFORM.jar" "$CACHE/extensions-$PLATFORM.jar"
fetch "$REPO/com/jetbrains/intellij/platform/util/$PLATFORM/util-$PLATFORM.jar" "$CACHE/util-$PLATFORM.jar"
fetch "https://repo1.maven.org/maven2/org/jetbrains/kotlin/kotlin-stdlib/$KOTLIN/kotlin-stdlib-$KOTLIN.jar" "$CACHE/kotlin-stdlib-$KOTLIN.jar"
fetch "$REPO/com/jetbrains/intellij/textmate/textmate/$TEXTMATE/textmate-$TEXTMATE.jar" "$CACHE/textmate-$TEXTMATE.jar"
fetch "https://plugins.jetbrains.com/plugin/download?updateId=$LSP4IJ_UPDATE" "$CACHE/lsp4ij-$LSP4IJ_UPDATE.zip"
if [ ! -d "$CACHE/lsp4ij-$LSP4IJ_UPDATE" ]; then
    (cd "$CACHE" && unzip -q -o "lsp4ij-$LSP4IJ_UPDATE.zip" -d "lsp4ij-$LSP4IJ_UPDATE")
fi
LSP4IJ_JAR=$(find "$CACHE/lsp4ij-$LSP4IJ_UPDATE" -name 'lsp4ij-*.jar' | head -n 1)
[ -n "$LSP4IJ_JAR" ] || {
    echo "build.sh: lsp4ij jar not found" >&2
    exit 1
}
CP="$CACHE/core-$PLATFORM.jar:$CACHE/extensions-$PLATFORM.jar:$CACHE/util-$PLATFORM.jar:$CACHE/kotlin-stdlib-$KOTLIN.jar:$CACHE/textmate-$TEXTMATE.jar:$LSP4IJ_JAR"
for j in "$CACHE/lsp4ij-$LSP4IJ_UPDATE"/*/lib/org.eclipse.lsp4j*.jar; do
    CP="$CP:$j"
done

rm -rf "$OUT"
mkdir -p "$OUT/classes/META-INF" "$OUT/Salam/lib" "$OUT/Salam/textmate/salam/syntaxes"
find "$HERE/src/main/java" -name '*.java' >"$OUT/sources.txt"
javac --release 17 -Xlint:all -Werror -d "$OUT/classes" -cp "$CP" @"$OUT/sources.txt"
sed "s/@VERSION@/$VERSION/" "$HERE/src/main/resources/META-INF/plugin.xml" >"$OUT/classes/META-INF/plugin.xml"
cp "$ROOT/extensions/vscode/icons/salam-file.svg" "$OUT/classes/META-INF/pluginIcon.svg"
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

(cd "$OUT" && rm -f "salam-jetbrains-$VERSION.zip" && zip -q -r "salam-jetbrains-$VERSION.zip" Salam)
echo "built $OUT/salam-jetbrains-$VERSION.zip"

if [ -n "$SMOKE" ]; then
    fetch "$REPO/com/jetbrains/intellij/platform/util-rt/$PLATFORM/util-rt-$PLATFORM.jar" "$CACHE/util-rt-$PLATFORM.jar"
    fetch "https://repo1.maven.org/maven2/org/jetbrains/kotlinx/kotlinx-coroutines-core-jvm/1.8.1/kotlinx-coroutines-core-jvm-1.8.1.jar" "$CACHE/kotlinx-coroutines-core-jvm-1.8.1.jar"
    RUN=$OUT/smoke
    mkdir -p "$RUN"
    (cd "$RUN" && unzip -q -o "$OUT/salam-jetbrains-$VERSION.zip")
    RCP="$CP:$CACHE/util-rt-$PLATFORM.jar:$CACHE/kotlinx-coroutines-core-jvm-1.8.1.jar"
    for j in "$CACHE/lsp4ij-$LSP4IJ_UPDATE"/*/lib/*.jar; do
        RCP="$RCP:$j"
    done
    RCP="$RCP:$RUN/Salam/lib/salam-jetbrains.jar"
    javac -nowarn -d "$RUN" -cp "$RCP" "$HERE/smoke/PluginSmoke.java" 2>/dev/null
    SALAM_PATH=$SMOKE java -cp "$RCP:$RUN" PluginSmoke "$RUN" | tee "$RUN/result.txt"
    if ! grep -q "initialize ok" "$RUN/result.txt" || ! grep -q "textmate/salam" "$RUN/result.txt"; then
        echo "build.sh: smoke test failed" >&2
        exit 1
    fi
    echo "smoke test passed"
fi

#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$0")/../.."

OUT="std/tls/roots_embedded.salam"
SRC="${1:-}"

if [ -z "$SRC" ]; then
    for p in /etc/ssl/certs/ca-certificates.crt \
        /etc/pki/tls/certs/ca-bundle.crt \
        /etc/ssl/ca-bundle.pem \
        /etc/ssl/cert.pem \
        /usr/ssl/certs/ca-bundle.crt; do
        if [ -f "$p" ]; then
            SRC="$p"
            break
        fi
    done
fi

if [ -z "$SRC" ] || [ ! -f "$SRC" ]; then
    echo "gen-tls-roots: no CA bundle found; pass one explicitly" >&2
    exit 1
fi

COUNT=$(grep -c 'BEGIN CERTIFICATE' "$SRC" || true)
if [ "$COUNT" -eq 0 ]; then
    echo "gen-tls-roots: $SRC contains no certificates" >&2
    exit 1
fi

echo "gen-tls-roots: embedding $COUNT certificates from $SRC" >&2

{
    cat <<'HDR'
/*
 * Salam Programming Language (2024-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

package tls

// GENERATED FILE - do not edit by hand.
// Regenerate with tools/bash/gen-tls-roots.sh
//
// Fallback trust roots, used only when no system CA bundle can be located.
// See roots.salam for the search order.
HDR
    echo "//"
    echo "// source:       $SRC"
    echo "// certificates: $COUNT"
    echo
    echo 'func _embedded_roots_data(): str:'
    echo '    ret """'
    awk '/BEGIN CERTIFICATE/,/END CERTIFICATE/' "$SRC"
    echo '"""'
    echo 'end'
} >"$OUT"

echo "gen-tls-roots: wrote $OUT" >&2
echo "gen-tls-roots: now point _embedded_roots_pem() in roots.salam at" >&2
echo "               _embedded_roots_data(), or delete the file to disable." >&2

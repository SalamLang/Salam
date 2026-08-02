#!/usr/bin/env bash
#
# Generate std/tls/roots_embedded.salam from a system CA bundle.
#
# The embedded bundle is the LAST-RESORT trust source for std/tls. It is used
# only when no explicit ca_file was configured, $SSL_CERT_FILE is unset, and
# none of the well-known system bundle paths exist, which in practice means a
# statically linked binary in a scratch container. See std/tls/roots.salam for
# the full search order.
#
# Without this file std/tls falls back to an EMPTY trust store. That fails every
# chain rather than trusting one, which is the safe direction, but it also means
# HTTPS will not work in such an image until this is generated.
#
# Usage:
#   c/tools/bash/gen-tls-roots.sh [path-to-ca-bundle.pem]
#
# Re-run whenever the upstream bundle is refreshed. The output is plain Salam
# source and is safe to commit.

set -euo pipefail

cd "$(dirname "$0")/../../.."

OUT="std/tls/roots_embedded.salam"
SRC="${1:-}"

if [ -z "$SRC" ]; then
    for p in /etc/ssl/certs/ca-certificates.crt \
             /etc/pki/tls/certs/ca-bundle.crt \
             /etc/ssl/ca-bundle.pem \
             /etc/ssl/cert.pem \
             /usr/ssl/certs/ca-bundle.crt; do
        if [ -f "$p" ]; then SRC="$p"; break; fi
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
// Regenerate with c/tools/bash/gen-tls-roots.sh
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
    # keep only the PEM blocks; some bundles interleave human-readable metadata
    awk '/BEGIN CERTIFICATE/,/END CERTIFICATE/' "$SRC"
    echo '"""'
    echo 'end'
} > "$OUT"

echo "gen-tls-roots: wrote $OUT" >&2
echo "gen-tls-roots: now point _embedded_roots_pem() in roots.salam at" >&2
echo "               _embedded_roots_data(), or delete the file to disable." >&2

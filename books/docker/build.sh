#!/usr/bin/env sh

set -eu

BOOKS_ROOT="${BOOKS_ROOT:-/books}"
LATEXMK_OPTS="${LATEXMK_OPTS:--xelatex -interaction=nonstopmode -halt-on-error -file-line-error}"
USE_GLOSSARY="${USE_GLOSSARY:-0}"

log() {
    printf "\n==> %s\n" "$*"
}

error() {
    printf "::error:: %s\n" "$*" >&2
}

run_latexmk() {
    # shellcheck disable=SC2086 # LATEXMK_OPTS holds multiple flags; must word-split.
    if [ "$USE_GLOSSARY" -eq 1 ]; then
        # shellcheck disable=SC2086 # LATEXMK_OPTS is a list of options; word splitting is intentional
        latexmk $LATEXMK_OPTS -use-makeglossaries book.tex
    else
        # shellcheck disable=SC2086 # LATEXMK_OPTS is a list of options; word splitting is intentional
        latexmk $LATEXMK_OPTS book.tex
    fi
}

build_one() {
    lang="$1"
    found=0
    for dir in "$BOOKS_ROOT/$lang"/*/; do
        [ -f "$dir/book.tex" ] || continue
        found=1
        log "Building '$lang' book: ${dir}book.tex"
        (
            cd "$dir"
            run_latexmk
            if [ ! -f "book.pdf" ]; then
                error "build failed for ${dir}book.tex (no PDF generated)"
                exit 1
            fi
            log "Done: ${dir}book.pdf"
        ) || return 1
    done
    if [ "$found" -eq 0 ]; then
        error "no book.tex found under $BOOKS_ROOT/$lang"
        return 1
    fi
}

clean_one() {
    lang="$1"
    for dir in "$BOOKS_ROOT/$lang"/*/; do
        [ -f "$dir/book.tex" ] || continue
        log "Cleaning ${dir}"
        (
            cd "$dir"
            latexmk -C || true
        )
    done
}

case "${1:-all}" in
en)
    build_one en
    ;;
fa)
    build_one fa
    ;;
all)
    build_one en
    build_one fa
    ;;
clean)
    clean_one en
    clean_one fa
    ;;
help | -h | --help)
    cat <<EOF
usage: build-books [en|fa|all|clean]

Targets:
en      Build English book
fa      Build Persian book
all     Build both (default)
clean   Remove build artifacts

Environment variables:
BOOKS_ROOT     Root path (default: /books)
LATEXMK_OPTS   Override latexmk options
USE_GLOSSARY   Set to 1 to enable glossaries

Examples:
build-books
build-books en
USE_GLOSSARY=1 build-books fa
EOF
    ;;
*)
    error "unknown target '$1' (expected: en, fa, all, clean)"
    exit 2
    ;;
esac

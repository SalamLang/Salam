#!/bin/sh
# Build the Salam tutorial books (English + Persian) with XeLaTeX via latexmk.
#
# Runs inside the books image (see books/docker/Dockerfile). The books/ tree is
# bind-mounted at /books, so the generated PDFs are written back to the host at
#   books/<lang>/intro-programming/book.pdf
#
# Usage (the argument selects which book):
#   build-books all    # both English and Persian (default)
#   build-books en     # English only  (lighter: polyglossia + tcolorbox)
#   build-books fa     # Persian only  (heavier: xepersian, right-to-left)
set -eu

# Root of the bind-mounted books/ tree; override with BOOKS_ROOT if mounted
# somewhere else.
BOOKS_ROOT="${BOOKS_ROOT:-/books}"

build_one() {
    lang="$1"
    dir="$BOOKS_ROOT/$lang/intro-programming"
    if [ ! -f "$dir/book.tex" ]; then
        echo "::error:: no book.tex found under $dir" >&2
        return 1
    fi
    echo "==> Building '$lang' book: $dir/book.tex"
    cd "$dir"
    # latexmk runs XeLaTeX as many times as needed (table of contents, refs,
    # bookmarks). -halt-on-error + -file-line-error make failures obvious.
    latexmk -xelatex -interaction=nonstopmode -halt-on-error -file-line-error book.tex
    echo "==> Done: $dir/book.pdf"
}

case "${1:-all}" in
    en) build_one en ;;
    fa) build_one fa ;;
    all)
        build_one en
        build_one fa
        ;;
    -h | --help | help)
        echo "usage: build-books [en|fa|all]   (default: all)"
        ;;
    *)
        echo "unknown target '$1' (expected: en, fa, or all)" >&2
        exit 2
        ;;
esac

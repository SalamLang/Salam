#!/usr/bin/env python3
"""Highlight the probe files with GtkSourceView and report the style each
token is given.

Usage: check_gtksource.py <definition-dir> <file...>
       check_gtksource.py --probe-modules

Exit status: 0 success, 1 the definition or a source file failed, and 3 only
when the GObject bindings are not installed. The caller relies on 3 being the
one code that means "skip"; everything else has to surface as a failure.

GtkSourceView is reachable only through GObject introspection, so this stays
in Python for the same reason check_textmate.mjs stays in JavaScript: it is
the engine's own runtime. Everything the check decides lives in
check_engines.salam.
"""

# pylint: disable=import-error,import-outside-toplevel,invalid-name

import json
import sys

MODULES_MISSING = 3


def load():
    """Import GtkSourceView lazily so a missing binding is a skip, not a crash."""
    import gi

    gi.require_version("GtkSource", "4")
    from gi.repository import GtkSource

    return GtkSource


def signature(tags):
    """Name the styling a set of tags applies, so two tokens can be compared."""
    parts = []
    for tag in tags:
        name = tag.get_property("name")
        if name and name.startswith("gtksourceview:context-classes:"):
            continue
        if name:
            parts.append(name)
            continue
        if tag.get_property("foreground-set"):
            colour = tag.get_property("foreground-rgba")
            parts.append(f"fg={colour.red:.2f},{colour.green:.2f},{colour.blue:.2f}")
        if tag.get_property("weight-set"):
            parts.append(f"weight={tag.get_property('weight')}")
    return " ".join(parts)


def styles(source, buffer):
    """Highlight one file and return its [token, style] pairs, first use wins."""
    with open(source, encoding="utf-8") as handle:
        buffer.set_text(handle.read())
    buffer.ensure_highlight(buffer.get_start_iter(), buffer.get_end_iter())

    out = []
    seen = set()
    position = buffer.get_start_iter()
    while not position.is_end():
        following = position.copy()
        if not following.forward_to_tag_toggle(None):
            following = buffer.get_end_iter()
        text = buffer.get_text(position, following, False).strip()
        if text and text not in seen:
            seen.add(text)
            out.append([text, signature(position.get_tags())])
        if position.equal(following):
            if not position.forward_char():
                break
        else:
            position = following
    return out


def main(argv):
    """Print one JSON object mapping each source path to its token styles."""
    try:
        GtkSource = load()
    except (ImportError, ValueError) as error:
        print(f"the GtkSourceView bindings are not installed: {error}", file=sys.stderr)
        return MODULES_MISSING
    if argv and argv[0] == "--probe-modules":
        return 0
    if len(argv) < 2:
        print(__doc__, file=sys.stderr)
        return 1

    definitions, sources = argv[0], argv[1:]
    manager = GtkSource.LanguageManager()
    manager.set_search_path([definitions] + list(manager.get_search_path()))
    language = manager.get_language("salam")
    if language is None:
        print("the engine did not load a language with id 'salam'", file=sys.stderr)
        return 1

    buffer = GtkSource.Buffer()
    scheme = GtkSource.StyleSchemeManager.get_default().get_scheme("classic")
    if scheme:
        buffer.set_style_scheme(scheme)
    buffer.set_language(language)
    buffer.set_highlight_syntax(True)

    print(json.dumps({source: styles(source, buffer) for source in sources}))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

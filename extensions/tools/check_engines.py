#!/usr/bin/env python3
"""Run the VS Code, Kate, Sublime Text and GtkSourceView definitions through
the real engines that read them.

Every engine is optional. Where one is missing the check is skipped rather
than failed, so this runs anywhere `validate.py` does.

What it asserts is the property that matters for a trilingual language: the
Persian and Arabic spelling of a construct must be styled exactly like the
English one, and keywords, types and comments must stay distinguishable.
"""

# pylint: disable=import-error,import-outside-toplevel

import html
import json
import os
import re
import shutil
import subprocess
import sys

FIXTURES = "extensions/tools/fixtures"
KATE_DEFINITION = "extensions/kate/salam.xml"
GTK_DEFINITION = "extensions/gtksourceview"
SUBLIME_DEFINITION = "extensions/sublime/Salam.sublime-syntax"
TEXTMATE_DEFINITION = "extensions/vscode/syntaxes/salam.tmLanguage.json"
TEXTMATE_DRIVER = "extensions/tools/check_textmate.mjs"

# A theme that gives keywords, types and function names three different
# colours, so the assertions below can tell them apart.
SYNTECT_THEME = "Monokai Extended"

# The same construct in each keyword pack. Every row has to come out styled
# the same way in all three columns.
PARALLEL = [
    ("keyword", ["func", "تابع", "دالة"]),
    ("keyword", ["if", "اگر", "إذا"]),
    ("keyword", ["end", "تمام", "نهاية"]),
    ("keyword", ["match", "تطبیق", "طابق"]),
    ("type", ["int", "صحیح۳۲", "صحيح"]),
    ("builtin", ["println", "چاپ", "اطبع"]),
    ("string", ["hello", "سلام", "مرحبا"]),
    ("number", ["42", "۱۲۳", "٤٥٦"]),
    ("comment", ["comment", "comment", "comment"]),
    ("contextual", ["static", "ایستا", "ساكن"]),
]

DISTINCT = ("keyword", "type", "comment")

LANGUAGES = ("en", "fa", "ar")

ENGINE_ERRORS = (
    OSError,
    ValueError,
    RuntimeError,
    IndexError,
    subprocess.CalledProcessError,
)


def probe_path(language):
    return os.path.join(FIXTURES, f"probe_{language}.salam")


def textmate_styles(path, _unused):
    """vscode-textmate reports a scope per token, which names the rule that
    matched far more precisely than a colour does."""
    environment = dict(os.environ)
    out = subprocess.run(
        ["node", TEXTMATE_DRIVER, TEXTMATE_DEFINITION, path],
        capture_output=True,
        text=True,
        check=True,
        env=environment,
    ).stdout
    tokens = json.loads(out)[path]
    styles = {}
    for text, scope in tokens:
        styles.setdefault(text, scope)
    return styles


def have_textmate():
    if not shutil.which("node"):
        return False
    probe = subprocess.run(
        ["node", TEXTMATE_DRIVER, TEXTMATE_DEFINITION, probe_path("en")],
        capture_output=True,
        text=True,
        check=False,
    )
    return probe.returncode == 0


def bat_binary():
    """syntect ships inside bat, which Debian and Ubuntu install as batcat."""
    return shutil.which("batcat") or shutil.which("bat")


def syntect_cache(definition):
    """Build a bat syntax cache holding only the Salam definition."""
    root = os.path.join(
        os.environ.get("TMPDIR", "/tmp"), "salam-engine-check", "syntect"
    )
    syntaxes = os.path.join(root, "syntaxes")
    os.makedirs(syntaxes, exist_ok=True)
    shutil.copy(definition, syntaxes)
    subprocess.run(
        [bat_binary(), "cache", "--build", "--source", root, "--target", root],
        capture_output=True,
        text=True,
        check=True,
    )
    return root


def syntect_styles(path, cache_dir):
    environment = dict(os.environ, BAT_CACHE_PATH=cache_dir)
    out = subprocess.run(
        [
            bat_binary(),
            "--language=salam",
            "--color=always",
            "--style=plain",
            "--paging=never",
            f"--theme={SYNTECT_THEME}",
            path,
        ],
        capture_output=True,
        text=True,
        check=True,
        env=environment,
    ).stdout

    # syntect emits one span per scope change, so a string arrives as three
    # pieces and a comment as two. Merge neighbours that share a colour back
    # into the token a reader sees.
    runs = []
    colour = ""
    for piece in re.split(r"(\x1b\[[0-9;]*m)", out):
        if piece.startswith("\x1b["):
            codes = piece[2:-1]
            colour = "" if codes in ("", "0") else codes
            continue
        for index, line in enumerate(piece.split("\n")):
            if index:
                runs.append((None, "\n"))
            if line:
                runs.append((colour, line))

    styles = {}
    merged = []
    for colour, text in runs:
        if merged and merged[-1][0] == colour:
            merged[-1][1] += text
        else:
            merged.append([colour, text])
    for colour, text in merged:
        if colour is not None and text.strip():
            styles.setdefault(text.strip(), colour)
    return styles


def kate_styles(path, definition_dir):
    environment = dict(os.environ, XDG_DATA_HOME=definition_dir)
    out = subprocess.run(
        ["kate-syntax-highlighter", "-s", "Salam", "-f", "html", path],
        capture_output=True,
        text=True,
        check=True,
        env=environment,
    ).stdout
    body = out.split("<pre>", 1)[1].split("</pre>", 1)[0]
    styles = {}
    for style, text, plain in re.findall(
        r'<span style="([^"]*)">(.*?)</span>|([^<]+)', body
    ):
        content = html.unescape(text if text else plain).strip()
        if content:
            styles.setdefault(content, style.strip().rstrip(";"))
    return styles


def gtk_styles(path, definition_dir):
    import gi

    gi.require_version("GtkSource", "4")
    from gi.repository import GtkSource

    manager = GtkSource.LanguageManager()
    manager.set_search_path([definition_dir] + list(manager.get_search_path()))
    language = manager.get_language("salam")
    if language is None:
        raise RuntimeError("the engine did not load a language with id 'salam'")

    schemes = GtkSource.StyleSchemeManager.get_default()
    buffer = GtkSource.Buffer()
    scheme = schemes.get_scheme("classic")
    if scheme:
        buffer.set_style_scheme(scheme)
    buffer.set_language(language)
    buffer.set_highlight_syntax(True)
    with open(path, encoding="utf-8") as handle:
        buffer.set_text(handle.read())
    buffer.ensure_highlight(buffer.get_start_iter(), buffer.get_end_iter())

    styles = {}
    position = buffer.get_start_iter()
    while not position.is_end():
        following = position.copy()
        if not following.forward_to_tag_toggle(None):
            following = buffer.get_end_iter()
        text = buffer.get_text(position, following, False).strip()
        if text:
            styles.setdefault(text, signature(position.get_tags()))
        if position.equal(following):
            if not position.forward_char():
                break
        else:
            position = following
    return styles


def signature(tags):
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


def lookup(styles, token):
    if token in styles:
        return styles[token]
    for text, style in styles.items():
        if token in text:
            return style
    return None


def compare(per_language):
    """Every row of PARALLEL must come out styled alike in all three packs."""
    problems = []
    category_style = {}
    for category, spellings in PARALLEL:
        found = {}
        for language, token in zip(LANGUAGES, spellings):
            style = lookup(per_language[language], token)
            if style is None:
                problems.append(
                    f"{category}: {token!r} never appears in the {language} output"
                )
            elif not style:
                problems.append(
                    f"{category}: {token!r} ({language}) is not styled at all"
                )
            else:
                found.setdefault(style, []).append(f"{language}:{token}")
        if len(found) > 1:
            problems.append(f"{category}: the three spellings differ: {found}")
        elif found:
            category_style.setdefault(category, next(iter(found)))

    for first in DISTINCT:
        for second in DISTINCT:
            if first < second and category_style.get(first) == category_style.get(
                second
            ):
                problems.append(f"{first} and {second} are styled identically")
    return problems, category_style


def check(engine, reader, definition_dir):
    print(f"{engine}:")
    try:
        per_language = {
            language: reader(probe_path(language), definition_dir)
            for language in LANGUAGES
        }
    except ENGINE_ERRORS as error:
        print(f"  FAIL: {error}")
        return 1

    problems, category_style = compare(per_language)

    for category, style in sorted(category_style.items()):
        print(f"  {category:<9} {style}")
    if problems:
        print("  FAIL")
        for problem in problems:
            print(f"    {problem}")
        return 1
    print("  all three keyword packs are styled alike")
    return 0


def staged(source, subdirectory, filename):
    """Put a definition where its engine expects to find it."""
    root = os.path.join(os.environ.get("TMPDIR", "/tmp"), "salam-engine-check")
    target = os.path.join(root, subdirectory)
    os.makedirs(target, exist_ok=True)
    shutil.copy(source, os.path.join(target, filename))
    return root


def main():
    if not os.path.isdir(FIXTURES):
        print("run me from the repository root", file=sys.stderr)
        return 2

    failures = 0

    if shutil.which("kate-syntax-highlighter"):
        root = staged(
            KATE_DEFINITION, "org.kde.syntax-highlighting/syntax", "salam.xml"
        )
        failures += check("kate", kate_styles, root)
    else:
        print("kate: skipped (kate-syntax-highlighter is not installed)")

    if have_textmate():
        failures += check("vscode-textmate", textmate_styles, None)
    else:
        print("vscode-textmate: skipped (npm install vscode-textmate vscode-oniguruma)")

    if bat_binary():
        try:
            failures += check(
                "syntect", syntect_styles, syntect_cache(SUBLIME_DEFINITION)
            )
        except ENGINE_ERRORS as error:
            print(f"syntect: FAIL building the cache: {error}")
            failures += 1
    else:
        print("syntect: skipped (neither batcat nor bat is installed)")

    try:
        import gi

        gi.require_version("GtkSource", "4")
        have_gtk = True
    except (ImportError, ValueError):
        have_gtk = False

    if have_gtk:
        failures += check("gtksourceview", gtk_styles, os.path.abspath(GTK_DEFINITION))
    else:
        print("gtksourceview: skipped (gir1.2-gtksource-4 is not installed)")

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())

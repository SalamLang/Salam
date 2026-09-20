#!/usr/bin/env python3
"""Check that every generated editor syntax file parses and that its regexes
compile. Run from the repository root."""

import configparser
import json
import os
import plistlib
import re
import sys
import xml.dom.minidom
from xml.parsers.expat import ExpatError

try:
    import regex
except ImportError:
    regex = re

try:
    import yaml
except ImportError:
    yaml = None

ROOT = "extensions"
PARSE_ERRORS = (OSError, ValueError, ExpatError, KeyError, RuntimeError)

failures = []


def fail(where, message):
    failures.append(f"{where}: {message}")


def read_text(path):
    with open(path, encoding="utf-8") as handle:
        return handle.read()


def to_python_regex(pattern):
    """Rewrite Oniguruma's \\x{hhhh} as an escape Python's engines accept."""
    return re.sub(
        r"\\x\{([0-9A-Fa-f]+)\}",
        lambda match: f"\\u{int(match.group(1), 16):04x}",
        pattern,
    )


def compile_all(patterns, where):
    count = 0
    for pattern in patterns:
        if not pattern:
            continue
        count += 1
        try:
            regex.compile(to_python_regex(pattern))
        except re.error as error:
            fail(where, f"regex {pattern[:70]!r}: {error}")
    return count


def load_json(path, strip_comments=False):
    text = read_text(path)
    if strip_comments:
        text = re.sub(r"^\s*//.*$", "", text, flags=re.MULTILINE)
    try:
        return json.loads(text)
    except (OSError, json.JSONDecodeError) as error:
        fail(path, str(error))
        return None


def load_xml(path):
    try:
        return xml.dom.minidom.parse(path)
    except (OSError, ExpatError) as error:
        fail(path, str(error))
        return None


def load_yaml(path):
    try:
        return yaml.safe_load(read_text(path))
    except (OSError, yaml.YAMLError) as error:
        fail(path, str(error))
        return None


def check_textmate(path):
    grammar = load_json(path)
    if grammar is None:
        return
    patterns = []

    def walk(node):
        if isinstance(node, dict):
            for key, value in node.items():
                if key in ("match", "begin", "end") and isinstance(value, str):
                    patterns.append(value)
                elif (
                    key == "include"
                    and isinstance(value, str)
                    and value.startswith("#")
                ):
                    if value[1:] not in grammar["repository"]:
                        fail(path, f"include of unknown repository key {value!r}")
                else:
                    walk(value)
        elif isinstance(node, list):
            for item in node:
                walk(item)

    walk(grammar["patterns"])
    walk(grammar["repository"])
    print(f"  {path:<52} {compile_all(patterns, path)} regexes")


def check_sublime_syntax(path):
    if yaml is None:
        print(f"  {path:<52} skipped (no PyYAML)")
        return
    doc = load_yaml(path)
    if doc is None:
        return

    variables = doc.get("variables", {})

    def expand(text, depth=0):
        if depth > 10:
            raise RuntimeError("variable recursion")
        return re.sub(
            r"\{\{(\w+)\}\}",
            lambda match: expand(variables[match.group(1)], depth + 1),
            text,
        )

    contexts = doc["contexts"]
    patterns = []

    def walk(node, where):
        if isinstance(node, list):
            for item in node:
                walk(item, where)
        elif isinstance(node, dict):
            if "include" in node and node["include"] not in contexts:
                fail(path, f"{where} includes unknown context {node['include']!r}")
            if "match" in node:
                patterns.append(expand(node["match"]))
            for key in ("push", "set"):
                if key not in node:
                    continue
                if isinstance(node[key], str):
                    if node[key] not in contexts:
                        fail(path, f"{where} {key} unknown context {node[key]!r}")
                else:
                    walk(node[key], where)

    for name, body in contexts.items():
        walk(body, name)
    print(f"  {path:<52} {compile_all(patterns, path)} regexes")


def check_kate(path):
    doc = load_xml(path)
    if doc is None:
        return
    names = {c.getAttribute("name") for c in doc.getElementsByTagName("context")}
    lists = {item.getAttribute("name") for item in doc.getElementsByTagName("list")}
    attributes = {i.getAttribute("name") for i in doc.getElementsByTagName("itemData")}

    for element in doc.getElementsByTagName("*"):
        target = element.getAttribute("context")
        if target and not target.startswith("#") and target not in names:
            fail(path, f"rule points at unknown context {target!r}")
        used = element.getAttribute("attribute")
        if used and used not in attributes:
            fail(path, f"rule uses unknown itemData {used!r}")
    for keyword in doc.getElementsByTagName("keyword"):
        used = keyword.getAttribute("String")
        if used not in lists:
            fail(path, f"keyword rule uses unknown list {used!r}")

    patterns = [e.getAttribute("String") for e in doc.getElementsByTagName("RegExpr")]
    print(f"  {path:<52} {compile_all(patterns, path)} regexes, {len(lists)} lists")


def check_gtksourceview(path):
    doc = load_xml(path)
    if doc is None:
        return
    ids = {
        c.getAttribute("id")
        for c in doc.getElementsByTagName("context")
        if c.getAttribute("id")
    }
    styles = {s.getAttribute("id") for s in doc.getElementsByTagName("style")}

    for context in doc.getElementsByTagName("context"):
        target = context.getAttribute("ref")
        if target and ":" not in target and target not in ids:
            fail(path, f"context ref {target!r} is not defined here")
        used = context.getAttribute("style-ref")
        if used and ":" not in used and used not in styles:
            fail(path, f"style-ref {used!r} is not defined here")

    patterns = []
    for tag in ("match", "start", "end", "define-regex"):
        for element in doc.getElementsByTagName(tag):
            if element.firstChild:
                patterns.append(element.firstChild.nodeValue.replace("\\%{ident}", "X"))
    print(f"  {path:<52} {compile_all(patterns, path)} regexes")


def check_micro(path):
    if yaml is None:
        print(f"  {path:<52} skipped (no PyYAML)")
        return
    doc = load_yaml(path)
    if doc is None:
        return
    patterns = []

    def walk(node):
        if isinstance(node, dict):
            for value in node.values():
                if isinstance(value, str):
                    patterns.append(value)
                else:
                    walk(value)
        elif isinstance(node, list):
            for item in node:
                walk(item)

    walk(doc["rules"])
    walk(doc["detect"])
    for pattern in patterns:
        if "(?=" in pattern or "(?!" in pattern or "(?<" in pattern:
            fail(path, f"lookaround is not supported by Go RE2: {pattern[:60]!r}")
    count = compile_all(patterns, path)
    print(f"  {path:<52} {count} regexes, {len(doc['rules'])} rules")


def check_nano(path):
    text = read_text(path)
    if not re.search(r"^syntax ", text, flags=re.MULTILINE):
        fail(path, "no syntax directive")
    for line in text.splitlines():
        if line.startswith("color ") and line.count('"') < 2:
            fail(path, f"unbalanced quoting: {line[:60]!r}")
    print(f"  {path:<52} {text.count(chr(10) + 'color ')} rules")


def check_plist(path):
    try:
        with open(path, "rb") as handle:
            data = plistlib.load(handle)
    except (OSError, plistlib.InvalidFileException, ExpatError) as error:
        fail(path, str(error))
        return
    if "scope" not in data:
        fail(path, "no scope")
    print(f"  {path:<52} {data.get('name')}")


def check_ini(path, required_sections):
    parser = configparser.ConfigParser(strict=False, interpolation=None)
    try:
        parser.read(path, encoding="utf-8")
    except (OSError, configparser.Error) as error:
        fail(path, str(error))
        return
    for section in required_sections:
        if section not in parser:
            fail(path, f"no [{section}] section")
    print(f"  {path:<52} {len(parser.sections())} sections")


def main():
    if not os.path.isdir(ROOT):
        print("run me from the repository root", file=sys.stderr)
        return 2

    print("editor syntax files")
    check_textmate("extensions/vscode/syntaxes/salam.tmLanguage.json")
    check_sublime_syntax("extensions/sublime/Salam.sublime-syntax")
    check_kate("extensions/kate/salam.xml")
    check_gtksourceview("extensions/gtksourceview/salam.lang")
    check_micro("extensions/micro/salam.yaml")
    check_nano("extensions/nano/salam.nanorc")
    check_ini("extensions/geany/filetypes.Salam.conf", ("keywords", "settings"))

    for path in ("extensions/notepadpp/salam.xml", "extensions/jetbrains/Salam.xml"):
        if load_xml(path) is not None:
            print(f"  {path:<52} ok")

    for path in (
        "extensions/sublime/Comments.tmPreferences",
        "extensions/sublime/Indentation Rules.tmPreferences",
        "extensions/sublime/Symbol List.tmPreferences",
    ):
        check_plist(path)

    for path in (
        "extensions/sublime/Salam.sublime-build",
        "extensions/sublime/Salam.sublime-settings",
        "extensions/sublime/Default.sublime-commands",
        "extensions/sublime/Salam.sublime-completions",
        "extensions/sublime/messages.json",
        "extensions/vscode/package.json",
        "extensions/vscode/language-configuration.json",
        "extensions/vscode/snippets/salam.code-snippets",
    ):
        if load_json(path, strip_comments=True) is not None:
            print(f"  {path:<52} ok")

    if failures:
        print(f"\n{len(failures)} problem(s):")
        for message in failures:
            print(f"  {message}")
        return 1
    print("\nall editor syntax files are valid")
    return 0


if __name__ == "__main__":
    sys.exit(main())

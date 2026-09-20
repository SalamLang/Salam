#!/usr/bin/env python3
"""Check that every generated editor syntax file parses and that its
regexes compile. Run from the repository root."""

import json
import os
import re
import sys
import xml.dom.minidom

try:
    import regex
except ImportError:
    regex = re

try:
    import yaml
except ImportError:
    yaml = None

ROOT = "extensions"
failures = []


def fail(where, message):
    failures.append("%s: %s" % (where, message))


def to_python_regex(pattern):
    return re.sub(r"\\x\{([0-9A-Fa-f]+)\}",
                  lambda m: "\\u%04x" % int(m.group(1), 16), pattern)


def compile_all(patterns, where):
    count = 0
    for pattern in patterns:
        if not pattern:
            continue
        count += 1
        try:
            regex.compile(to_python_regex(pattern))
        except Exception as exc:
            fail(where, "regex %r: %s" % (pattern[:70], exc))
    return count


def check_json(path, strip_comments=False):
    text = open(path, encoding="utf-8").read()
    if strip_comments:
        text = re.sub(r"^\s*//.*$", "", text, flags=re.M)
    try:
        return json.loads(text)
    except Exception as exc:
        fail(path, str(exc))
        return None


def check_xml(path):
    try:
        return xml.dom.minidom.parse(path)
    except Exception as exc:
        fail(path, str(exc))
        return None


def check_textmate(path):
    grammar = check_json(path)
    if grammar is None:
        return
    patterns = []

    def walk(node):
        if isinstance(node, dict):
            for key, value in node.items():
                if key in ("match", "begin", "end") and isinstance(value, str):
                    patterns.append(value)
                elif key == "include" and isinstance(value, str) and value.startswith("#"):
                    if value[1:] not in grammar["repository"]:
                        fail(path, "include of unknown repository key %r" % value)
                else:
                    walk(value)
        elif isinstance(node, list):
            for item in node:
                walk(item)

    walk(grammar["patterns"])
    walk(grammar["repository"])
    print("  %-52s %d regexes" % (path, compile_all(patterns, path)))


def check_sublime_syntax(path):
    if yaml is None:
        print("  %-52s skipped (no PyYAML)" % path)
        return
    try:
        doc = yaml.safe_load(open(path, encoding="utf-8"))
    except Exception as exc:
        fail(path, str(exc))
        return

    variables = doc.get("variables", {})

    def expand(text, depth=0):
        if depth > 10:
            raise RuntimeError("variable recursion")
        return re.sub(r"\{\{(\w+)\}\}",
                      lambda m: expand(variables[m.group(1)], depth + 1), text)

    contexts = doc["contexts"]
    patterns = []

    def walk(node, where):
        if isinstance(node, list):
            for item in node:
                walk(item, where)
        elif isinstance(node, dict):
            if "include" in node and node["include"] not in contexts:
                fail(path, "%s includes unknown context %r" % (where, node["include"]))
            if "match" in node:
                patterns.append(expand(node["match"]))
            for key in ("push", "set"):
                if key in node:
                    if isinstance(node[key], str):
                        if node[key] not in contexts:
                            fail(path, "%s %s unknown context %r" % (where, key, node[key]))
                    else:
                        walk(node[key], where)

    for name, body in contexts.items():
        walk(body, name)
    print("  %-52s %d regexes" % (path, compile_all(patterns, path)))


def check_kate(path):
    doc = check_xml(path)
    if doc is None:
        return
    names = {c.getAttribute("name") for c in doc.getElementsByTagName("context")}
    for element in doc.getElementsByTagName("*"):
        target = element.getAttribute("context")
        if target and not target.startswith("#") and target not in names:
            fail(path, "rule points at unknown context %r" % target)
    lists = {l.getAttribute("name") for l in doc.getElementsByTagName("list")}
    for keyword in doc.getElementsByTagName("keyword"):
        used = keyword.getAttribute("String")
        if used not in lists:
            fail(path, "keyword rule uses unknown list %r" % used)
    attributes = {i.getAttribute("name") for i in doc.getElementsByTagName("itemData")}
    for element in doc.getElementsByTagName("*"):
        used = element.getAttribute("attribute")
        if used and used not in attributes:
            fail(path, "rule uses unknown itemData %r" % used)
    patterns = [e.getAttribute("String") for e in doc.getElementsByTagName("RegExpr")]
    print("  %-52s %d regexes, %d lists" % (path, compile_all(patterns, path), len(lists)))


def check_gtksourceview(path):
    doc = check_xml(path)
    if doc is None:
        return
    ids = {c.getAttribute("id") for c in doc.getElementsByTagName("context") if c.getAttribute("id")}
    for reference in doc.getElementsByTagName("context"):
        target = reference.getAttribute("ref")
        if target and ":" not in target and target not in ids:
            fail(path, "context ref %r is not defined here" % target)
    styles = {s.getAttribute("id") for s in doc.getElementsByTagName("style")}
    for context in doc.getElementsByTagName("context"):
        used = context.getAttribute("style-ref")
        if used and ":" not in used and used not in styles:
            fail(path, "style-ref %r is not defined here" % used)
    patterns = []
    for tag in ("match", "start", "end", "define-regex"):
        for element in doc.getElementsByTagName(tag):
            if element.firstChild:
                patterns.append(element.firstChild.nodeValue.replace("\\%{ident}", "X"))
    print("  %-52s %d regexes" % (path, compile_all(patterns, path)))


def check_micro(path):
    if yaml is None:
        print("  %-52s skipped (no PyYAML)" % path)
        return
    try:
        doc = yaml.safe_load(open(path, encoding="utf-8"))
    except Exception as exc:
        fail(path, str(exc))
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
            fail(path, "lookaround is not supported by Go RE2: %r" % pattern[:60])
    print("  %-52s %d regexes, %d rules" % (path, compile_all(patterns, path), len(doc["rules"])))


def check_nano(path):
    text = open(path, encoding="utf-8").read()
    if not re.search(r"^syntax ", text, flags=re.M):
        fail(path, "no syntax directive")
    for line in text.splitlines():
        if line.startswith("color ") and line.count('"') < 2:
            fail(path, "unbalanced quoting: %r" % line[:60])
    print("  %-52s %d rules" % (path, text.count("\ncolor ")))


def check_plist(path):
    import plistlib
    try:
        with open(path, "rb") as handle:
            data = plistlib.load(handle)
    except Exception as exc:
        fail(path, str(exc))
        return
    if "scope" not in data:
        fail(path, "no scope")
    print("  %-52s %s" % (path, data.get("name")))


def check_ini(path, required_sections):
    import configparser
    parser = configparser.ConfigParser(strict=False, interpolation=None)
    try:
        parser.read(path, encoding="utf-8")
    except Exception as exc:
        fail(path, str(exc))
        return
    for section in required_sections:
        if section not in parser:
            fail(path, "no [%s] section" % section)
    print("  %-52s %d sections" % (path, len(parser.sections())))


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
        if check_xml(path) is not None:
            print("  %-52s ok" % path)

    for path in ("extensions/sublime/Comments.tmPreferences",
                 "extensions/sublime/Indentation Rules.tmPreferences",
                 "extensions/sublime/Symbol List.tmPreferences"):
        check_plist(path)

    for path in ("extensions/sublime/Salam.sublime-build",
                 "extensions/sublime/Salam.sublime-settings",
                 "extensions/sublime/Default.sublime-commands",
                 "extensions/sublime/messages.json",
                 "extensions/vscode/package.json",
                 "extensions/vscode/language-configuration.json",
                 "extensions/vscode/snippets/salam.code-snippets"):
        if check_json(path, strip_comments=True) is not None:
            print("  %-52s ok" % path)
    if check_json("extensions/sublime/Salam.sublime-completions", strip_comments=True) is not None:
        print("  %-52s ok" % "extensions/sublime/Salam.sublime-completions")

    if failures:
        print("\n%d problem(s):" % len(failures))
        for message in failures:
            print("  " + message)
        return 1
    print("\nall editor syntax files are valid")
    return 0


if __name__ == "__main__":
    sys.exit(main())

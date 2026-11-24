import glob
import re
import sys

FILES = ["compiler/driver/driver.salam", "compiler/driver/driver_api_web.salam"]
FILES += sorted(glob.glob("compiler/jsgen/*.salam"))

SCRIPTS = [
    "compiler/sal_layout/layout_webstate.salam",
    "compiler/sal_layout/layout_style_js.salam",
    "compiler/docgen/docgen.salam",
    "compiler/driver/driver_js.salam",
    "std/net/http/swagger/ui.salam",
    "agent/salam/browser.salam",
]
SCRIPTS += sorted(glob.glob("std/layoutgen/*.salam"))

LIT = re.compile(r'"(?:[^"\\\n]|\\.)*"|`[^`]*`')
BLOCK = re.compile(r"`[^`]*`")
BAD = re.compile(r"\.([A-Z][A-Za-z0-9_]*)\(")

JS_BUILTINS = {"UTC", "DateTimeFormat", "NumberFormat"}

bad = 0


def read(path):
    global bad
    try:
        return open(path, encoding="utf-8").read()
    except OSError:
        print("%s: cannot read the file" % path)
        bad += 1
        return None


for path in FILES:
    text = read(path)
    if text is None:
        continue
    for lineno, line in enumerate(text.split("\n"), 1):
        for lit in LIT.findall(line):
            for m in BAD.finditer(lit):
                if m.group(1) in JS_BUILTINS:
                    continue
                print("%s:%d: capitalised method '%s' in emitted JavaScript: %s" % (path, lineno, m.group(1), m.group(0)))
                bad += 1

for path in SCRIPTS:
    text = read(path)
    if text is None:
        continue
    for block in BLOCK.finditer(text):
        for m in BAD.finditer(block.group(0)):
            if m.group(1) in JS_BUILTINS:
                continue
            lineno = text.count("\n", 0, block.start() + m.start()) + 1
            print("%s:%d: capitalised method '%s' in an embedded script: %s" % (path, lineno, m.group(1), m.group(0)))
            bad += 1

sys.exit(1 if bad else 0)

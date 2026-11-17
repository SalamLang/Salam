import glob
import re
import sys

FILES = ["compiler/driver/driver.salam", "compiler/driver/driver_api_web.salam"]
FILES += sorted(glob.glob("compiler/jsgen/*.salam"))

LIT = re.compile(r'"(?:[^"\\\n]|\\.)*"|`[^`]*`')
BAD = re.compile(r"\.([A-Z][A-Za-z0-9_]*)\(")

bad = 0
for path in FILES:
    try:
        text = open(path, encoding="utf-8").read()
    except OSError:
        print("%s: cannot read the file" % path)
        bad += 1
        continue
    for lineno, line in enumerate(text.split("\n"), 1):
        for lit in LIT.findall(line):
            for m in BAD.finditer(lit):
                print("%s:%d: capitalised method '%s' in emitted JavaScript: %s" % (path, lineno, m.group(1), m.group(0)))
                bad += 1
sys.exit(1 if bad else 0)

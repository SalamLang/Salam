import re
import sys

FILES = [
    "compiler/driver/driver.salam",
    "compiler/driver/driver_api_web.salam",
]
import glob
FILES += sorted(glob.glob("compiler/jsgen/*.salam"))

NAMES = (
    "Add|Remove|Slice|Split|Trim|Push|Pop|Includes|Repeat|Has|Set|Get|Concat|"
    "Write|Read|Join|Map|Filter|Keys|Values|Delete|Clear|Shift|Unshift|Sort|"
    "Reverse|Fill|Find|IndexOf|StartsWith|EndsWith|Replace|Substring|ToString|"
    "Contains|Focus|Click|Close|Append|Insert"
)
LIT = re.compile(r'"(?:[^"\\\n]|\\.)*"|`[^`]*`')
BAD = re.compile(r"[A-Za-z_\]\)]\.(" + NAMES + r")\(")

bad = 0
for path in FILES:
    try:
        text = open(path, encoding="utf-8").read()
    except OSError:
        continue
    for lineno, line in enumerate(text.split("\n"), 1):
        for lit in LIT.findall(line):
            m = BAD.search(lit)
            if m:
                print("%s:%d: capitalised JavaScript method in emitted code: %s" % (path, lineno, m.group(0)))
                bad += 1
sys.exit(1 if bad else 0)

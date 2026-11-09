import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from geo_ref import counts

DUMP = os.environ.get("INTEROP_GEO_DUMP", "geo_dump")
GEO = os.environ.get("INTEROP_GEO_DIR", "")

CASES = (
    ("site", "ls/geosite.dat", ("cn", "google", "geolocation-cn", "private")),
    ("ip", "ls/geoip.dat", ("ir", "cn", "private", "us")),
    ("site", "ir/geosite.dat", ("ir", "category-ir")),
    ("ip", "ir/geoip.dat", ("ir", "private", "cn")),
    ("site", "v2/dlc.dat", ("google", "cn")),
)


def main():
    if not GEO:
        print("set INTEROP_GEO_DIR to a directory holding ls/, ir/ and v2/ with the dat files")
        sys.exit(2)
    bad = 0
    for kind, rel, names in CASES:
        path = os.path.join(GEO, rel)
        ref = counts(path, kind)
        out = subprocess.run([DUMP, kind, path, "x"] + list(names), capture_output=True, timeout=300).stdout.decode().splitlines()
        got = {}
        for line in out:
            p = line.split()
            if len(p) > 2 and p[1] == "count":
                got[p[0]] = {p[i]: int(p[i + 1]) for i in range(3, len(p) - 1, 2)}
        for n in names:
            want = ref.get(n, {})
            have = got.get(n, {})
            if kind == "site":
                want = {k: v for k, v in want.items() if v}
                have = {k: v for k, v in have.items() if v}
            else:
                have = {"v4": have.get("v4entries", 0), "v6": have.get("v6entries", 0)}
            ok = want == have
            bad += 0 if ok else 1
            print(("PASS " if ok else "FAIL ") + "%s %s %s: reference %s, ours %s" % (kind, rel, n, want, have))
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()

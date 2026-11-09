import sys


def varint(b, i):
    r = 0
    s = 0
    while True:
        c = b[i]
        i += 1
        r |= (c & 0x7F) << s
        s += 7
        if c < 0x80:
            return r, i


def fields(b):
    i = 0
    n = len(b)
    while i < n:
        k, i = varint(b, i)
        f = k >> 3
        w = k & 7
        if w == 0:
            v, i = varint(b, i)
        elif w == 2:
            ln, i = varint(b, i)
            v = b[i : i + ln]
            i += ln
        elif w == 1:
            v = b[i : i + 8]
            i += 8
        elif w == 5:
            v = b[i : i + 4]
            i += 4
        else:
            raise SystemExit("bad wire type")
        yield f, w, v


def counts(path, kind):
    data = open(path, "rb").read()
    out = {}
    for f, w, v in fields(data):
        if f != 1:
            continue
        code = None
        cnt = {}
        for f2, w2, v2 in fields(v):
            if f2 == 1:
                code = v2.decode()
            elif f2 == 2:
                if kind == "site":
                    t = 0
                    for f3, w3, v3 in fields(v2):
                        if f3 == 1:
                            t = v3
                    name = {0: "keywords", 1: "regexes", 2: "suffix", 3: "full"}[t]
                    cnt[name] = cnt.get(name, 0) + 1
                else:
                    ip = b""
                    for f3, w3, v3 in fields(v2):
                        if f3 == 1:
                            ip = v3
                    name = "v6" if len(ip) == 16 else "v4"
                    cnt[name] = cnt.get(name, 0) + 1
        out[code.lower()] = cnt
    return out


if __name__ == "__main__":
    c = counts(sys.argv[1], sys.argv[2])
    for name in sys.argv[3:]:
        print(name, c.get(name))
    print("categories", len(c))

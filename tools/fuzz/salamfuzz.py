#!/usr/bin/env python3
"""Random differential testing of Salam backends.

Generates small, well-defined Salam programs (no overflow, no division by
zero, no out-of-range indexing), builds each with a reference backend and a
backend under test, runs both and reports any program whose output differs.

Usage: salamfuzz.py <salam> [--count N] [--seed S] [--target TRIPLE --runner CMD]
                   [--ref c|llvm] [--keep DIR]
"""

import argparse
import os
import random
import shutil
import subprocess
import sys
import tempfile

M = 1000003


class Gen:
    def __init__(self, rng):
        self.r = rng

    def leaf(self, vs):
        r = self.r.random()
        if r < 0.6 and vs:
            return self.r.choice(vs)
        k = self.r.choice([0, 1, 2, 3, 5, 7, 13, 255, 1000, 65535, -1, -7])
        return f"({k} as i64)" if k >= 0 else f"(0 - {-k} as i64)"

    def expr(self, vs, d):
        if d <= 0 or self.r.random() < 0.25:
            return self.leaf(vs)
        a = self.expr(vs, d - 1)
        b = self.expr(vs, d - 1)
        k = self.r.randrange(13)
        if k == 0:
            return f"(({a} + {b}) % {M})"
        if k == 1:
            return f"(({a} - {b}) % {M})"
        if k == 2:
            return f"((({a} % 46337) * ({b} % 46337)) % {M})"
        if k == 3:
            return f"({a} / (({b} % 7) + 8))"
        if k == 4:
            return f"({a} % (({b} % 11) + 13))"
        if k == 5:
            return f"(({a} & 65535) << ({b} & 15))"
        if k == 6:
            return f"({a} >> ({b} & 31))"
        if k == 7:
            return f"({a} & {b})"
        if k == 8:
            return f"({a} | {b})"
        if k == 9:
            return f"({a} ^ {b})"
        if k == 10:
            c = self.expr(vs, d - 1)
            op = self.r.choice(["<", "<=", ">", ">=", "==", "!="])
            return f"({a} {op} {b} ? {c} : {a})"
        if k == 11:
            return f"(({a} as i32) as i64)"
        return f"((({a} as u8) as i64) + (({b} as i16) as i64))"

    def block(self, vs, depth, lines, ind, loopvar):
        n = self.r.randint(1, 4)
        for _ in range(n):
            k = self.r.randrange(9)
            sp = "    " * ind
            use = vs + ([loopvar] if loopvar else [])
            if k <= 3 or depth <= 0:
                t = self.r.choice(["x", "y", "z"])
                lines.append(f"{sp}{t} = {self.expr(use, 3)}")
            elif k == 4:
                lines.append(f"{sp}if {self.expr(use, 2)} + x > {self.expr(use, 2)}:")
                self.block(vs, depth - 1, lines, ind + 1, loopvar)
                lines.append(f"{sp}else:")
                self.block(vs, depth - 1, lines, ind + 1, loopvar)
                lines.append(f"{sp}end")
            elif k == 5 and loopvar is None:
                lv = f"i{ind}"
                cnt = self.r.choice([1, 3, 4, 5, 8, 9, 16, 17, 33])
                lines.append(f"{sp}repeat {cnt} in {lv}:")
                lines.append(f"{sp}    z = (z + ({lv} as i64)) % {M}")
                self.block(vs, depth - 1, lines, ind + 1, f"({lv} as i64)")
                lines.append(f"{sp}end")
            elif k == 6:
                lines.append(f"{sp}v.push({self.expr(use, 2)})")
            elif k == 7:
                ix = self.expr(use, 2)
                lines.append(f"{sp}if v.len() > 0:")
                lines.append(f"{sp}    v.set(((({ix} % (v.len() as i64)) + (v.len() as i64)) % (v.len() as i64)) as int, {self.expr(use, 2)})")
                lines.append(f"{sp}end")
            else:
                lines.append(f"{sp}each e in v:")
                lines.append(f"{sp}    y = (y + e * 3) % {M}")
                lines.append(f"{sp}end")

    def func(self, idx):
        lines = [f"noinline func f{idx}(a: i64, b: i64): i64:",
                 "    mut x := a",
                 "    mut y := b",
                 "    mut z := a ^ b",
                 "    mut v := Vector {} as Vector<i64>",
                 "    v.push(a)"]
        self.block(["x", "y", "z", "a", "b"], 3, lines, 1, None)
        lines += ["    x = x + 1",
                  "    y = y + 1",
                  "    z = z + 1",
                  "    v.push(0 as i64)",
                  "    mut s := x * 7 + y * 3 + z",
                  "    each w in v:",
                  f"        s = (s * 31 + w) % {M}",
                  "    end",
                  "    v.free()",
                  "    ret s",
                  "end"]
        return "\n".join(lines)

    def kernel(self, idx):
        """A loop over vectors in the shapes the loop vectorizer handles."""
        ty, lim = self.r.choice([("int", 1000), ("u8", 256), ("i16", 1000), ("i64", 100000)])

        def to(e):
            return e if ty == "i64" else f"(({e}) as {ty})"

        def wide(e):
            return e if ty == "i64" else f"({e} as i64)"

        def lit(v):
            return f"({v} as {ty})"

        n = self.r.choice([0, 1, 3, 4, 7, 8, 15, 16, 17, 31, 33, 64])
        lines = [f"noinline func k{idx}(a: i64, b: i64): i64:",
                 f"    n := {n} + ((a - a) as int)",
                 f"    mut x := Vector {{}} as Vector<{ty}>",
                 f"    mut y := Vector {{}} as Vector<{ty}>",
                 f"    mut o := Vector {{}} as Vector<{ty}>",
                 "    repeat n in i:",
                 f"        x.push({to(f'((a + (i as i64) * 7919) % {lim} + {lim}) % {lim}')})",
                 f"        y.push({to(f'((b + (i as i64) * 104729) % {lim} + {lim}) % {lim}')})",
                 f"        o.push({lit(0)})",
                 "    end"]
        k = self.r.randrange(7)
        if k == 0:
            op = self.r.choice(["+", "-", "&", "|", "^"])
            lines += ["    repeat n in j:", f"        o.set(j, x.get(j) {op} y.get(j))", "    end"]
        elif k == 1 and ty != "u8":
            lines += ["    repeat n in j:", f"        o.set(j, x.get(j) * {lit(3)} - y.get(j))", "    end"]
        elif k == 2:
            cmp = self.r.choice(["<", "<=", ">", ">=", "==", "!="])
            lines += ["    repeat n in j:", f"        o.set(j, x.get(j) {cmp} y.get(j) ? x.get(j) : y.get(j))", "    end"]
        elif k == 3:
            lines += ["    mut m := 0 as i64", "    repeat n in j:",
                      f"        if {wide('x.get(j)')} > m:", f"            m = {wide('x.get(j)')}", "        end", "    end",
                      f"    o.push({to('m')})"]
        elif k == 4:
            lines += ["    mut t := 0 as i64", "    repeat n in j:",
                      f"        t = t + {wide('x.get(j)')} + {wide('y.get(j)')}", "    end",
                      f"    o.push({to(f'(t % {lim})')})"]
        elif k == 5 and ty in ("int", "i64"):
            sh = self.r.randint(1, 3)
            lines += ["    repeat n in j:", f"        o.set(j, (x.get(j) >> {lit(sh)}) + (y.get(j) << {lit(sh)}))", "    end"]
        else:
            lines += ["    mut jj := 0", "    until jj <= n - 1:", "        o.set(jj, x.get(jj) & y.get(jj))", "        jj = jj + 1", "    end"]
        lines += ["    mut s := 0 as i64",
                  "    each v in o:",
                  f"        s = (s * 31 + {wide('v')}) % {M}",
                  "    end",
                  "    x.free()", "    y.free()", "    o.free()",
                  "    ret s",
                  "end"]
        return "\n".join(lines)

    def floats(self, idx):
        """f64 arithmetic, clamped before the conversion back to an integer."""
        lines = [f"noinline func d{idx}(a: i64, b: i64): i64:",
                 "    mut p := ((a % 1000) as f64) * 0.5 + 1.25",
                 "    mut q := ((b % 1000) as f64) * 0.25 - 3.5"]
        for _ in range(self.r.randint(2, 6)):
            t = self.r.choice(["p", "q"])
            o = self.r.choice(["p + q", "p - q", "p * 0.75 + q", "q / (p * p + 1.0)", "p > q ? p - 1.5 : q + 2.25", "(p + q) * 0.5"])
            lines.append(f"    {t} = {o}")
            lines.append(f"    {t} = {t} > 100000.0 ? 100000.0 : ({t} < 0.0 - 100000.0 ? 0.0 - 100000.0 : {t})")
        lines += ["    p = p + 0.5", "    q = q - 0.25",
                  "    ret ((p * 1000.0) as i64) + ((q * 1000.0) as i64)", "end"]
        return "\n".join(lines)

    def calls(self, idx):
        """Eight arguments (some on the stack on every ABI) and a struct by reference."""
        e = " + ".join(f"{self.r.choice([1, 3, 5, 7])} * v{i}" for i in range(8))
        return "\n".join([
            f"noinline func h{idx}(acc &: Acc, v0: i64, v1: i64, v2: i64, v3: i64, v4: i64, v5: i64, v6: i64, v7: i64): i64:",
            f"    acc.x = (acc.x + {e}) % {M}",
            "    acc.y = acc.y + 1",
            "    ret acc.x * 3 + acc.y",
            "end",
            "",
            f"noinline func c{idx}(a: i64, b: i64): i64:",
            "    mut acc := Acc {}",
            "    mut r := 0 as i64",
            "    repeat 3 in i:",
            "        r = r + h" + str(idx) + "(acc, a, b, a + (i as i64), b - 1, a * 2, b * 3, a ^ b, (i as i64))",
            "    end",
            f"    ret (r + acc.x + acc.y) % {M}",
            "end"])

    def program(self, nf):
        parts = ["struct Acc:\n    pub x: i64 = 0\n    pub y: i64 = 0\nend"]
        names = []
        for k in range(nf):
            parts.append(self.func(k))
            names.append(f"f{k}")
            kind = self.r.randrange(3)
            if kind == 0:
                parts.append(self.kernel(k))
                names.append(f"k{k}")
            elif kind == 1:
                parts.append(self.floats(k))
                names.append(f"d{k}")
            else:
                parts.append(self.calls(k))
                names.append(f"c{k}")
        main = ["func main:"]
        for nm in names:
            for (a, b) in [(0, 0), (1, 2), (-5, 7), (123456, -987), (65535, 3)]:
                ea = f"({a} as i64)" if a >= 0 else f"(0 - {-a} as i64)"
                eb = f"({b} as i64)" if b >= 0 else f"(0 - {-b} as i64)"
                main.append(f"    println {nm}({ea}, {eb})")
        main.append("end")
        return "\n\n".join(parts) + "\n\n" + "\n".join(main) + "\n"


def run(cmd, cwd, timeout=120):
    try:
        p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, timeout=timeout)
        return p.returncode, p.stdout, p.stderr
    except subprocess.TimeoutExpired:
        return -999, "", "timeout"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("salam")
    ap.add_argument("--count", type=int, default=50)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--funcs", type=int, default=3)
    ap.add_argument("--ref", default="c")
    ap.add_argument("--target", default="")
    ap.add_argument("--runner", default="")
    ap.add_argument("--keep", default="")
    a = ap.parse_args()
    salam = os.path.abspath(a.salam)
    work = tempfile.mkdtemp(prefix="salamfuzz.")
    bad = 0
    for n in range(a.count):
        seed = a.seed + n
        src = Gen(random.Random(seed)).program(a.funcs)
        d = os.path.join(work, f"s{seed}")
        os.makedirs(d, exist_ok=True)
        with open(os.path.join(d, "p.salam"), "w") as f:
            f.write(src)
        rc, _, err = run([salam, "build", "p.salam", f"--backend={a.ref}", "--release", "--output=ref", "--log-level=error"], d)
        if rc != 0:
            bad += 1
            print(f"FAIL seed {seed}: reference build failed: {err.strip()[:300]}")
            continue
        tgt = [f"--target={a.target}"] if a.target else []
        rc, _, err = run([salam, "build", "p.salam", "--backend=native", "--release", "--output=nat", "--log-level=error"] + tgt, d)
        if rc != 0:
            bad += 1
            print(f"FAIL seed {seed}: native build failed: {err.strip()[:300]}")
            continue
        rc1, out1, err1 = run(["./ref"], d)
        runner = a.runner.split() if a.runner else []
        rc2, out2, err2 = run(runner + ["./nat"], d)
        if rc1 != 0 or rc2 != 0:
            bad += 1
            print(f"FAIL seed {seed}: run failed, exit {rc1} vs {rc2}: {(err1 if rc1 else err2).strip()[:200]}")
        elif out1 != out2:
            bad += 1
            print(f"DIFF seed {seed}: output differs")
        if (rc1 != 0 or rc2 != 0 or out1 != out2) and a.keep:
            os.makedirs(a.keep, exist_ok=True)
            shutil.copy(os.path.join(d, "p.salam"), os.path.join(a.keep, f"diff_{seed}.salam"))
        shutil.rmtree(d, ignore_errors=True)
    shutil.rmtree(work, ignore_errors=True)
    print(f"salamfuzz: {a.count - bad} agree, {bad} failed")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
import csv
import json
import os
import shutil
import statistics
import subprocess
import sys
import time

ROOT = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(ROOT)
PROGRAMS_DIR = os.path.join(ROOT, "programs")
WORK = os.environ.get("BENCH_WORK", "/tmp/salam_bench_work")
WORK_PROGRAMS = os.path.join(WORK, "programs")
BUILD_DIR = os.path.join(WORK, "build")
RESULTS_DIR = os.path.join(ROOT, "results")
SALAM_BIN = os.environ.get("SALAM_BIN", os.path.join(REPO, "salam"))
SALAM_STD = os.environ.get("SALAM_STD", os.path.join(REPO, "compiler", "std"))
RUNS = int(os.environ.get("BENCH_RUNS", "10"))
TIMEOUT = 180

LANG_ORDER = ["c", "cpp", "salam_llvm", "salam_gcc", "salam_tcc", "php", "python"]
LANG_LABEL = {
    "c": "C (gcc -O3)",
    "cpp": "C++ (g++ -O3)",
    "salam_llvm": "Salam (llvm -O3)",
    "salam_gcc": "Salam (C backend, gcc -O3)",
    "salam_tcc": "Salam (tcc, default)",
    "php": "PHP",
    "python": "Python",
}


def sh(argv, cwd=None, env=None):
    return subprocess.run(argv, cwd=cwd, env=env, capture_output=True, text=True, timeout=TIMEOUT)


def salam_env():
    env = dict(os.environ)
    env["SALAM_STD"] = SALAM_STD
    return env


def build_program(name):
    src_dir = os.path.join(WORK_PROGRAMS, name)
    built = {}
    errors = {}

    c_bin = os.path.join(BUILD_DIR, name + "_c")
    r = sh(["gcc", "-O3", "-o", c_bin, os.path.join(src_dir, "prog.c"), "-lm"])
    if r.returncode == 0:
        built["c"] = [c_bin]
    else:
        errors["c"] = r.stderr[-2000:]

    cpp_bin = os.path.join(BUILD_DIR, name + "_cpp")
    r = sh(["g++", "-O3", "-o", cpp_bin, os.path.join(src_dir, "prog.cpp")])
    if r.returncode == 0:
        built["cpp"] = [cpp_bin]
    else:
        errors["cpp"] = r.stderr[-2000:]

    salam_src = os.path.join(src_dir, "prog.salam")

    tcc_bin = os.path.join(BUILD_DIR, name + "_salam_tcc")
    r = sh([SALAM_BIN, "build", salam_src, "--output=" + tcc_bin], cwd=BUILD_DIR, env=salam_env())
    if r.returncode == 0 and os.path.exists(tcc_bin):
        built["salam_tcc"] = [tcc_bin]
    else:
        errors["salam_tcc"] = (r.stdout + r.stderr)[-2000:]

    gcc_bin = os.path.join(BUILD_DIR, name + "_salam_gcc")
    predecl = os.path.join(WORK, "salam_predecl.h")
    r = sh([SALAM_BIN, "build", salam_src, "--cc=gcc -O3 -include " + predecl,
            "--output=" + gcc_bin], cwd=BUILD_DIR, env=salam_env())
    if r.returncode == 0 and os.path.exists(gcc_bin):
        built["salam_gcc"] = [gcc_bin]
    else:
        errors["salam_gcc"] = (r.stdout + r.stderr)[-2000:]

    llvm_bin = os.path.join(BUILD_DIR, name + "_salam_llvm")
    r = sh([SALAM_BIN, "llvm", salam_src, "-O3", "--exec", "-o", llvm_bin], cwd=BUILD_DIR, env=salam_env())
    if r.returncode == 0 and os.path.exists(llvm_bin):
        built["salam_llvm"] = [llvm_bin]
    else:
        errors["salam_llvm"] = (r.stdout + r.stderr)[-2000:]

    built["python"] = ["python3", os.path.join(src_dir, "prog.py")]
    built["php"] = ["php", "-n", os.path.join(src_dir, "prog.php")]
    return built, errors


def verify(name, cmds):
    outputs = {}
    for lang, argv in cmds.items():
        try:
            r = sh(argv)
            outputs[lang] = r.stdout.strip() if r.returncode == 0 else "<exit %d: %s>" % (r.returncode, r.stderr.strip()[:200])
        except subprocess.TimeoutExpired:
            outputs[lang] = "<timeout>"
    ref = outputs.get("c", "")
    mismatched = {l: o for l, o in outputs.items() if o != ref}
    return ref, mismatched


def bench(argv, runs):
    samples = []
    try:
        sh(argv)
        for _ in range(runs):
            t0 = time.time_ns()
            r = subprocess.run(argv, capture_output=True, timeout=TIMEOUT)
            t1 = time.time_ns()
            if r.returncode != 0:
                return None
            samples.append({"epoch_start_ns": t0, "epoch_end_ns": t1, "ms": (t1 - t0) / 1e6})
    except subprocess.TimeoutExpired:
        return None
    return samples


def stats(samples):
    ms = [s["ms"] for s in samples]
    return {
        "min_ms": min(ms),
        "max_ms": max(ms),
        "avg_ms": sum(ms) / len(ms),
        "median_ms": statistics.median(ms),
        "stdev_ms": statistics.stdev(ms) if len(ms) > 1 else 0.0,
        "runs": len(ms),
    }


def tool_versions():
    info = {}
    probes = {
        "gcc": ["gcc", "--version"],
        "g++": ["g++", "--version"],
        "python": ["python3", "--version"],
        "php": ["php", "--version"],
        "tcc": ["tcc", "-v"],
        "salam": [SALAM_BIN, "version"],
        "clang": ["clang", "--version"],
    }
    for k, argv in probes.items():
        try:
            r = sh(argv)
            info[k] = (r.stdout or r.stderr).strip().splitlines()[0]
        except Exception:
            info[k] = "unavailable"
    try:
        with open("/proc/cpuinfo") as f:
            for line in f:
                if line.startswith("model name"):
                    info["cpu"] = line.split(":", 1)[1].strip()
                    break
    except Exception:
        pass
    return info


def fmt_ms(v):
    if v is None:
        return "-"
    return "%.1f" % v


def geomean(values):
    if not values:
        return None
    p = 1.0
    for v in values:
        p *= v
    return p ** (1.0 / len(values))


def write_report(results, env_info, started_epoch, finished_epoch):
    lines = []
    lines.append("# Salam Benchmark Suite Results\n")
    lines.append("Ten CPU-bound programs implemented identically in C, Python, PHP, and Salam.")
    lines.append("Each variant is run once as warm-up, then %d timed runs; times are wall-clock" % RUNS)
    lines.append("per full process (start/end captured as Unix-epoch nanoseconds).\n")
    lines.append("- Started (epoch ns): %d" % started_epoch)
    lines.append("- Finished (epoch ns): %d" % finished_epoch)
    lines.append("")
    lines.append("## Environment\n")
    for k in ["cpu", "gcc", "g++", "tcc", "clang", "python", "php", "salam"]:
        if k in env_info:
            lines.append("- **%s**: %s" % (k, env_info[k]))
    lines.append("")
    lines.append("Salam is measured three ways: the LLVM backend at -O3, the C backend")
    lines.append("compiled by gcc -O3 (with `salam_predecl.h` forward declarations to work")
    lines.append("around the generated-header ordering issue), and the default tcc toolchain")
    lines.append("(no optimizer) as the out-of-the-box reference. C and C++ run at -O3 too.\n")

    lines.append("## Per-program results (milliseconds, %d runs)\n" % RUNS)
    for name in sorted(results):
        prog = results[name]
        lines.append("### %s\n" % name)
        if prog.get("mismatched"):
            lines.append("Output mismatches vs C reference: `%s`\n" % json.dumps(prog["mismatched"]))
        lines.append("Reference output: `%s`\n" % prog["reference_output"].replace("\n", " / "))
        lines.append("| Language | Min | Avg | Max | Median | Stdev | vs C (avg) |")
        lines.append("|---|---|---|---|---|---|---|")
        c_avg = prog["langs"].get("c", {}).get("stats", {}).get("avg_ms")
        for lang in LANG_ORDER:
            entry = prog["langs"].get(lang)
            if not entry or not entry.get("stats"):
                reason = prog.get("errors", {}).get(lang, "run failed")
                lines.append("| %s | failed | | | | | %s |" % (LANG_LABEL[lang], reason.splitlines()[-1][:80] if reason else ""))
                continue
            st = entry["stats"]
            ratio = "%.2fx" % (st["avg_ms"] / c_avg) if c_avg else "-"
            lines.append("| %s | %s | %s | %s | %s | %s | %s |" % (
                LANG_LABEL[lang], fmt_ms(st["min_ms"]), fmt_ms(st["avg_ms"]), fmt_ms(st["max_ms"]),
                fmt_ms(st["median_ms"]), fmt_ms(st["stdev_ms"]), ratio))
        lines.append("")

    lines.append("## Summary (average ms per program)\n")
    header = "| Program | " + " | ".join(LANG_LABEL[l] for l in LANG_ORDER) + " |"
    lines.append(header)
    lines.append("|---" * (len(LANG_ORDER) + 1) + "|")
    for name in sorted(results):
        row = ["| " + name]
        for lang in LANG_ORDER:
            st = results[name]["langs"].get(lang, {}).get("stats")
            row.append(fmt_ms(st["avg_ms"]) if st else "failed")
        lines.append(" | ".join(row) + " |")
    lines.append("")

    lines.append("## Overall comparison (geometric mean of per-program avg ratio vs C)\n")
    lines.append("| Language | Slowdown vs C (%s) |" % LANG_LABEL["c"])
    lines.append("|---|---|")
    for lang in LANG_ORDER:
        ratios = []
        for name in results:
            c_avg = results[name]["langs"].get("c", {}).get("stats", {}).get("avg_ms")
            st = results[name]["langs"].get(lang, {}).get("stats")
            if c_avg and st:
                ratios.append(st["avg_ms"] / c_avg)
        g = geomean(ratios)
        lines.append("| %s | %s |" % (LANG_LABEL[lang], "%.2fx" % g if g else "-"))
    lines.append("")
    lines.append("Notes:")
    lines.append("- All timings include interpreter/process start-up, identical for every run.")
    lines.append("- Every program prints a deterministic result; all variants are checked against the C output before timing.")
    lines.append("- Raw per-run epoch timestamps are in `results/results.csv` and `results/results.json`.")
    lines.append("")

    with open(os.path.join(ROOT, "RESULTS.md"), "w") as f:
        f.write("\n".join(lines))


def main():
    only = sys.argv[1:] or None
    shutil.rmtree(WORK, ignore_errors=True)
    shutil.copytree(PROGRAMS_DIR, WORK_PROGRAMS)
    shutil.copy(os.path.join(ROOT, "salam_predecl.h"), WORK)
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.makedirs(RESULTS_DIR, exist_ok=True)
    names = sorted(d for d in os.listdir(PROGRAMS_DIR) if os.path.isdir(os.path.join(PROGRAMS_DIR, d)))
    if only:
        names = [n for n in names if n in only]

    started_epoch = time.time_ns()
    env_info = tool_versions()
    results = {}

    for name in names:
        print("[bench] %s: building" % name, flush=True)
        cmds, errors = build_program(name)
        ref, mismatched = verify(name, cmds)
        if mismatched:
            print("[bench] %s: OUTPUT MISMATCH %s" % (name, json.dumps(mismatched)), flush=True)
        prog = {"reference_output": ref, "mismatched": mismatched, "errors": errors, "langs": {}}
        for lang in LANG_ORDER:
            if lang not in cmds:
                continue
            if lang in mismatched:
                prog["langs"][lang] = {"stats": None, "samples": [], "note": "output mismatch"}
                continue
            print("[bench] %s: timing %s" % (name, lang), flush=True)
            samples = bench(cmds[lang], RUNS)
            if samples is None:
                prog["langs"][lang] = {"stats": None, "samples": []}
                print("[bench] %s: %s FAILED at runtime" % (name, lang), flush=True)
                continue
            st = stats(samples)
            prog["langs"][lang] = {"stats": st, "samples": samples}
            print("[bench] %s: %s min=%.1f avg=%.1f max=%.1f ms" % (name, lang, st["min_ms"], st["avg_ms"], st["max_ms"]), flush=True)
        results[name] = prog

    finished_epoch = time.time_ns()

    with open(os.path.join(RESULTS_DIR, "results.json"), "w") as f:
        json.dump({"started_epoch_ns": started_epoch, "finished_epoch_ns": finished_epoch,
                   "environment": env_info, "runs": RUNS, "results": results}, f, indent=2)

    with open(os.path.join(RESULTS_DIR, "results.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["program", "language", "run", "epoch_start_ns", "epoch_end_ns", "duration_ms"])
        for name in sorted(results):
            for lang in LANG_ORDER:
                entry = results[name]["langs"].get(lang)
                if not entry:
                    continue
                for i, s in enumerate(entry["samples"], 1):
                    w.writerow([name, lang, i, s["epoch_start_ns"], s["epoch_end_ns"], "%.3f" % s["ms"]])

    write_report(results, env_info, started_epoch, finished_epoch)
    print("[bench] done: RESULTS.md, results/results.json, results/results.csv", flush=True)


if __name__ == "__main__":
    main()

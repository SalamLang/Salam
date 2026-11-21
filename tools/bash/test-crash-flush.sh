#!/bin/sh
# Tests that a program killed by a fatal signal still prints the output it
# had buffered, says which signal killed it, and still dies by that signal,
# with stdout in a pipe, on the C, native and LLVM backends.

set -u

salam=${1:-}
root=$(CDPATH='' cd -- "$(dirname -- "$0")/../.." && pwd)
[ -n "$salam" ] || salam="$root/salam"
case "$salam" in /* | [A-Za-z]:*) ;; *) salam="$root/$salam" ;; esac

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT INT TERM
export SALAM_STD="$root/std"
cd "$work" || exit 1

cat >segv.salam <<'SRC'
func ptr(n: int): int*:
    ret (n as u64) as int*
end

func main:
    println "step 1 done"
    println "step 2 done"
    p := ptr(args().Len() - 1)
    println p[0]
end
SRC

cat >deep.salam <<'SRC'
func depth(n: i64, k: i64): i64:
    if n == 0:
        ret k
    end
    buf := [n, k, n + k, n * (3 as i64)]
    r := depth(n - (1 as i64), buf[(n % (4 as i64)) as int])
    ret r ^ buf[((n + (1 as i64)) % (4 as i64)) as int]
end

func main:
    println "start"
    println depth((args().Len() as i64) * (100000000 as i64), 1 as i64)
end
SRC

cat >run.py <<'PY'
import subprocess
import sys

p = subprocess.run([sys.argv[1]], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
sys.stdout.write(p.stdout.decode("utf-8", "replace"))
for line in p.stderr.decode("utf-8", "replace").splitlines():
    if not line.startswith("Segmentation fault"):
        print(line)
if p.returncode < 0:
    print("exit %d" % (128 - p.returncode))
else:
    print("exit-code %d" % p.returncode)
PY

for backend in c native llvm; do
    for prog in segv deep; do
        echo "== $backend $prog"
        if ! "$salam" build "$prog.salam" --backend="$backend" --output="$prog-$backend" --log-level=error >build.txt 2>&1; then
            cat build.txt
            continue
        fi
        python3 -I run.py "./$prog-$backend"
    done
done

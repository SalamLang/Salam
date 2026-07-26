#!/bin/bash
# Stage-8a driver gate (category a): dr.Build() (C backend) vs HEAD
# `bin/salam build <f> --output=x && ./x`. Run from repo root under WSL.
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD=$PWD/compiler

FILES=(
  compiler/tests/en/basics/hello.salam
  compiler/tests/en/basics/average.salam
  compiler/tests/en/basics/collatz.salam
  compiler/tests/en/basics/dec_to_bin.salam
  compiler/tests/en/basics/digit_sum.salam
  compiler/tests/en/basics/lcm.salam
  compiler/tests/en/basics/leap_year.salam
  compiler/tests/en/basics/minmax.salam
  compiler/tests/en/basics/palindrome.salam
  compiler/tests/en/basics/power_iter.salam
  compiler/tests/en/basics/prime_factors.salam
  compiler/tests/en/basics/range_sum.salam
  compiler/tests/en/basics/slices.salam
  compiler/tests/en/basics/sqrt_newton.salam
  compiler/tests/en/basics/star_triangle.salam
  compiler/tests/en/features/counter_closure.salam
  compiler/tests/en/features/default_args.salam
  compiler/tests/en/features/defer_early_ret.salam
  compiler/tests/en/features/defer_test.salam
  compiler/tests/en/features/func_modifiers.salam
  compiler/tests/en/features/impl_trait.salam
  compiler/tests/en/features/match.salam
  compiler/tests/en/features/mut_usage.salam
  compiler/tests/en/features/mutability.salam
  compiler/tests/en/features/packages.salam
  compiler/tests/en/features/style_commaless.salam
  compiler/tests/en/features/ternary.salam
  compiler/tests/en/features/variant.salam
  compiler/tests/en/general/generic_box.salam
  compiler/tests/en/general/funcaddr.salam
  compiler/tests/en/general/break_continue_complex.salam
  compiler/tests/en/general/generic_stack.salam
  compiler/tests/en/general/generic_option.salam
  compiler/tests/en/general/cast.salam
  compiler/tests/en/general/hashmap_iter.salam
  compiler/tests/en/general/hashmap.salam
  compiler/tests/en/general/charlit.salam
  compiler/tests/en/general/impl.salam
)

pass=0
fail=0
for f in "${FILES[@]}"; do
  if [ ! -f "$f" ]; then echo "SKIP (missing) $f"; continue; fi
  out="${f%.salam}.out"
  if [ ! -f "$out" ]; then echo "SKIP (no .out) $f"; continue; fi

  rm -f /tmp/dr_gate_a /tmp/dr_gate_b
  ./bin/salam build "$f" --output=/tmp/dr_gate_b >/tmp/dr_gate_b.buildlog 2>&1
  brc_b=$?
  /tmp/driver_run_bin build "$f" /tmp/dr_gate_a >/tmp/dr_gate_a.buildlog 2>&1
  brc_a=$?

  if [ $brc_b -ne 0 ] && [ $brc_a -ne 0 ]; then
    echo "SKIP (both fail to build) $f"
    continue
  fi
  if [ $brc_b -ne 0 ] || [ $brc_a -ne 0 ]; then
    echo "FAIL (build rc mismatch: head=$brc_b port=$brc_a) $f"
    fail=$((fail+1))
    continue
  fi

  out_b=$(/tmp/dr_gate_b 2>&1)
  out_a=$(/tmp/dr_gate_a 2>&1)
  expected=$(cat "$out")

  if [ "$out_a" == "$out_b" ] && [ "$out_a" == "$expected" ]; then
    echo "PASS $f"
    pass=$((pass+1))
  else
    echo "FAIL $f"
    echo "  expected: $expected"
    echo "  head:     $out_b"
    echo "  port:     $out_a"
    fail=$((fail+1))
  fi
done
echo "=== build-path gate: $pass passed, $fail failed (of $((pass+fail)) compared) ==="

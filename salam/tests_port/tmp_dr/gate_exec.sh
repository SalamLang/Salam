#!/bin/bash
# Stage-8a driver gate (category b): dr.Exec() (interpreter path) vs HEAD
# `bin/salam exec <f>`. Run from repo root under WSL.
set -u
cd /mnt/c/Users/Admin/Projects/SalamLang/Salam
export SALAM_STD=$PWD/compiler

FILES=(
  compiler/tests/en/exec/compound_assign.salam
  compiler/tests/en/exec/compound_assign_struct.salam
  compiler/tests/en/exec/console.salam
  compiler/tests/en/exec/constfold_eval.salam
  compiler/tests/en/exec/default_args.salam
  compiler/tests/en/exec/fnmods_exec.salam
  compiler/tests/en/exec/generic_default_args.salam
  compiler/tests/en/exec/incdec.salam
  compiler/tests/en/exec/match.salam
  compiler/tests/en/exec/math.salam
  compiler/tests/en/exec/mut_mutated_ok.salam
  compiler/tests/en/exec/mutability_tiers.salam
  compiler/tests/en/exec/prec_01_add_mul.salam
  compiler/tests/en/exec/prec_02_sub_div_mod.salam
  compiler/tests/en/exec/prec_03_div_mod_chain.salam
  compiler/tests/en/exec/prec_04_power_right_assoc.salam
  compiler/tests/en/exec/prec_05_unary_minus_power.salam
  compiler/tests/en/exec/prec_10_and_or.salam
  compiler/tests/en/exec/prec_12_assignment_lowest.salam
  compiler/tests/en/exec/prec_17_postfix_incdec.salam
)

pass=0
fail=0
for f in "${FILES[@]}"; do
  if [ ! -f "$f" ]; then echo "SKIP (missing) $f"; continue; fi
  out="${f%.salam}.out"
  if [ ! -f "$out" ]; then echo "SKIP (no .out) $f"; continue; fi

  out_b=$(./bin/salam exec "$f" 2>&1)
  out_a=$(/tmp/driver_run_bin exec "$f" 2>&1)
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
echo "=== exec-path gate: $pass passed, $fail failed (of $((pass+fail)) compared) ==="

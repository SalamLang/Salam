# Hardware AES and carry-less multiplication

`std/cpu` gives Salam programs the processor's AES and carry-less multiply
instructions. `std/tls` uses them for AES-GCM, so the VPN and proxy stack
(shadowsocks, vmess, vless, transports) and the TLS record layer get them with
no change in their own code.

## What is accelerated

| target                          | instructions                                 |
| ------------------------------- | -------------------------------------------- |
| x86-64 (Linux, macOS, Windows)  | AES-NI (`aesenc`, `aesenclast`), `pclmulqdq` |
| AArch64 (Linux, macOS)          | Crypto Extensions (`aese`, `aesmc`, `pmull`) |
| anything else, or an old CPU    | the existing table and bit-sliced code       |

AES-NI and PCLMULQDQ do not use data-dependent table lookups or branches, so
they are constant time. The TLS record layer therefore takes the hardware path
when it is available and keeps the bit-sliced code as its fallback.

## The mechanism

The package `cpu` exports a small set of functions on `u128`:

| function                    | meaning                                              |
| --------------------------- | ---------------------------------------------------- |
| `Features()`                | bit mask, 0 when the backend or the CPU has nothing  |
| `HwAesEnc(state, key)`      | one AES round, like `aesenc`                         |
| `HwAesEncLast(state, key)`  | the last AES round, like `aesenclast`                |
| `HwClmul(a, b, imm)`        | 64x64 carry-less multiply; `imm` as in `pclmulqdq`   |
| `HwSwap64(x)`               | swap the two 64-bit halves                           |
| `HwBswap(x)`                | reverse all 16 bytes                                 |
| `HwLoad(p)`, `HwStore(p,x)` | unaligned 16-byte load and store                     |

Every one of them has a plain Salam body in `std/cpu/cpu.salam`: a slow but
exact software model of the instruction. That body is what runs on every
backend that does not know the functions. The LLVM backend (`--backend=llvm`,
and every release build) recognises calls to them in `compiler/llvmgen/llvm_hw.salam`
and emits the instruction instead:

- x86-64: `llvm.x86.aesni.aesenc`, `llvm.x86.aesni.aesenclast`,
  `llvm.x86.pclmulqdq`; `Features()` runs `cpuid` (leaf 1) and reports the
  hardware when AES, PCLMULQDQ, SSSE3 and SSE4.1 are all present.
- AArch64: `llvm.aarch64.crypto.aese`/`aesmc` (with a zero key and an `eor`, to
  keep the x86 meaning of a round) and `llvm.aarch64.neon.pmull64`;
  `Features()` reads `AT_HWCAP` on Linux (`HWCAP_AES` and `HWCAP_PMULL`) and
  assumes the extensions on Apple silicon.
- Any function that contains such a call, directly or through a function it
  calls, gets `"target-features"="+aes,+pclmul,+ssse3,+sse4.1"` (or `+aes,+neon`
  on AArch64). The rest of the program is compiled for the baseline CPU, so the
  binary still starts on a machine without the extensions; only the guarded code
  needs them.
- `HwClmul` needs its third argument to be an integer literal, because the
  instruction takes it as an immediate.

The C backend, the native backend, the interpreter and the JavaScript backend do
not lower these calls. `Features()` returns 0 there, so `HasHardwareCrypto()` is
false and the software paths run. They still compile and give the same results.

## Choosing the path at runtime

`cpu.HasHardwareCrypto()` is true when the backend lowered the intrinsics, the
CPU has them, and the switch is on. The switch is
`cpu.SetHardware(true|false)` or the environment variable
`SALAM_NO_HWCRYPTO` (any non-empty value turns hardware off). A cipher context
reads the switch when it is created.

## The AES-GCM kernel

`std/tls/aeshw.salam` is the kernel. Round keys come from the existing scalar key
schedule (it runs once per key, so `aeskeygenassist` would not make anything
faster) and are stored as 16-byte blocks. Counter mode runs 8 blocks at a time,
then 4, then 1. GHASH multiplies 8 blocks per reduction with precomputed powers
of H, using four `pclmulqdq` per block and one reduction for the group. The file
is generated: the 8, 4 and 1 wide bodies are the same template unrolled.

## Seed compiler note

`std/tls` is compiled by the compiler itself, and now uses `u128` and the `cpu`
functions. A seed compiler that does not know `u128` (anything before 0.6.3)
cannot build it, so the change needs a release that carries the LLVM lowering
before `std/tls` can rely on it in the self-host chain.

## Tests and measurement

- `tests/en/stdlib/aes_hw_gcm.salam` seals and opens with the hardware switch on
  and off and compares the bytes, for both key sizes and sizes from 0 to 70000
  with tampering checks. `aes_fast_gcm` and `tls_selftest` pass on both paths.
- `tools/bench/aes_gcm_hw.salam` prints MB/s for AES-128-GCM, AES-256-GCM and
  ChaCha20-Poly1305 at 1 KB, 16 KB and 1 MB, with a checksum of the last output.
  Run it once normally and once with `SALAM_NO_HWCRYPTO=1`.

## Measured

Machine: AMD EPYC (virtual), AES-NI and PCLMULQDQ present, shared and loaded, so
wall-clock numbers are rough. Build: `--backend=llvm --release`. Instruction
counts come from callgrind and are the reliable figures: the cost per byte is the
difference between runs over 2 MB and 6 MB, which removes key setup.

| cipher            | size  | software, instructions/byte | hardware, instructions/byte |
| ----------------- | ----- | --------------------------- | --------------------------- |
| AES-128-GCM       | 1 KB  | 57.1                        | 3.42                        |
| AES-128-GCM       | 16 KB | 55.9                        | 3.06                        |
| AES-128-GCM       | 1 MB  | 55.8                        | 3.05                        |
| AES-256-GCM       | 1 KB  | 74.1                        | 3.81                        |
| AES-256-GCM       | 16 KB | 72.7                        | 3.44                        |
| AES-256-GCM       | 1 MB  | 72.6                        | 3.42                        |
| ChaCha20-Poly1305 | all   | 28.1 to 30.0                | unchanged                   |

Wall clock on the loaded machine, single runs of `tools/bench/aes_gcm_hw.salam`:
AES-128-GCM 95 to 160 MB/s in software and 1200 to 2400 MB/s in hardware;
ChaCha20-Poly1305 160 to 360 MB/s either way. Results were checked against
OpenSSL (python `cryptography`) on 368 random cases up to 1 MB with associated
data, for both key sizes and both paths, and against the NIST GCM vectors in
`aes_fast_gcm`.

Not run: AArch64. The code is compiled for `aarch64-linux-gnu` and the object
contains `aese`, `aesmc` and `pmull`, but there was no AArch64 machine or
emulator, so its results are unchecked.

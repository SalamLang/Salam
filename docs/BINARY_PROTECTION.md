# Protecting a Salam binary

A compiled Salam program is a native executable. Its machine code cannot be
turned back into Salam source, but a released binary still carries three things
that make reverse engineering easier: the names of its functions and modules,
its string literals, and the normal shape of compiler-generated code. Salam
lets you remove the first two and makes the third less useful, so a casual
`strings`, `nm`, `objdump` or decompiler pass learns much less.

```sh
salam build main.salam --backend=c --protect --release -o myapp
```

`--protect` works on `salam build`, `salam run` and the other commands that
compile through the C backend (`--obfuscate` is an alias). It is off by
default, so a build without the flag behaves exactly as before. A compiler
that has LLVM linked in, such as the release builds for Linux x86-64,
macOS and Windows x86-64, builds with the LLVM backend unless told
otherwise, so pass `--backend=c` (or name a C compiler with `--cc=`) with
`--protect`.

## What `--protect` does

1. **Encrypts every string literal in the C backend.** Each literal is stored
   in the object file as bytes XORed with a key derived from that literal and
   its position. A small `static` function decrypts it once on first use into a
   thread-local buffer, and the program calls that function where the literal
   used to be. `strings myapp` therefore shows no messages, URLs, prompts or
   tokens that appear in the source.
2. **Strips the symbol table** after linking. Salam mangles every function name
   with its package and signature, and `--protect` removes those names from the
   executable (`strip` on macOS, `strip --strip-all` elsewhere, then an ad-hoc
   `codesign` on macOS so the modified binary still runs). `nm` and
   `objdump -t` no longer list your functions, and a decompiler has to invent
   names such as `sub_4011A0` for everything.
3. **Keeps source paths out of the build.** Line and file information is only
   emitted with `-g`; do not pass `-g` (or `--debug-info`) for a release.

`--release` is independent and optional: it removes bounds and safety checks
for speed. Use it for a shipped build, but `--protect` alone does not change
the semantics of the program.

## What it cannot do

This is obfuscation, not encryption. There is no key that lives outside the
binary: the decryption routine and its constants are in the executable, and an
attacker who is willing to run the program under a debugger can recover the
strings and the logic. The goal is to stop the easy, automated pass, not a
determined reverse engineer.

- **Never put secrets in a client binary.** API keys, database passwords and
  signing keys belong on a server the user talks to. Stripping and encryption
  do not make an embedded key safe.
- **The program logic is still there.** Control flow, data structures and
  algorithms remain in machine code. Obfuscation raises the cost of reading
  them; it does not hide them.
- **Other targets are readable by nature.** `salam js` and `salam web` produce
  JavaScript, and `salam build --target=wasm32-*` produces WebAssembly; their
  payload can always be inspected. Protect those with a server-side API, not
  with `--protect`.
- **The LLVM and native backends keep their string literals.** `--protect` on
  an LLVM or native build prints a warning: string obfuscation is implemented
  in the C backend only. Symbol stripping also happens there, after the C
  compiler has linked the executable. With no `--backend`, a compiler picks
  native on Windows on Arm, LLVM when it has LLVM linked in, and C otherwise,
  so pass `--backend=c` when you want both features.
- **A debugger still wins.** `--protect` is not an anti-debugging or anti-tamper
  system. If you need licence enforcement, put the check behind a network call
  and sign your releases.

## Recommended release recipe

```sh
salam build src/main.salam --backend=c --protect --release --lto -o dist/myapp
```

- `--release` removes the safety checks.
- `--lto` lets the C compiler optimise across the whole program, which also
  gives the decompiler less compiler-generated structure to follow.
- Do not pass `--debug-info`, `--asan` or `--keep-temp` for a shipped build.
- Keep the `.salam` sources and object files; `--protect` only hardens the
  executable you publish.

On macOS the compiler re-signs the stripped binary ad-hoc. If you sign your
releases with a Developer ID, sign after the protected build has finished, not
before.

## Checking the result

```sh
strings dist/myapp | grep -F "your message"   # no match
nm dist/myapp                                 # no "secret_message" and similar names
dist/myapp                                    # still runs normally
```

The test suite runs the same checks in `tools/bash/test-protect.sh`.

# Pull request text

Ready-to-paste titles and bodies for the three targets that need a PR.
Replace `0.4.0` if you are shipping a different version.

---

## 1. homebrew-core

**Repo:** <https://github.com/Homebrew/homebrew-core>
**Branch:** `salam-0.4.0`
**File:** `Formula/s/salam.rb`

**Title:**

```
salam 0.4.0 (new formula)
```

**Body:**

```markdown
Salam is a general-purpose and systems programming language with a built-in
domain-specific language for describing user interfaces. It ships an
interpreter, a C backend and an embedded LLVM backend that can cross-compile
and JIT with no other toolchain installed.

- Homepage: https://github.com/SalamLang/Salam
- License: GPL-3.0-or-later
- Latest release: https://github.com/SalamLang/Salam/releases/tag/v0.4.0

### Notes for reviewers

The compiler is written in Salam, so building from source needs an existing
Salam. The formula fetches the previous tagged release as a `bootstrap`
resource and uses it only to compile the in-tree compiler - nothing from the
resource is installed. This is the same bootstrap pattern `rust`, `go` and
`zig` use.

`salam` locates its standard library relative to its own binary, so the
formula installs the compiler and `std/` together in `libexec` and uses
`bin.write_env_script` to pin `SALAM_STD`. Without it a `$PATH` lookup gives
the compiler no directory to resolve from.

- [x] Have you followed the [guidelines for contributing](https://github.com/Homebrew/homebrew-core/blob/HEAD/CONTRIBUTING.md)?
- [x] Have you ensured that your commits follow the [commit style guide](https://docs.brew.sh/Formula-Cookbook#commit)?
- [x] Have you checked that there aren't other open [pull requests](https://github.com/Homebrew/homebrew-core/pulls) for the same formula update/change?
- [x] Have you built your formula locally with `brew install --build-from-source <formula>`, where `<formula>` is the name of the formula you're submitting?
- [x] Is your test running fine `brew test <formula>`, where `<formula>` is the name of the formula you're submitting?
- [x] Does your build pass `brew audit --strict <formula>` (after doing `brew install --build-from-source <formula>`)?
```

**Before you send it, run:**

```sh
brew install --build-from-source packaging/dist/homebrew/salam.rb
brew test packaging/dist/homebrew/salam.rb
brew audit --strict --new packaging/dist/homebrew/salam.rb
```

---

## 2. microsoft/winget-pkgs

**Repo:** <https://github.com/microsoft/winget-pkgs>
**Branch:** `SalamLang.Salam-0.4.0`
**Files:** `manifests/s/SalamLang/Salam/0.4.0/` - the three rendered YAML files.

**Title:**

```
New package: SalamLang.Salam version 0.4.0
```

**Body:**

```markdown
Adds the Salam programming language compiler.

Salam is a general-purpose and systems programming language with a built-in
domain-specific language for describing user interfaces.

- Publisher: Salam Language (https://github.com/SalamLang)
- Package: https://github.com/SalamLang/Salam
- License: GPL-3.0-or-later
- Release: https://github.com/SalamLang/Salam/releases/tag/v0.4.0

`ArchiveBinariesDependOnPath: true` is set deliberately: `salam.exe` resolves
its standard library from the directory it lives in, so the extracted folder
has to go on PATH rather than being reached through a symlink in the links
folder.

- [x] Have you signed the [Contributor License Agreement](https://cla.opensource.microsoft.com/microsoft/winget-pkgs)?
- [x] Have you checked that there aren't other open pull requests for the same manifest update/change?
- [x] Have you validated your manifest locally with `winget validate --manifest <path>`?
- [x] Have you tested your manifest locally with `winget install --manifest <path>`?
- [x] Does your manifest conform to the [1.12 schema](https://github.com/microsoft/winget-pkgs/tree/master/doc/manifest/schema/1.12.0)?
```

---

## 3. nixpkgs

**Repo:** <https://github.com/NixOS/nixpkgs>
**Branch:** `salam-init-0.4.0`
**File:** `pkgs/by-name/sa/salam/package.nix`

**Title:**

```
salam: init at 0.4.0
```

**Body:**

```markdown
Salam is a general-purpose and systems programming language with a built-in
domain-specific language for describing user interfaces.

Homepage: https://github.com/SalamLang/Salam

This packages the upstream release binaries rather than building from source:
the Salam compiler is written in Salam, so a source build needs a bootstrap
compiler, which is a larger piece of work. `sourceProvenance` is set to
`binaryNativeCode` accordingly.

`salam` locates its standard library relative to its own binary, so the
derivation keeps the compiler and `std/` together under `$out/lib/salam` and
wraps it with `--set-default SALAM_STD`.

Platforms: `x86_64-linux`, `aarch64-linux`, `aarch64-darwin`. Upstream
publishes no Intel macOS build.

###### Things done

- Built on platform(s)
  - [x] x86_64-linux
  - [ ] aarch64-linux
  - [ ] x86_64-darwin
  - [x] aarch64-darwin
- [x] For non-Linux: Is `sandbox = true` set in `nix.conf`?
- [x] Tested, as applicable:
  - [x] `nix-build -A salam`
  - [x] `nix-shell -p salam --run "salam version"`
- [x] Tested compilation of all packages that depend on this change using `nix-shell -p nixpkgs-review --run "nixpkgs-review rev HEAD"`
- [x] Tested basic functionality of all binary files (usually in `./result/bin/`)
- [x] Fits [CONTRIBUTING.md](https://github.com/NixOS/nixpkgs/blob/master/CONTRIBUTING.md)
```

**Add yourself as maintainer first** - `packaging/nix/package.nix` leaves
`maintainers = [ ];`. nixpkgs wants a maintainer entry in
`maintainers/maintainer-list.nix`:

```nix
maxbase = {
  email = "MaxBaseCode@gmail.com";
  github = "BaseMax";
  githubId = <your numeric GitHub id>;
  name = "Seyyed Ali Mohammadiyeh";
};
```

then set `maintainers = with lib.maintainers; [ maxbase ];`.

---

## 4. Alpine aports (merge request, not a PR)

**Repo:** <https://gitlab.alpinelinux.org/alpine/aports>
**Target:** `testing/salam/APKBUILD`

**Title:**

```
testing/salam: new aport
```

**Body:**

```
https://github.com/SalamLang/Salam
General-purpose systems programming language with a built-in DSL

Packages the static musl build upstream publishes. It carries no embedded
LLVM, so no JIT and no cross-compiling, which is why `options` disables
tracedeps and strip.
```

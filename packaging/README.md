# Packaging Salam

Everything needed to ship a Salam release through the major package managers.

Templates live in this directory with `@PLACEHOLDER@` tokens.
`packaging/bump.sh <version>` fills them in from the published GitHub release
and writes the result to `packaging/dist/` (generated, not committed).

```sh
packaging/bump.sh 0.4.0
```

It reads the release's own `SHA256SUMS`, so no checksum is ever typed by hand,
and it fails if any placeholder is left unsubstituted.

## Why every package ships a wrapper

The compiler finds its standard library by looking at the directory its own
binary sits in. A shell that resolves `salam` through `$PATH` passes the bare
name as `argv[0]`, so there is no directory to look at.

Every package here therefore installs the real binary and `std/` together
under a private prefix, and puts a small wrapper on `$PATH` that exports
`SALAM_STD` before exec'ing it:

|                          | payload                       | on `$PATH`                              |
| ------------------------ | ----------------------------- | --------------------------------------- |
| deb / rpm / AUR / Alpine | `/usr/lib/salam/{salam,std}`  | `/usr/bin/salam` wrapper                |
| Homebrew                 | `#{libexec}/{salam,std}`      | `bin.write_env_script`                  |
| Nix                      | `$out/lib/salam/{salam,std}`  | `makeWrapper`                           |
| Snap                     | `$SNAP/lib/salam/{salam,std}` | `apps.salam.environment`                |
| Scoop                    | extracted dir                 | `env_set.SALAM_STD`                     |
| WinGet                   | extracted dir                 | `ArchiveBinariesDependOnPath: true`     |
| Chocolatey               | `tools/salam-windows/`        | `Install-ChocolateyEnvironmentVariable` |

`SALAM_STD` is only a default (`${SALAM_STD:-...}`), so a user who sets it
still wins.

## What you need before publishing

Where to get each credential, and the secret name CI expects. `RELEASE_TOKEN`
already exists on this repository, so the `.deb`/`.rpm` build (inside
`compiler-release.yml`) needs nothing new.

| Service                     | Get the token here                                                                                                                                                     | Secret name                                        | Wired in CI?      |
| --------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------- | ----------------- |
| GitHub Releases (.deb/.rpm) | built-in - `GITHUB_TOKEN`, falls back to the existing `RELEASE_TOKEN`                                                                                                  | `RELEASE_TOKEN` _(already set)_                    | yes               |
| AUR                         | register at <https://aur.archlinux.org/register>, then paste an SSH **public** key at <https://aur.archlinux.org/account/> → _Edit_ → _SSH Public Key_                 | `AUR_SSH_PRIVATE_KEY`, `AUR_USERNAME`, `AUR_EMAIL` | yes, secret-gated |
| npm                         | <https://www.npmjs.com/settings/~/tokens> → _Generate New Token_ → _Granular Access Token_, read+write on `salamlang`                                                  | `NPM_TOKEN`                                        | yes, secret-gated |
| Chocolatey                  | register at <https://push.chocolatey.org/>, key at <https://push.chocolatey.org/account>                                                                               | `CHOCO_API_KEY`                                    | yes, secret-gated |
| Snap Store                  | `snapcraft register salam`, then `snapcraft export-login --snaps=salam --acls package_access,package_push,package_update,package_release creds.txt` and paste the file | `SNAPCRAFT_STORE_CREDENTIALS`                      | not yet           |
| Fedora COPR                 | log in at <https://copr.fedorainfracloud.org/>, token at <https://copr.fedorainfracloud.org/api/> (expires after 180 days)                                             | `COPR_CONFIG`                                      | not yet           |
| Scoop bucket                | no token - create `SalamLang/scoop-salam` and push                                                                                                                     | -                                                  | not yet           |
| asdf plugin                 | no token - create `SalamLang/asdf-salam` and push                                                                                                                      | -                                                  | not yet           |
| homebrew-core               | no token - a PR from your GitHub account                                                                                                                               | -                                                  | n/a               |
| WinGet                      | no token - a PR from your GitHub account, after signing the [Microsoft CLA](https://cla.opensource.microsoft.com/microsoft/winget-pkgs)                                | -                                                  | n/a               |
| nixpkgs                     | no token - a PR from your GitHub account                                                                                                                               | -                                                  | n/a               |
| Alpine aports               | no token - a merge request on <https://gitlab.alpinelinux.org/alpine/aports>                                                                                           | -                                                  | n/a               |

Add secrets at
<https://github.com/SalamLang/Salam/settings/secrets/actions>.

Nothing in CI pushes anywhere until the matching secret exists. The workflow
gates each publish step on its secret and logs a notice when it skips, so a
release with none of these set still builds and attaches the `.deb`/`.rpm`
and does nothing else.

## Per-target instructions

### Debian / Ubuntu - `.deb` on the release

Fully automatic, and built inside `compiler-release.yml`'s `create-release`
job itself - not in `packaging-release.yml`. GitHub's org-enforced
immutable-releases setting locks a release's assets the instant it publishes,
so anything trying to attach a `.deb`/`.rpm` afterward (which is what
`packaging-release.yml` did originally) fails with `HTTP 422: Cannot upload
assets to an immutable release`. The fix is building
`salam_<version>-1_{amd64,arm64,armhf,i386}.deb` from the Linux tarballs
already sitting in `./release/` before the release is created, so they ship
in the same initial asset list.

Users install with:

```sh
wget https://github.com/SalamLang/Salam/releases/download/v0.4.0/salam_0.4.0-1_amd64.deb
sudo apt install ./salam_0.4.0-1_amd64.deb
```

`apt install ./file.deb` (not `dpkg -i`) pulls in `libxml2` and friends.

To build locally:

```sh
packaging/debian/build-deb.sh --version 0.4.0 --input ./release --output ./debs
```

### Arch - AUR

Two packages: `salam-bin` (the prebuilt release, what most people want) and
`salamlang` (builds from source using the previous release as the bootstrap
seed - installs the same `salam` command; the plain name `salam` is already
an unrelated, actively-maintained AUR package, so ours can't use it).

First time only, create them:

1. Register at <https://aur.archlinux.org> and add an SSH **public** key under
   _My Account_.
2. Add the matching **private** key to this repository as `AUR_SSH_PRIVATE_KEY`,
   plus `AUR_USERNAME` and `AUR_EMAIL`.
3. Seed each repository once by hand:

```sh
packaging/bump.sh 0.4.0
git clone ssh://aur@aur.archlinux.org/salam-bin.git
cd salam-bin
cp ../packaging/dist/aur/salam-bin/PKGBUILD .
makepkg --printsrcinfo > .SRCINFO
git add PKGBUILD .SRCINFO
git commit -m "Initial import of salam-bin 0.4.0"
git push origin master
```

After that the `publish-aur` job updates both on every release.
Users install with `yay -S salam-bin` or `yay -S salamlang`.

**The plain `salam` name is already taken on the AUR** by an unrelated,
actively-maintained package (a different, MIT-licensed project, last updated
recently - not abandoned, so orphan-requesting it isn't an option). The
`publish-aur` job pushes `salam-bin` fine but fails on `salam` with
`git-receive-pack: permission denied`, since our key isn't a co-maintainer on
someone else's package. Either rename our source-build package (e.g.
`salam-lang`, to match the npm name) or drop it and ship only `salam-bin` -
most users want the prebuilt binary anyway.

### Homebrew - homebrew-core

`packaging/homebrew/salam.rb` builds from source, which homebrew-core requires,
using the previous release as a bootstrap seed (the same pattern `rust` and
`go` use). See `packaging/PULL-REQUESTS.md` for the PR title and body.

Test the formula before sending it:

```sh
packaging/bump.sh 0.4.0
brew install --build-from-source --verbose --debug packaging/dist/homebrew/salam.rb
brew test packaging/dist/homebrew/salam.rb
brew audit --strict --new packaging/dist/homebrew/salam.rb
```

`brew audit --new` is what the reviewers run; fix everything it reports first.

**Heads-up:** homebrew-core builds every formula on Intel macOS too, and Salam
publishes no `macos-x86_64` binary, so the bootstrap resource has no Intel
seed. Either add an Intel build to the release matrix, or the formula needs
`depends_on arch: :arm64`, which reviewers will question. A `salamlang/salam`
tap has neither problem and would ship the same night - worth keeping as the
fallback if core pushes back.

### Windows - Scoop, WinGet, Chocolatey

**Scoop** is the fastest: create `SalamLang/scoop-salam`, drop
`packaging/dist/scoop/salam.json` into `bucket/salam.json`, push. Users then:

```powershell
scoop bucket add salam https://github.com/SalamLang/scoop-salam
scoop install salam
```

**WinGet** is a PR to `microsoft/winget-pkgs`; the manifests go to
`manifests/s/SalamLang/Salam/0.4.0/`. Validate first:

```powershell
winget validate --manifest packaging\dist\winget
winget install --manifest packaging\dist\winget
```

**Chocolatey** needs an API key, then:

```powershell
cd packaging\dist\chocolatey
choco pack
choco apikey --key <API_KEY> --source https://push.chocolatey.org/
choco push salam.0.4.0.nupkg --source https://push.chocolatey.org/
```

Expect moderation. The 108 MB download will draw a reviewer comment; the
answer is that the embedded LLVM toolchain is the point of the distribution.

### Nix

`packaging/nix/flake.nix` works straight from the repository:

```sh
nix run github:SalamLang/Salam?dir=packaging/nix
nix profile install github:SalamLang/Salam?dir=packaging/nix
```

For nixpkgs proper, `packaging/dist/nix/package.nix` goes to
`pkgs/by-name/sa/salam/package.nix`. PR text is in `PULL-REQUESTS.md`.

### Fedora / RHEL - COPR

Create a project at <https://copr.fedorainfracloud.org>, then either upload
`packaging/dist/rpm/salam.spec` by hand or point COPR at this repository.
`compiler-release.yml` already builds the `.rpm` and attaches it to the
release, so users can also `dnf install` it directly without COPR.

### Alpine

`packaging/alpine/APKBUILD` packages the **musl** build: 2.5 MB, fully static,
but with no embedded LLVM, so no JIT and no cross-compiling. That is the right
trade for Alpine. Submitting to `aports` is a merge request on
<https://gitlab.alpinelinux.org/alpine/aports>.

### Snap

Register the name first (`snapcraft register salam`) - it can take a few days
for a name that matches a project you do not obviously own. Then:

```sh
snapcraft
snapcraft upload --release=stable salam_0.4.0_amd64.snap
```

Note `confinement: classic`: a compiler has to read and write files anywhere
the user can, and classic confinement needs a manual review on the store.

### npm

```sh
cd packaging/dist/npm
npm publish --access public
```

Installs as `salamlang`; the postinstall downloads the right release binary
and verifies it against the published `.sha256`. Users get `npx salamlang`.

### asdf / mise

`packaging/asdf/` is a plugin. It needs its own repository, `SalamLang/asdf-salam`,
containing just the `bin/` directory. Then:

```sh
asdf plugin add salam https://github.com/SalamLang/asdf-salam
asdf install salam 0.4.0
# mise reads asdf plugins directly:
mise use -g salam@0.4.0
```

## Release checklist

1. Tag and publish `v<version>` as usual (`release:` commit on `main`).
   `compiler-release.yml`'s `create-release` job builds and attaches the
   `.deb`/`.rpm` itself, as part of the same release.
2. Wait for that to finish - `packaging-release.yml`'s `bump.sh` step needs
   the release's `SHA256SUMS` to already be published.
3. `packaging-release.yml` then runs on its own: renders every manifest,
   pushes to the AUR/npm/Chocolatey if their secrets are set.
4. Download the `salam-package-manifests` artifact for the PR-based targets.
5. Send the Homebrew / WinGet / nixpkgs PRs from `PULL-REQUESTS.md`.

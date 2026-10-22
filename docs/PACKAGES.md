# Packages: salam.toml, salam.lock and the package commands

A Salam project is a directory with a `main.salam` entry file and a
`salam.toml` manifest. `salam new <name>` and `salam init` create both.

```text
myapp/
  main.salam      the entry file; nothing else is ever picked as the entry
  salam.toml      what the project is and what it needs
  salam.lock      written by the package commands; commit it
  util/util.salam your own packages, one folder each
```

## salam.toml

```toml
[package]
name = "myapp"            # the program or library name (defaults to the folder)
version = "0.1.0"
salam = ">=0.5.7"         # oldest compiler this project builds with
kind = "bin"              # "bin" (default) or "lib"

[dependencies]
"github.com/owner/repo" = "v1.2.0"           # a GitHub package, pinned to a tag
"db.redis" = "v0.5.7"                         # an official package (follows the compiler)
"example.com/me/lib" = { path = "../lib" }    # a local folder standing in for a package

[native.sqlite3]          # only in packages that wrap a C library
link = "sqlite3"
pkg-config = "sqlite3"
source-url = "https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip"
source-sha256 = "77823cb1..."
sources = ["sqlite-amalgamation-3460100/sqlite3.c"]
cflags = ["-O2"]
libs = ["m"]
apt = "libsqlite3-dev"
dnf = "sqlite-devel"
pacman = "sqlite"
brew = "sqlite"
winget = "SQLite.SQLite"
```

- **`[package].salam`** is checked by `salam add`, `update`, `deps` and
  `install`, for the project and for every dependency. A package that needs
  a newer compiler stops the command with "needs Salam X or newer; run:
  salam upgrade".
- **`[package].kind = "lib"`** makes a bare `salam build` produce a shared
  library, as if `--lib` was given. `--lib=static` still overrides it.
- **`[package].name`** names the executable or library that `salam build`
  writes for the project.
- **Dependency versions** are exact tags (`v1.2.0`) or pseudo versions
  (`v0.0.0-<12 hex>`) for repositories without a release tag. `salam.lock`
  records what was actually chosen.

## salam.lock

```toml
# Written by salam add, remove and update. Do not edit by hand.

[[package]]
path = "github.com/owner/repo"
version = "v1.2.0"
commit = "4f1c..."
sum = "sha256:9a0e..."
```

Every package the build can reach is listed, including dependencies of
dependencies. When two packages ask for different versions of the same
module, the newer one is used (minimal version selection, as in Go). `sum`
covers every file of the package except `.git` and `.salam-commit`, so a
download by Git and a download of GitHub's tar.gz give the same sum.
`salam deps` re-checks it.

## Importing

```salam
import net.http                          // standard library
import db.redis                          // official package, after `salam add db.redis`
import "github.com/owner/repo"           // binds repo
import tools "github.com/owner/repo/sub" // the package in the repo's sub/ folder
```

A path import resolves to `<repo>/<last segment>.salam` (or
`<repo>/<sub>/<sub>.salam`). It only works inside a project whose
`salam.toml` lists the module.

## Commands

| Command                                 | What it does                                                                                |
| --------------------------------------- | ------------------------------------------------------------------------------------------- |
| `salam init [dir]` / `salam new <name>` | create `main.salam`, `salam.toml`, `.gitignore`                                             |
| `salam add <pkg>[@vX.Y.Z]`              | add the newest release (or that tag), download it, update both files                        |
| `salam remove <pkg>`                    | take it out of both files; the download stays cached                                        |
| `salam update [pkg...]`                 | move to the newest release **with the same major version**; it says when a new major is out |
| `salam update <pkg>@latest`             | move across a major version                                                                 |
| `salam deps`                            | download everything `salam.lock` lists and verify each sum; run it after a clone            |
| `salam deps clean`                      | empty the download cache                                                                    |
| `salam install <pkg>[/cmd][@v]`         | build that program's `main.salam` into `~/.salam/bin`                                       |
| `salam install`                         | the same for the project you are in                                                         |
| `salam uninstall [name]`                | remove an installed program; with no name, list them                                        |
| `salam search <words>`                  | search the standard library, the official packages and `registry/index.toml`                |
| `salam build --lib[=static]`            | build a library and a C header from the entry file's `export:` block                        |

## Where things live

| Path                                      | What                                              |
| ----------------------------------------- | ------------------------------------------------- |
| `~/.salam/pkg/<module>@<version>/`        | downloaded packages (`$SALAM_PKG` moves the root) |
| `~/.salam/pkg/salam-packages@v<version>/` | the official packages for one compiler version    |
| `~/.salam/pkg/native/`                    | C libraries built from source                     |
| `~/.salam/pkg/installed.txt`              | what `salam install` put where                    |
| `~/.salam/bin/`                           | installed programs (`$SALAM_BIN` moves it)        |

## Environment

| Variable                      | Effect                                                                  |
| ----------------------------- | ----------------------------------------------------------------------- |
| `SALAM_PKG`                   | the package cache root                                                  |
| `SALAM_BIN`                   | where `salam install` puts programs                                     |
| `SALAM_NO_GIT=1`              | download GitHub packages as tar.gz even when Git is installed           |
| `SALAM_PACKAGES`              | a folder of official packages to use instead of downloading them        |
| `SALAM_RELEASES_URL`          | the base URL official package archives are downloaded from              |
| `SALAM_REGISTRY`              | a registry file or URL for `salam search`                               |
| `SALAM_NATIVE=source\|system` | build C libraries from source, or always use the system copy            |
| `GITHUB_TOKEN` / `GH_TOKEN`   | raises GitHub's API rate limit for tag listing                          |
| `GITLAB_TOKEN`                | sent to gitlab.com without Git, for private projects and the rate limit |

## Fetching

With Git on PATH, versions come from `git ls-remote --tags` and a package is a
shallow fetch of the tag. Without Git, packages on github.com, gitlab.com
and codeberg.org come from each host's API as a tar.gz that Salam unpacks
itself; other hosts need Git. Git's own configuration applies, so private repositories work
with the credentials Git already has, and `url.<base>.insteadOf` can point a
module at a mirror. `tools/bash/test-packages.sh` uses that to test the
whole flow offline.

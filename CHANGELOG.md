# Changelog

All notable changes to the Salam compiler and standard library. Salam is before 1.0, so a minor release can change behaviour; anything that can break existing code is listed under "Behaviour changes" with the pull request that made it. Entries are written when a change merges, so the Unreleased section is always current.

## Unreleased (after 0.6.19)

### Security

- sshd and sftpd no longer fall back to a built-in host key derived from a public constant. A missing or unreadable `host_key_file` stops `Listen`; with no file the server makes a random key for that run. OpenSSH ed25519 keys now load (#3739).
- SFTP and FTP clients can no longer write or create links outside the root; the jail check fails closed, symlink targets are resolved and checked, Windows backslashes are treated as separators (#3742).
- `net/http` no longer crashes on a very large `Content-Length`, and `GET`, `HEAD` and `OPTIONS` bodies are read instead of being parsed as the next request. Header lines with whitespace before the colon and folded lines get 400 (#3737, #3743).
- sshd and sftpd serve connections concurrently, with a login grace time of 120 s by default, so one idle connection no longer blocks everyone (#3746).
- Installers check the download against the published sha256 and refuse a mismatch. `SALAM_SKIP_CHECKSUM=1` skips the check (#3733).
- `SECURITY.md` describes how to report a vulnerability (#3734).

### Behaviour changes

- `fs.Permissions` returns the real mode bits (for example 420 for `rw-r--r--`), the same number `fs.Chmod` takes. `Stat`, `ModTime` and friends no longer start a shell (#3745).
- `ssh.ServerConfig.host_seed` and `sshd.Config.host_seed` are removed; use `host_key_file` or `host_key_hex` (#3739).
- Float to integer casts saturate at the target range and turn NaN into 0, on every backend and for every width (#3752).
- `MIN / -1` is `MIN` and `MIN % -1` is 0 for every signed width; signed `+ - *` wrap on every backend, the LLVM backend no longer assumes they cannot overflow (#3749).
- Slices are bounds checked when they are made and when a `T[]` parameter is indexed, except in `--release` builds (#3747).
- Casting an integer to an enum checks the value: a constant that is not a member is error E023, a runtime value panics with `value 105 is not a valid Day` (not in `--release`) (#3751).
- An unknown or malformed escape in a string or char literal is error E260 (`"\d"` must be written `"\\d"` or as a raw string). `\x` needs 2 hex digits, `\u` 4, `\U` 8; surrogates and values above 10FFFF are rejected (#3759).
- A source file with a NUL byte or invalid UTF-8 is error E261, a UTF-8 BOM is accepted, and an unreadable file says so instead of reporting a missing `main` (#3760).
- Two packages that declare the same name, or a local package named like a std package, are an error instead of silently merging (#3748).
- Types are identified by their declaration, so a local `struct a_Item` is no longer the same type as `a.Item` (#3758).
- Expressions taller than 512 nodes and if/else chains of more than 256 arms are errors instead of crashing the compiler (#3750).
- `salam test` reports a test that crashes as FAIL, runs the rest of the file, and exits non-zero (#3740, #3763).
- A program killed by a fatal signal keeps the output it had buffered, names the signal, and still dies by that signal (#3756).

### Fixed

- `fs.ReadAllBytes` and `fs.WriteAllBytes` read and write files with NUL bytes; the MD5, SHA-1, SHA-256 and SHA-512 file helpers hash the real bytes and no longer leak (#3744).
- The native backend computed signed `/`, `%` and `>>` wrongly on narrow integer constants (#3755).
- A float cast in a struct literal member is evaluated once (#3752).

### Release and tooling

- A release now waits for the native backend gate on its own commit and stops if it failed (#3727).
- Readme badges for workflows that are turned off are removed (#3764); the platform notes no longer say the 32-bit Linux releases embed LLVM (#3736).

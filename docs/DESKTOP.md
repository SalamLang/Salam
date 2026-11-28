# Desktop applications

`std/gui/webview` builds desktop applications in Salam alone, the kind you would
build with Tauri or Electron. There is no Rust and no bundled browser: the
interface is HTML, CSS and JavaScript in the engine the operating system
already has, and the program is one native executable.

| System  | Engine    | Needed at runtime                                 |
| ------- | --------- | ------------------------------------------------- |
| Linux   | WebKitGTK | `libgtk-3-0`, `libwebkit2gtk-4.1-0`               |
| Windows | WebView2  | The WebView2 runtime, included in current Windows |
| macOS   | WKWebView | Nothing                                           |

## A first application

`salam new notes --desktop` writes this project for you (`--lang=fa` for Persian).

```salam
import gui.webview

func main:
    webview.Init(webview.Config { id = "com.example.notes", name = "Notes", assets_dir = "www" })
    webview.Command("greet", (c: webview.Call):
        c.ResolveStr("Hello " + c.Str("name"))
    end)
    win := webview.Open(webview.WindowOptions { title = "Notes", width = 900, height = 600 })
    if win.IsClosed():
        println "could not open a window"
        ret 1
    end
    webview.Run()
end
```

`www/index.html` calls it:

```js
const text = await salam.invoke('greet', { name: 'Sara' });
```

`tests/en/desktop/sample` is a complete project with a `salam.toml`.

## Talking to the page

| From the page                    | In Salam                                                                    |
| -------------------------------- | --------------------------------------------------------------------------- |
| `await salam.invoke(name, args)` | `webview.Command` runs on the UI thread, `webview.CommandAsync` on a worker |
| `salam.emit(name, data)`         | `webview.On(name, handler)`                                                 |
| `salam.listen(name, fn)`         | `webview.Emit(name, json)`, `win.Emit`                                      |

A command finishes with one `Resolve` method or `Reject`, from any thread. A
second answer is ignored, and the text in a `Call` is freed by the answer, so
read what you need first. `Str` returns a copy the caller frees.
Arguments and results are JSON. `webview.Dispatch` runs code on the UI thread
from a worker, which is where every window call belongs.

## Where pages come from

- **Files inside the program.** The default: `index.html` from the asset pack,
  served at `app://localhost` (`https://app.localhost` on Windows, so use
  relative paths or `webview.AssetURL`).
- **A folder.** `assets_dir` in `Config`, for development. A reload shows your
  edits.
- **A site.** `url` in `WindowOptions`.
- **A string.** `html` in `WindowOptions`.

The pack is compressed and scrambled so that names and contents do not show up
in a text editor or `strings`. That hides files, it does not protect them:
whatever a page can read, a determined user can extract. Secrets belong on a
server.

## What a page may do

- Local pages may run every command. Remote origins may run nothing until you
  name them with `webview.Allow(origin, commands)`. A pattern may end in `*`.
  Call it before opening windows, because the list is built into the script
  each window installs.
- The bridge is installed only in the top frame of an allowed origin. Every
  message carries a secret created for this run, and Salam checks it together
  with the origin of the page before running anything, so a frame or another
  site cannot call a command. Answers carry their origin, so they cannot reach
  a page that replaced the caller.
- A link to any other origin opens in the system browser. `allow_navigation`
  lists origins a window may visit. They still cannot run commands.
- Paths that try to leave the assets are refused.

WebKitGTK cannot tell a frame from the page when it asks about navigation. On
Linux a cross-origin frame therefore has to be listed in `allow_navigation`,
and only a navigation the user started opens the browser.

## System services

| Function                                 | Linux                              | Windows               | macOS                                  |
| ---------------------------------------- | ---------------------------------- | --------------------- | -------------------------------------- |
| `PickFolder`, `PickFile`, `SaveFile`     | GTK file chooser                   | `IFileDialog`         | `NSOpenPanel`, `NSSavePanel`           |
| `Message`, `Confirm`                     | GTK dialog                         | `MessageBoxW`         | `NSAlert`                              |
| `Notify`                                 | Desktop notifications over D-Bus   | Tray balloon or toast | `NSUserNotification`, then `osascript` |
| `RequestAttention`                       | Urgency hint                       | `FlashWindow`         | Dock bounce                            |
| `SecretSet`, `SecretGet`, `SecretDelete` | libsecret                          | Credential Manager    | Keychain                               |
| `SetMenu`                                | Menu bar                           | Menu bar              | Application menu                       |
| `SetTray`                                | AppIndicator, then `GtkStatusIcon` | Notification area     | Status item                            |
| `SingleInstance`                         | Lockfile and socket                | Message window        | Lockfile and socket                    |
| `OpenURL`                                | GIO                                | `ShellExecuteW`       | `NSWorkspace`                          |

A secret falls back to a `0600` file in the data folder when the system has no
keychain, for example on a Linux machine with no secret service.

`libsecret` and AppIndicator are loaded when needed, so an application still
starts without them.

## Packaging

Add an `[app]` table to `salam.toml` (`salam init` writes one for a dsl project; every key
of the project file is in [PROJECT_FILE.md](PROJECT_FILE.md)):

```toml
[app]
id = "com.example.notes"
name = "Notes"
version = "1.0.0"
main = "main.salam"
assets = "www"
icon = "icon.png"
description = "Take notes"
author = "Example"
category = "Utility"

[windows]
company = "Example Ltd"
copyright = "2026 Example Ltd"
```

`author` (or the older `publisher`) is the maintainer of the `.deb` and the company in the
Windows version info. `[windows]`, `[macos]` and `[linux]` repeat any `[app]` key for that
system only, and `[windows]` also takes `subsystem`, `icon`, `manifest`, `product` and
`copyright`. A Windows build with no bundle (`salam build --platform=windows` in the project
folder) reads the same `[windows]` table and falls back to `[app]` for the icon, version,
product name, description, company and copyright. The icon may be a `.ico` or a `.png` file;
a PNG is stored as it is, so keep it at 256 by 256 pixels or smaller. Every backend and
linker embeds these, except the tcc C compiler, which warns instead. Such a build of a
`page:` program opens no console window unless `subsystem = "console"` asks for one.

```sh
salam bundle
salam bundle --target=x86_64-w64-windows-gnu
salam bundle --target=arm64-apple-macos --format=app,tar
```

| System  | Default      | Also, with the tool on `PATH`                   |
| ------- | ------------ | ----------------------------------------------- |
| Linux   | `tar`, `deb` | `rpm` (`rpmbuild`), `appimage` (`appimagetool`) |
| Windows | `zip`        | `nsis` (`makensis`)                             |
| macOS   | `app`, `tar` | `dmg` (`hdiutil`)                               |

Cross builds need nothing extra: Windows and macOS programs are linked by the
compiler itself. The assets go at the end of the executable on Linux and
Windows, and into `Contents/Resources/app.pack` on macOS so the signature of the
program stays valid. A PNG icon is converted to `.ico` and `.icns`.

Nothing is signed unless you ask. macOS binaries get the ad-hoc signature the compiler
writes, which is enough to run them after removing the quarantine flag with
`xattr -d com.apple.quarantine`. Sign and notarize for real distribution.

## Signing

```sh
salam build --sign
salam bundle --sign
salam bundle --sign --target=x86_64-w64-windows-gnu --format=zip,nsis
salam bundle --sign --notarize --target=arm64-apple-macos --format=dmg
salam sign dist/notes-setup.exe
```

Signing runs only with `--sign` or `salam sign`. The settings come from `[windows]`,
`[macos]` and `[linux]` in `salam.toml` (every key is in [PROJECT_FILE.md](PROJECT_FILE.md))
and from options, which win: `--sign-cert`, `--sign-key`, `--sign-thumbprint`, `--sign-tool`,
`--sign-command`, `--timestamp-url`, `--no-timestamp`, `--sign-identity`, `--entitlements`,
`--notarize`, `--notary-profile`, `--gpg-key` and `--sign-password-file`. A missing tool,
certificate or key stops the command before it builds. A signing step that fails makes the
command fail; the files already written are left in place and must not be given to anyone.

`salam bundle --sign` signs in this order:

| system  | what is signed                                                                                                                                                  |
| ------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Windows | the program, after the assets are added to it, then the NSIS setup program. The `.zip` holds the signed program. The uninstaller that NSIS writes is not signed |
| macOS   | the `.app`, which is then notarized and stapled when asked, before it goes into `app`, `tar` or `dmg`; the `.dmg` is signed, notarized and stapled too          |
| Linux   | each package gets a detached GPG signature, `NAME.asc`                                                                                                          |

`salam build --sign` signs the program or shared library it wrote. `salam sign FILE...`
signs files made by other tools; it tells the system from the file, or takes `--platform`.

### Passwords

The password of a certificate file, a key or a GPG key is read from
`SALAM_SIGN_PASSWORD`, `SALAM_SIGN_PASSWORD_FILE` or `--sign-password-file=FILE`, never
from `salam.toml`, and it is replaced by `****` in what Salam prints. osslsigncode and gpg
receive it through a file that only you can read, removed after the step. **signtool accepts
a password only on its command line**, where other processes of the same user (and
administrators) can see it, so Salam warns; prefer a certificate in the Windows store
(`sign_thumbprint`) or a token. A `sign_command` of yours inherits the environment.

### Windows

You need a code signing certificate from a certificate authority that Windows trusts.
Authorities usually require proof of identity, and current industry rules require the
private key of a publicly trusted code signing certificate to stay in hardware (a USB token,
a hardware security module or a cloud signing service), so a new certificate normally does
not come as a `.pfx` file. Check the authority's terms, eligibility and prices yourself.

- A token or smart card whose driver puts the certificate in the Windows store works with
  `sign_thumbprint` and signtool.
- A cloud signing service, a PKCS#11 token on Linux or anything else works with
  `sign_command`, for example:

```toml
[windows]
sign_command = ["my-signer", "--timestamp", "{timestamp_url}", "--name", "{description}", "{file}"]
```

`{file}` is required. The program is found on `PATH`, or by a path relative to the project.
It must change the file in place; Salam checks that a `.exe` or `.dll` then carries an
Authenticode signature.

- A `.pfx` file or a certificate and key (an older certificate, or one from your company's
  own authority) works with signtool on Windows or
  [osslsigncode](https://github.com/mtrojnar/osslsigncode) on any system.

signtool comes with the Windows SDK; Salam takes it from `PATH` or from the Windows Kits
folder. Timestamping is on by default (`http://timestamp.digicert.com`; set `timestamp_url`
to your authority's server). A timestamp keeps the signature valid after the certificate
expires, so use `--no-timestamp` only for local tests.

A self-signed certificate does not help your users: Windows does not trust it on their
computers.

### macOS

`codesign`, `notarytool` and `stapler` exist only on a Mac with the Xcode command line
tools. Distribution outside the App Store needs a "Developer ID Application" certificate in
your keychain, which comes with the Apple Developer Program. Store the notary credentials
once, then name the profile:

```sh
xcrun notarytool store-credentials salam-notary
```

```toml
[macos]
sign_identity = "Developer ID Application: Example Ltd (ABCDE12345)"
notary_profile = "salam-notary"
notarize = true
```

Salam signs with the hardened runtime and a timestamp, verifies the signature, waits for
Apple's answer and fails unless it is "Accepted". A bare program from `salam build` can be
notarized but not stapled, so a computer that is offline cannot confirm it; ship an `.app`
or `.dmg`. `sign_identity = "-"` makes an ad-hoc signature for local use. Programs that
need extra permissions list them in an `entitlements` file.

### Linux

`gpg_key` writes `FILE.asc` next to the file. Publish your public key so people can check
it with `gpg --verify FILE.asc FILE`. This is separate from the repository signatures of
`apt` or `dnf`.

### What signing does not do

A signature says who published a file and that it was not changed afterwards. It does not
promise that Windows Defender, SmartScreen or another antivirus will stay quiet: a new
publisher or a new file can still be warned about until it builds a reputation, and
heuristic detections happen to signed files too.

When Microsoft Defender flags your program and you are sure it is clean:

1. Update the Defender definitions and scan again, and note the detection name.
2. Check that the file you scanned is the one you built, and rebuild on a clean machine if
   in doubt.
3. Send the file to Microsoft at <https://www.microsoft.com/wdsi/filesubmission> as a
   software developer, say that it is an incorrect detection, and give the detection name,
   the product name and a link where it is published.
4. Keep the submission id and wait for the answer. Each new build is a new file, and may
   need to be sent again.

Other antivirus vendors have their own submission forms. A version resource, an icon, a
manifest, a stable download address and a signature all help reviewers, but none of them
guarantees a clean result.

## Low level and high level in one package

`gui.webview` holds both layers. `webview.Create` gives a plain window around
HTML, the right choice for a small tool with no commands. `webview.Init`,
`Command`, `Open` and the rest add an application on top of it: commands,
events, packed files, dialogs and installers. Both share one event loop, so
`webview.Run` drives either.

The backends are the `engine_*.salam` files and the system services are the
`os_*.salam` files. Windows does not link `WebView2Loader`. If the DLL is next
to the program it is used, otherwise the runtime is found in the registry. The
backend supports x86-64, ARM64 and 32-bit Windows.

## Choosing the engine

The default engine is the web view of the operating system, as in Tauri, so the
program stays one small executable. A second engine, Chromium through the
Chromium Embedded Framework, is available when you build:

```sh
salam build main.salam -DSALAM_WEBVIEW_CEF
salam bundle --engine=chromium --runtime=cef-dist
```

`engine = "chromium"` and `runtime = "cef-dist"` can also go in the `[app]`
table, and `SALAM_CEF_DIR` names the runtime folder. The folder holds the built
`salam_webview_cef` library, `salam_cef_helper` and the CEF runtime files; the
recipe is `std/gui/webview/native/CMakeLists.txt`. `salam bundle` copies them
next to the program, which works for `tar` on Linux and `zip` on Windows.

The program can ask which engine it got:

```salam
println webview.Engine(), webview.EngineName(), webview.SupportsCommands()
```

The Chromium engine is not complete yet. Windows open and pages load, but there
is no message channel, no custom scheme and none of the system services, so
`SupportsCommands()` is false there and a page cannot call `Command`. The
choice is made when the program is built, because linking both engines would
make every program need both.

## When a window stays blank

Set `SALAM_WEBVIEW_TRACE` to a filename and run the application again. Each line
records one step: how the runtime was started, each navigation, resource and
message, and any message the bridge refused. It works in an application with no
console. `webview.LastError()` says why a window could not open at all.

## Testing

- `tests/en/apps/desktop` and `tests/fa/apps/desktop`: logic that needs no
  window, in the normal test suite.
- `tests/en/desktop/smoke.salam`: everything through a real window. Run it with
  `xvfb-run` on a machine with no display.
- `tests/en/desktop/sample`: bundled by CI and run on each system with
  `--self-test`.
- `.github/workflows/desktop.yml` runs both on real Linux, Windows and macOS,
  along with the `agent/salam` test, every package format, and `salam bundle` on
  Windows and macOS themselves.
- `tools/bash/native-windows-webview-check.sh` tests the Windows backend under
  wine against the WebView2 mock in `std/gui/webview/native/mock`.

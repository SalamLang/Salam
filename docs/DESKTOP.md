# Desktop applications

`std/desktop` builds desktop applications in Salam alone, the kind you would
build with Tauri or Electron. There is no Rust and no bundled browser: the
interface is HTML, CSS and JavaScript in the engine the operating system
already has, and the program is one native executable.

| System  | Engine    | Needed at runtime                                 |
| ------- | --------- | ------------------------------------------------- |
| Linux   | WebKitGTK | `libgtk-3-0`, `libwebkit2gtk-4.1-0`               |
| Windows | WebView2  | The WebView2 runtime, included in current Windows |
| macOS   | WKWebView | Nothing                                           |

## A first application

```salam
import desktop

func main:
    desktop.Init(desktop.Config { id = "com.example.notes", name = "Notes", assets_dir = "www" })
    desktop.Command("greet", (c: desktop.Call):
        c.ResolveStr("Hello " + c.Str("name"))
    end)
    win := desktop.Open(desktop.WindowOptions { title = "Notes", width = 900, height = 600 })
    if win.IsClosed():
        println "could not open a window"
        ret 1
    end
    desktop.Run()
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
| `await salam.invoke(name, args)` | `desktop.Command` runs on the UI thread, `desktop.CommandAsync` on a worker |
| `salam.emit(name, data)`         | `desktop.On(name, handler)`                                                 |
| `salam.listen(name, fn)`         | `desktop.Emit(name, json)`, `win.Emit`                                      |

A command finishes with one `Resolve` method or `Reject`, from any thread. A
second answer is ignored, and the text in a `Call` is freed by the answer, so
read what you need first. `Str` returns a copy the caller frees.
Arguments and results are JSON. `desktop.Dispatch` runs code on the UI thread
from a worker, which is where every window call belongs.

## Where pages come from

- **Files inside the program.** The default: `index.html` from the asset pack,
  served at `app://localhost` (`https://app.localhost` on Windows, so use
  relative paths or `desktop.AssetURL`).
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
  name them with `desktop.Allow(origin, commands)`. A pattern may end in `*`.
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

Add an `[app]` table to `salam.toml`:

```toml
[app]
id = "com.example.notes"
name = "Notes"
version = "1.0.0"
main = "main.salam"
assets = "www"
icon = "icon.png"
description = "Take notes"
publisher = "Example"
category = "Utility"
```

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

Nothing is signed. macOS binaries get the ad-hoc signature the compiler
writes, which is enough to run them after removing the quarantine flag with
`xattr -d com.apple.quarantine`. Sign and notarize for real distribution.

## Lower level

`std/webview` is the layer underneath, and the right choice for a small window
with no commands. It has one public API over the backend files
`linux.salam`, `windows.salam`, `macos.salam` and `cef.salam`.

Windows does not link `WebView2Loader`. If the DLL is next to the program it is
used, otherwise the runtime is found in the registry. The backend supports
x86-64, ARM64 and 32-bit Windows.

The CEF build (`-DSALAM_WEBVIEW_CEF`) has no message channel or custom schemes,
so `std/desktop` does not support it.

## Testing

- `tests/en/apps/desktop` and `tests/fa/apps/desktop`: logic that needs no
  window, in the normal test suite.
- `tests/en/desktop/smoke.salam`: everything through a real window. Run it with
  `xvfb-run` on a machine with no display.
- `tests/en/desktop/sample`: bundled by CI and run on each system with
  `--self-test`.
- `.github/workflows/desktop.yml` runs both on real Linux, Windows and macOS.
- `tools/bash/native-windows-webview-check.sh` tests the Windows backend under
  wine against the WebView2 mock in `std/webview/native/mock`.

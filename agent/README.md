# Salam Agent (ایجنت سلام)

A desktop app for Windows, macOS and Linux that opens <https://ai.salamlang.ir>
and lets the assistant work on your computer. With your approval it can run
shell commands (Bash, sh, PowerShell, cmd), use Docker and WSL, read, search,
create and edit files in a folder you choose, and run your tests.

It is a [Tauri 2](https://tauri.app) app written in Rust. It uses the
system webview (WebView2, WKWebView or WebKitGTK), not Electron.

## How it works

```text
engine -> ai-api /bridge/mcp -> long poll -> this app -> runs the tool after approval
       <-                    <- /bridge/result <-
```

- The window loads the web app. Only `https://ai.salamlang.ir` may call the
  app's commands (`capabilities/remote.json`). Links to other sites open in
  the system browser.
- On sign-in the web app registers the computer (`POST /bridge/devices`). The
  device secret is kept in the OS keychain. If there is no keychain, it goes
  in a `0600` file in the app's config folder.
- `src/bridge.rs` long-polls `/bridge/poll`, runs each call and posts the
  result back. The backend side is described in `ai/README.md`.
- A chat started with a workspace folder is an agent chat. Paths are resolved
  against that folder (`src/workspace.rs`), and symlinks and `..` cannot
  escape it unnoticed.

## Access levels

You pick one per chat in the composer.

| Level                 | Reads in the folder | Writes and commands in the folder | Anything outside the folder |
| --------------------- | ------------------- | --------------------------------- | --------------------------- |
| پیش از هر تغییر بپرس  | run                 | ask (or "always" per tool)        | ask                         |
| خودکار در پوشه‌ی کاری | run                 | run                               | ask                         |
| فقط خواندنی           | run                 | refused                           | ask for reads, refuse rest  |

Writes inside `.git/` are always refused. A running command can be stopped
from its card. Output is capped at 30 KB (head and tail are kept), and
commands time out after 120 seconds unless the model asks for up to 30
minutes.

## Develop

Needs Rust 1.80+. On Linux it also needs the WebKitGTK 4.1 development
packages (`libwebkit2gtk-4.1-dev`, `libgtk-3-dev`, `libsoup-3.0-dev`).

```sh
cd agent/src-tauri
cargo test
cargo clippy --all-targets -- -D warnings
cargo run
```

Debug builds can point at a local stack:

```sh
SALAM_APP_URL=http://localhost:3000/ \
SALAM_API_URL=http://127.0.0.1:8080 \
SALAM_BRIDGE_URL=http://127.0.0.1:8081 \
cargo run
```

These variables are ignored in release builds.

## Names and icons

The window title and the product name are `Salam Agent`. The loader page and
the installer descriptions are Persian (ایجنت سلام). Filenames stay ASCII:
the binary is `salam-agent` and the identifier is `ir.salamlang.agent`.

The icons in `src-tauri/icons/` (including the multi-size `icon.ico` used for
the Windows executable) and `dist/favicon.ico` are generated from the
official logo, `design/logo/logo-box.svg`:

```sh
cd agent/src-tauri
bunx @tauri-apps/cli icon ../../design/logo/logo-box.svg -o icons
rm -rf icons/android icons/ios icons/Square*.png icons/StoreLogo.png
cp icons/icon.ico ../dist/favicon.ico
cp ../../design/logo/logo-box.svg ../dist/logo.svg
```

The app also sets its icon at runtime: the window icon on Windows and Linux,
and the Dock icon on macOS when the binary runs outside `Salam Agent.app`
(for example from a terminal or `cargo run`). Without that, macOS shows its
generic black "exec" icon for a bare executable.

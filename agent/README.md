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

## Project instructions and memory

Before each message in an agent chat the web app asks the app for the
workspace's context (`agent_project_context`): the project's `AGENTS.md`, or
`AGENT.md` when there is no `AGENTS.md` (up to 32 KB), and the memory index
(up to 16 KB). The backend passes them to the model when they change, so an
unchanged file is not resent on every turn.

The model keeps notes about the project with two tools, `save_memory` and
`forget_memory` (`src/memory.rs`). They write plain Markdown that is meant to
be read, edited and committed like any other file:

```text
.salam-agent/
  index.md                one line per memory, newest first, linking to its file
  memory/build-steps.md   one memory
```

```markdown
---
title: Build steps
description: How to build and test this project
created: 2026-10-26T02:30:00Z
updated: 2026-10-26T02:41:12Z
---

Run `salam build main.salam`, then `salam test`.
```

Saving with an existing slug updates that file and keeps its `created` time.
Memory calls run without asking in the two writing levels and ask in the
read-only one, and they never leave the workspace.

## Notifications

When the window is not in front (another window is on top, the app is
minimized, or on macOS it is hidden with Cmd+H), the app shows a system
notification and asks the OS to flash the taskbar or bounce the dock icon
(`src/notify.rs`):

- a reply is ready, or failed (the app watches every reply it starts)
- the server cannot be reached mid-reply (web app)
- a command or edit waits for approval (app)
- the assistant link is down for 20 seconds, and back again (app)
- this computer was unlinked, HTTP 401 (app)

A reply started in the window is handed to the app with `agent_watch_reply`,
and the app itself polls it while the window is away (`src/watch.rs`). Because
the check runs in Rust rather than in a page timer, the notification still
arrives when the window is minimized, the webview is throttled, or the page is
busy; each reply is announced once, whichever side sees it finish first.

Every one of these also appears inside the window as a toast in the corner,
with a short chime, even when the window is in front or the system
notification is unavailable. The bell next to the access level sets
notifications, sound and the reply preview, and its test button checks both
paths and reports which one worked. Settings are kept in
`notifications.json` in the app's config folder. The same notice is not
repeated within 4 seconds.

- Windows: toasts use the app's own identity when installed under
  `%LOCALAPPDATA%` or Program Files, and PowerShell's otherwise (portable ZIP).
- macOS: needs notification permission for Salam Agent in System Settings. A
  development build (not inside a `.app`) posts through `osascript`, because
  the notification centre drops notices from an unbundled binary.
- Linux: talks to the session's `org.freedesktop.Notifications` service over
  D-Bus (any desktop, or dunst/mako on a bare window manager). Clicking a
  notification raises the window. Without a service the taskbar hint still
  works and the test button explains what is missing.

## Access levels

You pick one per chat in the composer.

| Level                 | Reads in the folder | Writes and commands in the folder | Anything outside the folder |
| --------------------- | ------------------- | --------------------------------- | --------------------------- |
| پیش از هر تغییر بپرس  | run                 | ask (or "always" per tool)        | ask                         |
| خودکار در پوشه‌ی کاری | run                 | run                               | ask                         |
| فقط خواندنی           | run                 | refused                           | ask for reads, refuse rest  |
| همه‌چیز را خودکار کن  | run                 | run                               | run, without asking         |

The last level accepts every tool call everywhere on the computer. Choosing
it asks for confirmation first, and the select marks it in red while it is
active. It is meant for a task you fully trust, not as a default.

Writes inside `.git/` are always refused. A running command can be stopped
from its card. Output is capped at 30 KB (head and tail are kept), and
commands time out after 120 seconds unless the model asks for up to 30
minutes.

## When something goes wrong

The app never leaves WebKit's own error page on screen. `dist/error.html`
is a local, bilingual page with a reason, a short checklist and a retry
button:

- `#offline`: the network or the server cannot be reached at startup
- `#server`: the server answers with 5xx
- `#memory`: the macOS web content process was terminated (typically out
  of memory), caught by `on_web_content_process_terminate`
- `#crash`: the page stopped beating its heartbeat

The page probes the server with a tiny image request before retrying, so a
retry never navigates into a network error page. Offline and server pages
retry by themselves when the server comes back.

The web app sends `agent_alive` every 4 seconds, from an inline script in
`index.html` so it also runs while the bundle downloads and after a React
error (`src/watchdog.rs`). The watchdog only starts once the page has
finished loading, so a slow connection never counts as a crash. If the
window is focused and the heartbeat has been silent for 45 seconds, the app
asks the page to run a one-line script. A page that answers is alive and is
left alone. A page that does not answer for 15 seconds is reloaded once
silently, and the local error page appears only if that happens again within
two minutes (or the network is down). Minimized and unfocused windows are
never judged, and the silence timer restarts when focus returns.

## Locked window

The app should feel native, so `src/lock.js` is injected into every page of
the window. Text cannot be selected except in chat messages and input
fields, Ctrl+A only works inside inputs, and the context menu, drag, F12,
Ctrl+Shift+I/J/C, Ctrl+U/S/P/R and F5 are blocked. Devtools are switched
off. Debug builds skip all of it when `SALAM_DEVTOOLS` is set.

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

macOS uses its own master, `design/logo/logo-box-macos.svg`, because Apple's
icon grid differs from the other platforms: a 1024x1024 canvas with an 824x824
rounded body centred and transparent margins, so the Dock draws the icon at the
same size as native apps instead of shrinking a full-bleed square. Regenerate
it after the command above, since the Tauri generator replaces `icon.icns`
with the full-bleed artwork:

```sh
agent/scripts/make-macos-icon.sh
```

It writes `src-tauri/icons/icon.icns` and `src-tauri/icons/icon-macos.png`
(the image the app uses for the Dock when it is not bundled) and needs
`rsvg-convert`, `sips` and `iconutil`.

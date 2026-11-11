---
title: Desktop and GUI applications with Salam: from WebView apps to a native UI drawn by Salam itself
seo_title: Desktop GUI Apps in Salam: WebView, Native UI and Installers
description: How to build a desktop app in Salam: a WebView interface like Tauri, installers from one command, raylib games and a native UI that Salam draws itself.
keywords: desktop app Salam, GUI programming, WebView desktop app, Tauri alternative, Electron alternative, native UI, cross-platform desktop application, raylib game, salam bundle, desktop installer
category: build
summary: Salam has three ways to put a window on the screen: a web interface in the system's WebView, a native UI drawn by Salam from the page language, and raylib for games. What each one does, how to package it, and where each stops.
date: 2026-11-11
---

Salam can build desktop applications for Windows, macOS and Linux in three ways. **The `gui.webview` package builds a desktop app whose interface is HTML, CSS and JavaScript running in the web view the operating system already ships**, the same model as Tauri, with the program logic in Salam and no bundled browser. The `gui.ui` runtime takes a different route and draws a Salam page in a native window by itself, with no browser at all. For games and visual programs, `gui.raylib` binds the raylib library. One command, `salam bundle`, turns a project into installers for all three systems.

This article walks through each option with real code, shows how packaging and cross-building work, and is specific about what is finished and what is not.

## Three ways to build a GUI in Salam {#three-ways}

| Approach | Package | What draws the pixels | Good for |
|---|---|---|---|
| Web interface | `gui.webview` | WebView2, WKWebView or WebKitGTK | Tools and apps with a rich interface you already know how to build in HTML |
| Native UI from a page | `gui.ui` | Salam itself (layout, text, painting) | A page written in Salam's page language, shown without a browser engine |
| Games and graphics | `gui.raylib` | raylib over OpenGL | Games, simulations, visual toys, teaching |

The first is the mature path: it has a security model, system services, installers and a CI workflow on real Windows, macOS and Linux machines. The second is newer and still growing. The third is a binding to a well-known C library, so it inherits raylib's strengths and its requirements.

## Desktop apps with a WebView, the Tauri model {#webview}

Electron ships a full copy of Chromium with every application. Tauri showed that you can instead use the web view the operating system already has, which keeps the download small. Salam's `gui.webview` follows the second model, without Rust and without a second language for the back end:

| System | Engine | Needed at run time |
|---|---|---|
| Linux | WebKitGTK | `libgtk-3-0`, `libwebkit2gtk-4.1-0` |
| Windows | WebView2 | The WebView2 runtime, included in current Windows |
| macOS | WKWebView | Nothing |

`salam new notes --desktop` writes a complete project: `main.salam`, a `salam.toml`, a `.gitignore` and a `www` folder with `index.html`, `app.js` and `style.css`. Add `--lang=fa` and the project is written in Persian. The generated entry point is short:

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

`Init` describes the application, `Command` registers a function the page can call, `Open` creates a window and `Run` drives the event loop until the user quits. The page calls the command like any async JavaScript function:

```javascript
const text = await salam.invoke("greet", { name: "Sara" });
```

During development `assets_dir` serves the folder directly, so a reload shows your edits. In a release build the same files are packed inside the executable and served from `app://localhost` (`https://app.localhost` on Windows). A page can also come from a remote site (`url` in `WindowOptions`) or from an HTML string (`html`).

### Talking between the page and Salam {#bridge}

Three calls cover the whole bridge:

| From the page | In Salam |
|---|---|
| `await salam.invoke(name, args)` | `webview.Command` runs on the UI thread, `webview.CommandAsync` on a worker |
| `salam.emit(name, data)` | `webview.On(name, handler)` |
| `salam.listen(name, fn)` | `webview.Emit(name, json)`, or `win.Emit` for one window |

Arguments and results travel as JSON. A command answers exactly once, with one of the `Resolve` methods or with `Reject`, which makes the page's promise throw. Slow work belongs in `CommandAsync`, because a plain `Command` runs on the UI thread and would freeze the window. Window calls belong on the UI thread, so a worker hands them over with `webview.Dispatch`. If you have used the [threads lesson](/learn/threads/), this is the usual rule of every GUI toolkit, written down explicitly.

### Security model {#security}

A desktop app that runs web content needs a clear line between the page and the machine. `gui.webview` draws it like this:

- Local pages may run every command. Remote origins may run nothing until you name them with `webview.Allow(origin, commands)`.
- The bridge is installed only in the top frame of an allowed origin. Every message carries a secret created for this run, checked together with the page's origin, so an embedded frame or another site cannot call a command.
- A link to any other origin opens in the system browser, and paths that try to leave the assets folder are refused.
- The packed assets are compressed and scrambled so they do not show up in `strings`, but that hides them; it does not protect them. Secrets belong on a server.

### System services {#system-services}

The same Salam call maps to the native facility on each system: file dialogs (`PickFolder`, `PickFile`, `SaveFile`), message boxes (`Message`, `Confirm`), notifications (`Notify`), the keychain (`SecretSet`, `SecretGet`, `SecretDelete`, backed by libsecret, Credential Manager or Keychain), a menu bar (`SetMenu`), a tray icon (`SetTray`), a single-instance lock (`SingleInstance`) and `OpenURL`. When a Linux machine has no secret service, a secret falls back to a `0600` file in the app's data folder. The [gui.webview reference](/std/gui/webview/) has a guide for each of these.

## Packaging installers with salam bundle {#bundle}

Describe the app in an `[app]` table in `salam.toml`:

```toml
[app]
id = "com.example.notes"
name = "Notes"
version = "1.0.0"
main = "main.salam"
assets = "www"
icon = "icon.png"
```

Then one command builds the packages:

```bash
salam bundle
salam bundle --target=x86_64-w64-windows-gnu
salam bundle --target=arm64-apple-macos --format=app,tar
```

| System | Default formats | Also, when the tool is on `PATH` |
|---|---|---|
| Linux | `tar`, `deb` | `rpm` (`rpmbuild`), `appimage` (`appimagetool`) |
| Windows | `zip` | `nsis` (`makensis`) |
| macOS | `app`, `tar` | `dmg` (`hdiutil`) |

Cross builds need nothing extra, because the compiler links Windows and macOS programs itself (see [cross-compiling Salam](/blog/cross-compilation-salam/)). On Linux and Windows the assets are appended to the executable; on macOS they go into `Contents/Resources/app.pack` so the program's signature stays valid. A PNG icon is converted to `.ico` and `.icns` for you.

> [!WARNING]
> Nothing is signed for you. macOS builds get the ad-hoc signature the compiler writes, which is enough to run after removing the quarantine flag, but real distribution still needs proper signing and notarization, as with any other toolchain.

For Windows programs that should not open a console, `salam build` also takes `--subsystem=windows`, `--icon=FILE.ico` and `--version-info=KEY=VALUE`, which need the native backend or the built-in linker.

### A Chromium engine, and its current limits {#chromium}

Some apps need the same engine on every system. Building with `-DSALAM_WEBVIEW_CEF` and bundling with `--engine=chromium --runtime=cef-dist` swaps in Chromium through the Chromium Embedded Framework. This engine is not complete yet: windows open and pages load, but there is no message channel, no custom scheme and none of the system services, so `webview.SupportsCommands()` returns false and a page cannot call your commands. For an application today, the system web view is the engine to use.

## A native UI that Salam draws itself {#native-ui}

A web view still means depending on a browser engine and accepting its start time and memory use. Salam's second approach removes the browser entirely. A program written in the [page language](/learn/layout/) can be compiled with `--ui=native`, and the `gui.ui` runtime shows it in a native window that Salam draws pixel by pixel, the way Slint and Flutter draw their widgets instead of using the operating system's.

The runtime is a stack of Salam packages: `gui.window` (X11, Win32, Cocoa and a headless backend), `gui.paint` (anti-aliased paths, borders, gradients, shadows), `gui.typeset` (TrueType fonts, shaping, bidirectional text and line breaking, so Persian works from the start), `gui.cssbox` (block, flex, grid and table layout), `gui.css` (the cascade) and `gui.reactive`. Here is a complete counter:

```salam
mut count := 0

page:
    title = "Counter"
    box:
        heading: content = "Count: " + count end
        button: id = "add" content = "+1" click = (): count++ end end
    end
end
```

The same file builds both ways:

```bash
salam page build counter.salam            # counter.html and counter.js for the web
salam run counter.salam --ui=native       # the same page in a native window
salam build counter.salam --ui=native     # a native executable
```

For a native build the compiler does all the style work at compile time: every CSS declaration is parsed, validated and expanded while compiling, and the page becomes a function of builder calls. No HTML or CSS text is parsed when the program runs. After each handler, only the parts of the view that read changed state run again, and hover, focus, scroll positions and typed text survive. The article on [components, state and events](/blog/interactive-ui-components-state-events/) covers that reactive model.

The native runtime can run headless for tests. Building the counter above on Linux x86-64 gave a 2.5 MB executable; run with `SALAM_WINDOW_BACKEND=headless` and a script that clicks the button three times and then prints the element tree, it reported:

```text
body 8 21 1008 81
  div 8 21 1008 81
    h1 "Count: 3" 8 21 1008 38
    button#add "+1" focused 8 80 36 22
```

You can also drive `gui.ui` from an ordinary program. `Handle` registers a named handler, the view is a function that returns the document, and the `headless` and `script` options run it without a display, which is how this example runs on the website's build machine:

```salam
import gui.ui

mut done := 0

func view(): str:
    ret `<!doctype html><html><head><title>Tasks</title></head><body><h1>Done: ` + done + `</h1><button id="finish" onclick="finish()">Finish one</button></body></html>`
end

func main:
    ui.Handle("finish", (): done++ end)
    opts := ui.Options { title = "Tasks", width = 320.0, height = 200.0, headless = true, script = "click #finish\nclick #finish\nquit" }
    ui.RunWith((): ret view() end, opts)
    println "tasks finished:", done
end
```

```output
tasks finished: 2
```

### What the native target cannot show {#native-limits}

Some page features only make sense in a browser. With `--ui=native` the compiler refuses elements such as `script`, raw `style`, `iframe` and inline `svg` with error E165, and ignores web metadata such as `meta` with warning W166. A page can keep web-only parts next to portable ones with `if SALAM_UI_WEB:`. The design document lists what is still planned: accessibility from `aria` roles, a GPU renderer, Wayland (today the Linux backend is X11), an SVG subset, and Android and iOS. For an app that must be accessible to screen readers today, the web view path is the safer choice.

## Games and graphics with raylib {#raylib}

`gui.raylib` binds [raylib](https://www.raylib.com/) 5.5, a small C library for 2D and 3D games. The function names follow the raylib cheat sheet, so any raylib tutorial translates almost line by line:

```salam
import gui.raylib

func main:
    raylib.InitWindow(800, 450, "Bouncing ball")
    raylib.SetTargetFPS(60)
    mut x := 400.0
    mut dx := 240.0
    until not raylib.WindowShouldClose():
        dt := raylib.GetFrameTime() as f64
        x += dx * dt
        if x < 20.0 or x > 780.0:
            dx = 0.0 - dx
        end
        raylib.BeginDrawing()
        raylib.ClearBackground(raylib.RayWhite())
        raylib.DrawCircle(x as int, 225, 20.0, raylib.Maroon())
        raylib.EndDrawing()
    end
    raylib.CloseWindow()
end
```

Remember that `until` loops while its condition is true, so the loop runs until the window should close. Programs link against the shared `libraylib`, which you install separately. Raylib passes structs such as `Color` and `Vector2` by value; Salam handles that on x86-64 and AArch64 through the default backends, while the native x86-64 backend refuses such calls with a clear message. This is ordinary [C interop](/blog/salam-c-interop-ffi/), and the same technique works for other C GUI libraries.

## How it is tested {#testing}

Desktop code is notoriously easy to break on the system you did not try. The repository's desktop workflow runs a smoke test through a real window, bundles a sample project and runs it with `--self-test` on real Linux, Windows and macOS machines, and builds every package format, including `salam bundle` on Windows and macOS themselves. Window-free logic lives in the normal test suite, and the Windows backend is also tested under Wine against a WebView2 mock.

The project uses the package for its own software too. The Salam Agent desktop app has a port written entirely in Salam on `gui.webview`, tested by that workflow; the released [Salam Agent](/agent/) is still the original Tauri build, so treat the Salam port as a large worked example rather than as the shipping product.

## Choosing an approach {#choosing}

- **Use `gui.webview`** for most desktop applications today. You get a familiar interface stack, system dialogs, a keychain, a tray icon, installers for three systems and a security model that has been thought through.
- **Use `--ui=native`** when you want one page source for the web and the desktop, no browser engine at all, and you can live with a runtime that is still adding features. It is also the interesting option for Persian interfaces, because shaping and bidirectional text are built in.
- **Use `gui.raylib`** for games and anything that redraws every frame.

Whichever you pick, the rest of the program is ordinary Salam, so the HTTP clients, databases and file handling of the [standard library](/std/) are available from your commands and handlers. The [web development article](/blog/web-development-with-salam/) explains the page language that both the web and the native targets start from.

## Frequently asked questions {#faq}

### Can Salam replace Electron or Tauri?

For many apps, yes: `gui.webview` uses the system web view like Tauri, so there is no bundled browser, and the back end is Salam instead of Rust or Node. It is much younger than either project and has a far smaller community, so expect to read the package reference rather than search for answers.

### Does a Salam desktop app need a runtime installed?

On macOS, no. On Windows it needs the WebView2 runtime, which current Windows includes. On Linux it needs GTK 3 and WebKitGTK 4.1, which most desktop distributions provide. A native `--ui=native` program needs neither, since Salam draws the window itself.

### Can I build a Windows installer from Linux?

Yes. `salam bundle --target=x86_64-w64-windows-gnu` cross-builds the program and packs a zip, and an NSIS installer when `makensis` is on your `PATH`. Signing is still up to you.

### Is the native UI ready for production?

It works and is tested headless in the repository, but it is the newest of the three paths and its roadmap still lists accessibility, Wayland and a GPU renderer. Try it for internal tools and prototypes first.

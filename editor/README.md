# Salam Web Playground

Run Salam entirely in the browser. Edit code, run it, and preview layout pages -
powered by the Salam compiler compiled to **WebAssembly** plus a small
text editor. Switching to **فارسی** flips the whole UI **and** the editor to
true right-to-left (right-aligned, lines starting from the right); English is
left-to-right. No server, no C toolchain in the browser.

> The editor is a plain `<textarea>` precisely so it can be genuinely RTL in

## How it works

The browser cannot run a C compiler, so general programs run through Salam's
**tree-walking interpreter** (`src/interp`) rather than the C-codegen backend.
The whole frontend (lexer → parser → semantic analyzer) plus the interpreter
and the layout engine are compiled to `salam-wa.wasm`/`salam-wa.js` with
Emscripten and driven from JavaScript:

| Mode       | JS entry point                         | What runs                                   |
| ---------- | -------------------------------------- | ------------------------------------------- |
| **App**    | `salam_web_run_app(source, lang)`      | the interpreter; returns the program output |
| **Layout** | `salam_web_build_layout(source, lang)` | the layout backend; returns an HTML page    |

The editor (`editor/app.js`) is bilingual: switching **English / فارسی** updates
both the UI strings and the document **direction (ltr/rtl)** in sync, and ships
matching English and Persian sample programs.

## Building the Wasm module

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):

```sh
git clone https://github.com/emscripten-core/emsdk
cd emsdk && ./emsdk install latest && ./emsdk activate latest && . ./emsdk_env.sh
cd /path/to/Salam/compiler
sh tools/bash/build-wasm.sh # -> ../editor/salam-wa.js, ../editor/salam-wa.wasm, ../editor/salam-wa.data
```

`build-wasm.sh` preloads the `std/` directory into the module's virtual filesystem so
`import` resolution and the layout schema work in the browser.

## Running it

Serve the `editor/` directory over HTTP (Wasm needs a real origin, not `file://`):

```sh
sh compiler/tools/bash/editor-serve.sh        # http://localhost:8080  (no rebuild for UI dev)
sh compiler/tools/bash/editor-serve.sh 9000   # ... on a custom port
```

Or serve it directly: `cd editor && python -m http.server 8080`.

If `salam-wa.js` is missing, the editor still loads and shows a banner telling
you to run `compiler/tools/bash/build-wasm.sh`.

## macOS 26 setup (Homebrew)

Install the required tools with Homebrew, then build and run the editor:

```sh
brew update
brew install emscripten python

cd /path/to/Salam/compiler
sh tools/bash/build-wasm.sh

cd /path/to/Salam
sh compiler/tools/bash/editor-serve.sh 8080
```

Then open `http://localhost:8080` in your browser.

## Trying the interpreter without the browser

The same interpreter runs natively:

```sh
salam exec examples/fa/types/07_struct.salam --lang=fa
salam run  --interp examples/en/apps/brainfuck.salam
```

## Scope

The interpreter targets pure-compute programs (functions, structs, enums,
arrays, `Vector`, `HashMap`, lambdas/closures, operator overloading, generics,
`defer`, `char`/strings, control flow) in both English and Persian, and matches
the C backend's output across the example suite. It intentionally does **not**
support things that need a real OS or the C ABI (file/socket/thread/HTTP/SQLite
APIs, FFI/`extern "C"`, and manual memory (`salam_alloc`)), none
of which apply in a browser sandbox. The `&ref` scalar parameters, multi-file
user imports, and the fully Persian-localized std API (`ریاضی.جذر`) are current
limitations of the interpreter, not the language.

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->

- [Salam Web Playground](#salam-web-playground)
  - [How it works](#how-it-works)
  - [Building the Wasm module](#building-the-wasm-module)
  - [Running it](#running-it)
  - [Trying the interpreter without the browser](#trying-the-interpreter-without-the-browser)
  - [Scope](#scope)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

# Salam Web Playground

Run Salam entirely in the browser. Edit code, run it, and preview layout pages -
powered by the Salam compiler compiled to **[WebAssembly](https://webassembly.org/)** plus a small
text editor. Switching to **فارسی** flips the whole UI **and** the editor to
true [right-to-left](https://en.wikipedia.org/wiki/Right-to-left_script) (right-aligned, lines starting from the right); English is
[left-to-right](https://en.wikipedia.org/wiki/Writing_system#Directionality). No server, no C toolchain in the browser.

> The editor is a plain `<textarea>` precisely so it can be genuinely RTL in

## How it works

The browser cannot run a C compiler, so general programs run through Salam's
**[tree-walking interpreter](<https://en.wikipedia.org/wiki/Interpreter_(computing)#Tree-walking_interpreters>)** (`src/interp`) rather than the C-codegen backend.
The whole frontend ([lexer](https://en.wikipedia.org/wiki/Lexical_analysis) → [parser](https://en.wikipedia.org/wiki/Parsing) → [semantic analyzer](<https://en.wikipedia.org/wiki/Semantic_analysis_(compilers)>)) plus the interpreter
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

## Trying the interpreter without the browser

The same interpreter runs natively:

```sh
salam exec examples/fa/types/07_struct.salam --lang=fa
salam run  --interp examples/en/apps/brainfuck.salam
```

## Scope

The interpreter targets pure-compute programs (functions, structs, enums,
arrays, [`Vector`](https://en.wikipedia.org/wiki/Dynamic_array), [`HashMap`](https://en.wikipedia.org/wiki/Hash_table), [lambdas](https://en.wikipedia.org/wiki/Anonymous_function)/[closures](<https://en.wikipedia.org/wiki/Closure_(computer_programming)>), [operator overloading](https://en.wikipedia.org/wiki/Operator_overloading), [generics](https://en.wikipedia.org/wiki/Generic_programming),
[`defer`](<https://en.wikipedia.org/wiki/Defer_(programming_idiom)>), `char`/strings, control flow) in both English and Persian, and matches
the C backend's output across the example suite. It intentionally does **not**
support things that need a real OS or the [C ABI](https://en.wikipedia.org/wiki/Application_binary_interface) (file/socket/thread/HTTP/SQLite
APIs, [FFI](https://en.wikipedia.org/wiki/Foreign_function_interface)/`extern "C"`, and manual memory (`mem.Allocate`)), none
of which apply in a browser sandbox. The `&ref` scalar parameters, multi-file
user imports, and the fully Persian-localized std API (`ریاضی.جذر`) are current
limitations of the interpreter, not the language.

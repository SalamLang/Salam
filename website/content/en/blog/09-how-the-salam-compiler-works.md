---
title: From source code to executable: how the Salam compiler works, stage by stage
seo_title: How the Salam Compiler Works: The Compiler Pipeline Explained
description: How a compiler works, shown on a real one: Salam's lexer, parser, AST, semantic checks, error codes, C, LLVM and native code generation, and linking.
keywords: how a compiler works, Salam compiler, compiler pipeline, lexer and parser, abstract syntax tree, semantic analysis, code generation, compiler error messages, salam check, salam inspect
category: compiler
summary: Follow a Salam program through the compiler: tokens, syntax tree, type checking and static checks, generated C or LLVM IR or machine code, and the final link, with real output from each stage.
date: 2026-11-11
---

A compiler is a program that reads source code, checks that it means something, and translates it into a form a machine can run. The Salam compiler does this in five broad stages: it **lexes** the text into tokens, **parses** the tokens into a syntax tree, **analyzes** the tree for types and mistakes, **generates code** through one of three backends (C, LLVM or its own native code generator for x86-64, ARM and RISC-V), and **links** the result into an executable. This article follows a real program through each stage of the Salam compiler, with the actual output of every step.

Everything here can be reproduced with the `salam` command. Where the article says "the compiler does X", the file that does it is named, so you can read the source on [GitHub](https://github.com/SalamLang/Salam).

## The compiler pipeline at a glance {#pipeline}

**A compiler pipeline is a chain of stages, each of which turns one representation of the program into the next.** Salam's compiler is written in Salam itself (see [the self-hosting article](/blog/self-hosting-compiler/)), and its source sits in `compiler/`, one directory per stage. The entry point, `compiler/main.salam`, is short: it reads the command-line arguments and hands them to `cli.Run`. Everything else hangs off that call.

| Stage | Input | Output | Directory | Size (lines) |
|---|---|---|---|---|
| Lexer | source text | tokens | `compiler/lexer/` | about 2,800 |
| Parser | tokens | syntax tree (AST) | `compiler/parser/` | about 5,100 |
| Semantic analysis | AST | typed, checked AST | `compiler/semantic/` | about 40,700 |
| C backend | checked AST | C source | `compiler/codegen/` | about 8,900 |
| LLVM backend | checked AST | LLVM IR | `compiler/llvmgen/` | about 11,900 |
| Native backend | LLVM IR text | machine code | `compiler/nativegen/` | about 52,600 |
| Built-in linker | object files | executable | `compiler/nativelink/` | about 12,600 |
| Driver | command line | the whole run | `compiler/driver/` | about 9,500 |

The numbers say something about where the work is. Lexing and parsing are the small, well-understood part. The semantic analysis is eight times bigger than the parser, because that is where a strict language earns its strictness. The native backend is the largest piece of all, since it contains a full optimizer and writes machine code for five processor families. Counting the interpreter, the JavaScript backend, the formatter, the language server and the page language, the compiler is about 340 source files and close to 200,000 lines.

We will follow this program through the pipeline:

```salam
func square(n: int): int:
    ret n * n
end

func main:
    println square(7)
end
```

```output
49
```

## Stage 1: reading the source and lexing {#lexing}

Before the lexer runs, the driver (`compiler/driver/driver_front.salam`) loads the file and decides which **language pack** it is written in. Salam source can be English or Persian, and the compiler detects which one from the keywords it finds, unless the file says so in a header comment. The pack decides which words are keywords: `func` in one file, `تابع` in another. After this point both languages produce the same tokens, which is why the rest of the compiler never has to care which one you wrote. The article [One language, two scripts](/blog/one-language-two-scripts/) covers the Persian side in depth.

The driver also builds the table of **conditional-compilation flags** for the target: `SALAM_OS_LINUX`, `SALAM_OS_WINDOWS`, `SALAM_ARCH_ARM64` and so on, plus anything you pass with `-D`. That table is used right after parsing.

The lexer (`compiler/lexer/`) then walks the text once and cuts it into **tokens**: keywords, identifiers, literals, operators and statement ends. Numbers, strings (including Salam's raw backtick strings and multi-line text blocks), escapes and identifiers each have their own file in that directory. You can see the token stream for any file with `salam inspect`:

```bash
salam inspect app.salam --emit-tokens
```

For a three-line `main` that declares `x := 1 + 2` and prints `x`, the stream is fifteen tokens, here with only their types:

```text
KEYWORD_FUNC IDENTIFIER COLON IDENTIFIER COLON_ASSIGN INT_LITERAL PLUS
INT_LITERAL STMT_END KEYWORD_PRINTLN IDENTIFIER STMT_END KEYWORD_END
STMT_END EOF
```

Each token in the XML also records its line and column, which is how every later error message can point at the exact character.

## Stage 2: parsing into a syntax tree {#parsing}

**A parser turns the flat list of tokens into a tree that shows how the program is nested**: this call is an argument of that `println`, this block belongs to that function. Salam's parser (`compiler/parser/`) is a hand-written recursive descent parser. Statements and declarations each get their own function, and expressions use the Pratt technique: every operator has a left and a right binding power (`+` and `-` bind at 60, `*`, `/` and `%` at 70, comparisons at 50, `and` at 30, `or` at 20), and one loop in `parse_expr_bp` builds the correct tree for any mix of operators. Precedence lives in one small table instead of a dozen grammar rules.

The result is an **abstract syntax tree** (AST). Salam's AST has about 70 node kinds (`FunctionDef`, `VariableDecl`, `Call`, `Literal` and so on). Nodes are stored in large pages and refer to each other by integer index rather than by pointer, which keeps the tree compact and cheap to walk, an important detail for a compiler that has to hold all of itself in memory when it builds itself.

Right after parsing, the driver **prunes conditional code**. A top-level block such as `if SALAM_OS_WINDOWS:` is evaluated against the flag table from stage 1, and the branches that do not apply are removed before anyone type-checks them. That is how one source file can declare Windows and Linux versions of the same external function without a conflict. The [compile-time lesson](/learn/compile-time/) shows how to use it.

When a package is spread over several files in one directory, those files are lexed and parsed too, and their trees are merged into one program before analysis.

## Stage 3: semantic analysis, where most of the work happens {#semantic-analysis}

**Semantic analysis is the stage that decides whether a syntactically valid program actually makes sense.** It is the biggest part of the Salam compiler, spread over 71 files in `compiler/semantic/`. The entry point, `Run` in `sema_api.salam`, goes through these steps in order:

1. **Load imports**, recursively, and load the `core` package that every program gets without asking (it holds things like `Error` and `Option`).
2. **Collect declarations**: every struct, enum, interface, function and constant is entered into a scope before any body is checked, so functions can call each other in any order.
3. **Check top-level order**: imports, then constants, then types, then functions, a rule Salam enforces to keep files readable.
4. **Check every function**: resolve names, infer the type of every expression, check every call against its signature, instantiate generics, check `match` for exhaustiveness, track definite initialization, and run the value-range analysis and the ownership analysis described in [the static typing article](/blog/static-typing-in-salam/) and [the memory article](/blog/memory-management-in-salam/).
5. **Report unused functions and imports**, which are errors in Salam, not warnings.

The analysis also rewrites the tree. You can see the result with `--emit-ast`, which prints the *decorated* tree, types included. For `x := 1 + 2`, the declaration comes out like this (location elements removed):

```text
<node kind="VariableDecl" name="x" type="i32" mut="false">
  <init>
    <node kind="Literal" type="i32" op="INT_LITERAL">
      <value kind="int">3</value>
    </node>
  </init>
</node>
```

Three things happened. The type of `x` was inferred as `i32` (Salam's `int`). The mutability was recorded as `false`, so any later assignment to `x` is an error. And `1 + 2` was folded into the constant `3` before any backend saw it. Loops are lowered too: an `each v in values:` over an array or slice comes out of analysis as an index loop with a compiler-generated counter named `__ei1`.

## What the error messages look like {#diagnostics}

A compiler is only as useful as its errors. Every Salam diagnostic has a severity, a code, the file and position, the source line with a caret, and often a help or fix line. Here are four common mistakes and what `salam check` prints for each (the last two cut to their first lines).

Reassigning a variable that was not declared `mut`:

```salam fragment e1.salam
func main:
    count := 10
    count = count + 1
    println count
end
```

```text
error[E013]: cannot reassign immutable variable 'count'; declare it 'mut' to reassign it, or assign to its elements or fields instead
 --> e1.salam:3:5
  |
3 |     count = count + 1
  |     ^
  |
```

A typo in a name. The compiler suggests the closest name in scope, and then also reports the variable that is now unused:

```text
error[E001]: unknown identifier 'totl'; did you mean 'total'?
 --> e4.salam:3:13
  |
3 |     println totl
  |             ^
  |
  = fix: Change to 'total'
```

Passing the wrong type, and indexing past the end of a fixed array:

```text
error[E012]: argument 2: cannot pass 'str' to 'i32'
 --> e2.salam:6:13

error[E021]: index 5 is out of bounds for an array of length 3
 --> e3.salam:3:13
```

The same diagnostics come in other shapes. `--error-style=gcc` gives the one-line `file:line:col: error:` form that editors and build tools parse, and `--error-format=json` gives one JSON object per diagnostic for scripts and other tools:

```json
{"level":"error","code":"E013","message":"cannot reassign immutable variable 'count'; declare it 'mut' to reassign it, or assign to its elements or fields instead","file":"e1.salam","line":3,"column":5}
```

With `--error-lang=fa` the same error is printed in Persian, code included, written with Persian digits (`خطا۰۱۳`). A Persian source file gets Persian errors by default.

> [!NOTE]
> Codes group related problems rather than naming each message: E021, for example, covers both an out-of-range literal index and a non-boolean `if` condition. Read the message as well as the number.

## Stage 4: code generation with three backends {#codegen}

**Code generation turns the checked tree into a lower-level language.** Salam has three backends for native code, and they share everything before this point:

- **C** (`compiler/codegen/`) writes C source and hands it to a C compiler.
- **LLVM** (`compiler/llvmgen/`) writes LLVM IR, which LLVM optimizes and turns into machine code.
- **Native** (`compiler/nativegen/`) reads the LLVM IR text that `llvmgen` produced, but instead of calling LLVM it runs its own optimizer and writes x86-64, AArch64, 32-bit x86, 32-bit ARM or RISC-V machine code directly.

There is also a JavaScript backend for the browser and an interpreter, covered in [the article on salam run and salam exec](/blog/salam-run-vs-salam-build/). The C backend is the easiest to read, so here is what it produces for `square` and `main` (from `salam build sq.salam --emit-c`, with the program above saved as `sq.salam`; the option keeps the generated files in `.salam-build/`):

```c
static int32_t _Salam_main_square_i32(int32_t n) {
    (void)n;
    return ((int32_t)((uint32_t)(n) * (uint32_t)(n)));
}

int main(int argc, char** argv) {
    salam_set_args(argc, argv);
    salam_mod_sq_init();
    (void)({ int64_t __po1 = (int64_t)(_Salam_main_square_i32(7)); salam_out_lock(); salam_out_i64(__po1); salam_out_put_("\n", 1); SALAM_OUT_END1(); salam_out_unlock(); });
    return 0;
}
```

Two details are worth noticing. The function name is **mangled** with its package and parameter types (`_Salam_main_square_i32`), which is how overloads and same-named functions in different packages stay apart. And the multiplication goes through `uint32_t`: in C, signed overflow is undefined behavior, so a development build multiplies as unsigned and converts back, which gives the wrap-around result on every C compiler. A `--release` build drops that guarantee in exchange for speed. The article [Three ways to generate code](/blog/salam-code-generation-c-llvm-native/) compares the backends in detail.

## Stage 5: compiling, caching and linking {#linking}

The last stage depends on the backend.

With the **C backend**, every source file, and every standard library module it uses, becomes its own pair of files (`salam_mod_<name>.c` and `.h`). The driver compiles them in parallel, one worker thread per CPU core, and keeps the object files. The next build recompiles only what changed, and the log says so (here for a small prime sieve program):

```text
[DRIVER][INFO] compiling 1 object file in parallel, reused 9 cached objects
[DRIVER][INFO] built executable: sieve
```

If nothing changed at all, `salam build` prints `up to date` and does no work. `-B` forces a full rebuild.

With the **LLVM backend**, the whole program becomes one LLVM module. The main release builds for Linux, macOS and Windows carry LLVM and its linker lld inside the `salam` binary, so they optimize and link in the same process with no outside tools. A build optimizes at `-O2` by default and `-O3` with `--release`.

With the **native backend**, the compiler writes the object file itself (ELF, COFF or Mach-O) and, in most cases, links it with its own built-in linker. The build log of a hello-world program shows the whole link in one line:

```text
built-in link: 5 objects, 15 live sections, 28 symbols, 1 GOT loads relaxed, 4 merge bytes to 4, 11488 bytes
```

That is an 11,488-byte executable produced without running any external compiler, assembler or linker program.

## Stopping early: salam check and salam inspect {#check-and-inspect}

Most of the time you do not need an executable; you need to know whether the code is right. Two commands stop the pipeline early:

- **`salam check`** runs everything up to code generation, imported packages included, prints the diagnostics, and exits with status 1 if there are any errors. It is the fast loop to run after every edit; the language server runs the same analysis inside your editor. It accepts a single file or a whole directory.
- **`salam inspect`** dumps one stage as XML: `--emit-tokens`, `--emit-ast` or `--emit-symbol` (the symbol table). It is the tool for learning how the compiler sees your code, and the project's porting guide recommends it for checking a re-implemented stage against a reference compiler, output against output.

```bash
salam check src/
salam check app.salam --error-format=json
salam inspect app.salam --emit-ast --xml-out=ast.xml
```

`salam check` cannot catch everything. A problem that only shows up in the generated code, the C compiler or the linker (a missing system library, for example) needs a real `salam build`, and the help text says so.

## Why the pipeline is shaped like this {#design}

A few design decisions explain most of the structure:

- **One front end, many backends.** Because lexing, parsing and analysis are shared, a new language feature is checked once and reaches every backend. The native backend goes further and reuses the LLVM backend's output, so the two cannot disagree on what a program means.
- **Checks before code.** Salam treats many things other compilers warn about (unused variables, dead branches, reassigning an immutable value, a definite out-of-range index) as errors, and it finds them before generating anything. The [rules lesson](/learn/rules/) lists the main ones.
- **Two languages, one tree.** The language pack is chosen at the very start. After lexing, English and Persian programs share one tree and one set of checks; the pack only decides which spellings of names are accepted and which language the messages use.
- **Everything is inspectable.** Tokens, the AST, the symbol table, the generated C, the LLVM IR (`salam llvm`) and the native assembly (`--emit-asm`) can all be printed. For students, this makes the Salam compiler a working, open textbook.

If you want to watch it happen on your own code, install the compiler from the [install page](/install/), or use the [online editor](https://editor.salamlang.ir), which runs the same compiler in the browser.

## Frequently asked questions {#faq}

### What are the main stages of a compiler?

Lexing (text to tokens), parsing (tokens to a syntax tree), semantic analysis (types and rules), code generation (tree to a lower-level language or machine code) and linking (objects to an executable). Many compilers add an optimization stage between analysis and code generation; in Salam it lives inside the LLVM and native backends.

### What language is the Salam compiler written in?

Salam. The compiler was ported from an earlier C implementation, which was removed from the repository in August 2026, and it has built itself ever since.

### Does Salam compile to C?

It can. The C backend is one of three; the others are LLVM and a native code generator that writes machine code directly. Official release builds use LLVM by default when they carry it, and you can pick any backend with `--backend=c`, `--backend=llvm` or `--backend=native`.

### How do I see what the compiler does with my code?

Use `salam inspect --emit-tokens`, `--emit-ast` or `--emit-symbol` for the front end, `salam build --emit-c` for the generated C, `salam llvm` for LLVM IR and `salam build --backend=native --emit-asm` for the machine code as assembly.

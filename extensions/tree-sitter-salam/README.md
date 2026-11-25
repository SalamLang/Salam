# tree-sitter-salam

A [tree-sitter](https://tree-sitter.github.io/) grammar for Salam, for editors
that highlight, fold and select code with tree-sitter: Neovim, Helix, Zed,
Emacs 29+ and others. English and Persian Salam parse the same way.

It parses 97.8% of the 4,230 valid Salam files in this repository (everything
under `tests/`, `std/` and `compiler/` outside an `errors/` directory) without
a single error node, and tree-sitter recovers locally in the rest.

## What is generated

The keyword spellings in both languages come from the compiler's own tables.
`extensions/tools/gen_syntaxes.salam` writes them into:

- `keywords.json`, which `grammar.js` reads
- `src/keywords.h`, which the external scanner reads
- `queries/highlights.scm`, here and in `extensions/vim/queries/salam/`

Edit the generator, not those files.

## Queries

`queries/` holds the hand-written files next to the generated highlights:

| File              | What it does                                                                                                                                                                                                                                               |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `highlights.scm`  | colours; page elements are `@tag`, their attributes `@tag.attribute` (generated)                                                                                                                                                                           |
| `folds.scm`       | every block that ends in `end`: functions, types, `page`, `component`, page elements, branches                                                                                                                                                             |
| `indents.scm`     | `@indent.begin` for the same blocks, `@indent.end` and `@indent.branch` for `end` and `else`, in both languages                                                                                                                                            |
| `locals.scm`      | scopes (functions, lambdas, blocks, `page`, `component`, page elements), definitions (parameters, variables, loop variables, page variables, functions, types) and references                                                                              |
| `textobjects.scm` | `nvim-treesitter-textobjects` names: `function`, `class` (struct, interface, enum, impl, component), `block` (`page` and every element), `attribute`, `conditional`, `loop`, `parameter`, `call`, `return`, `assignment`, `statement`, `comment`, `number` |
| `injections.scm`  | line and block comments are parsed as the `comment` language, which marks todo and fixme notes                                                                                                                                                             |

A Salam block has no body node, so `function.inner`, `class.inner` and
`block.inner` are the runs of statements, members or elements between the
colon and `end`. Strings holding SQL or regular expressions are not injected.
`tree-sitter.json` lists highlights, injections and locals; folds, indents
and text objects are found by editors by filename. `build.sh` checks that
every file in `queries/` compiles against the grammar by running it on
`test/queries/sample.salam`, which has a component, a page, a struct and a
function.

## The external scanner

Salam reads words that stand next to each other on one line as one name
(`fn ty`, `عدد اول`), and a keyword ends the name. A regular expression cannot
know the keywords, so `src/scanner.c` reads names word by word. Where the
compiler reads keywords as plain words, the grammar asks the scanner for a
different token: a name after `.` may start with a keyword
(`رشته.پایان با(...)`), a layout tag or attribute name runs to its `:` or `=`
(`border block end color`), and an import path runs to the end of the line.
`page`, `component` and `dyn` are keywords only where the parser can take
them, so `page := http.Get(...)` is a variable. A line in a component body
or a page block whose name is followed by `:=` declares a variable, the same
test the compiler makes.

## Building

```sh
extensions/tree-sitter-salam/build.sh          # generate, run test/corpus, compile queries/
extensions/tree-sitter-salam/build.sh --sweep  # also parse every Salam file here
```

`TREE_SITTER` points at the CLI if it is not on `PATH`. The generated parser
(`src/parser.c`, about 7 MB) is not committed; `build.sh` creates it.

## Using it

### Neovim

The Salam plugin in `extensions/vim` ships the queries. Build the parser and
register it:

```sh
cd extensions/tree-sitter-salam && tree-sitter generate && tree-sitter build -o salam.so
```

```lua
vim.treesitter.language.Add('salam', { path = '/path/to/salam.so' })
vim.api.nvim_create_autocmd('FileType', {
  pattern = 'salam',
  callback = function(args) vim.treesitter.start(args.buf, 'salam') end,
})
```

### Helix

After `tree-sitter generate`, point a grammar at this directory in
`languages.toml`, then run `hx --grammar build`:

```toml
[[grammar]]
name = "salam"
source = { path = "/path/to/Salam/extensions/tree-sitter-salam" }
```

and copy `queries/` to `runtime/queries/salam/` in your Helix config.

## Limits

- Several layout attributes on one line (`color = tint width = wide`) are
  ambiguous without the attribute schema, because `tint width` could be a
  two-word name; the compiler resolves it, this grammar does not.
- The grammar is for editors. The compiler is the authority on what is valid
  Salam, and `salam lsp` gives exact diagnostics and semantic highlighting.

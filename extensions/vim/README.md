# Salam for Vim and Neovim

Syntax highlighting, indentation, snippets, formatting and diagnostics for the
[Salam programming language](https://github.com/SalamLang/Salam).

One plugin covers both editors. Vim 8.0 and later get the vimscript half;
Neovim additionally loads a Lua layer with asynchronous `vim.diagnostic`
support and `:checkhealth salam`.

Salam keywords are spelled in English or Persian. Both are
highlighted, Arabic-script identifiers count as words for `w`, `*` and `gd`,
and the keyword pack is detected per file.

## Install

vim-plug:

```vim
Plug 'SalamLang/Salam', { 'rtp': 'extensions/vim' }
```

lazy.nvim:

```lua
{
  'SalamLang/Salam',
  config = function()
    vim.opt.rtp:append(vim.fn.stdpath('data') .. '/lazy/Salam/extensions/vim')
    require('salam').setup({})
  end,
}
```

packer.nvim:

```lua
use { 'SalamLang/Salam', rtp = 'extensions/vim' }
```

Manually:

```sh
git clone https://github.com/SalamLang/Salam.git
cp -r Salam/extensions/vim/* ~/.vim/          # Neovim: ~/.config/nvim/
vim -c 'helptags ALL' -c quit
```

The `salam` compiler must be on your `$PATH`, or `g:salam_compiler` must give
its full path.

## Commands

| Command                                        | What it does                                          |
| ---------------------------------------------- | ----------------------------------------------------- |
| `:SalamRun`                                    | Run the buffer (`salam exec`), or the project (`salam run`) |
| `:SalamBuild`                                  | `salam build`, errors in the quickfix list            |
| `:SalamNew [dir]`                              | Run the `salam init` wizard in a terminal             |
| `:SalamPlatform [name]`                        | Choose the platform for `:SalamRun` and `:SalamBuild` |
| `:SalamGotoBlock`                              | Jump to the enclosing block (a count goes further up) |
| `:SalamBlocks`                                 | Pick one of the enclosing blocks to jump to           |
| `:SalamBlockChain`                             | Echo the chain of enclosing blocks                    |
| `:SalamCheck`                                  | Type-check without running, into the quickfix list    |
| `:SalamFmt`                                    | Reformat the buffer, keeping undo and the cursor      |
| `:SalamFmtCheck`                               | Report whether the file is already formatted          |
| `:SalamTokens` / `:SalamAst` / `:SalamSymbols` | Open the compiler's XML dumps                         |
| `:SalamVersion`                                | Echo the compiler version                             |

## Projects and platforms

In a project made by `salam init` (a `salam.toml` above the file), `:SalamRun`
runs `salam run` in a terminal split from the project root, and `:SalamBuild`
runs `salam build`. A web project serves the page with live reload.

`:SalamPlatform` lists the `platforms` of `[project]` in `salam.toml` (every
platform outside a project), plus `auto` and `all`. The choice is remembered
per project and added as `--platform=NAME`:

```vim
:SalamPlatform linux
:SalamPlatform auto      " follow salam.toml again
```

Outside a project `:SalamRun` keeps using `salam exec` until a platform is
chosen, then it runs `salam run FILE --platform=NAME`. `all` and `js` only
apply to builds.

## Block chain

`salam#BlockChain()` returns the headers that enclose the cursor, found by
indentation, for example `page > box > button`. Put it in the statusline:

```vim
autocmd FileType salam setlocal statusline=%f\ %{salam#BlockChain()}\ %=%l:%c
```

Neovim can show it in the winbar instead: `let g:salam_winbar = 1`.
`:SalamGotoBlock` jumps to the enclosing header, `3:SalamGotoBlock` three
levels up; `:SalamBlocks` offers the whole chain.

## Mappings

Buffer-local, on `<LocalLeader>`: `r` run, `b` build, `c` check, `f` format,
`n` new project, `p` platform, `u` enclosing block (with a count), `j` pick a
block. `let g:salam_no_mappings = 1` turns them off.

## Options

```vim
let g:salam_compiler       = 'salam'
let g:salam_lang           = 'auto'    " auto | en | fa
let g:salam_run_command    = 'exec'    " exec | run
let g:salam_format_indent  = '4'       " tab, or 1-16
let g:salam_format_on_save = 0
let g:salam_check_on_save  = 0
let g:salam_stdlib_path    = ''
let g:salam_winbar         = 0     " Neovim: show the block chain
let g:salam_no_mappings    = 0
```

## Neovim

```lua
require('salam').setup({
  compiler = 'salam',
  lang = 'auto',
  indent = '4',
  format_on_save = false,
  diagnostics = true,
  lsp = true,
  messages = 'auto',
})
```

`lsp = true` (the default) starts the Salam language server, `salam lsp`, for
every Salam buffer through Neovim's built-in client: diagnostics as you type,
completion, hover (`K`), go to definition, references, rename, outline,
semantic highlighting and formatting (`vim.lsp.buf.format()`). `messages` picks
the language of its messages, `'en'` or `'fa'`. With a compiler that has no
`salam lsp`, the plugin falls back to the behaviour below.

`diagnostics = true` without the language server type-checks the file after
every write and on read, and publishes the results through `vim.diagnostic`,
so they show up in whatever diagnostic UI you already use. `:checkhealth salam`
reports what was found.

Other editors and the server's options are covered in
[`docs/LSP.md`](../../docs/LSP.md).

## Snippets

`UltiSnips/salam.snippets` is picked up automatically by
[UltiSnips](https://github.com/SirVer/ultisnips). Triggers: `main`, `func`,
`funcv`, `if`, `ifelse`, `until`, `each`, `eachi`, `eachkv`, `repeat`,
`repeati`, `repeatrange`, `var`, `mut`, `const`, `struct`, `enum`,
`interface`, `impl`, `import`, `extern`, `export`, `page`, `component`, `println`.

## ALE

With [ALE](https://github.com/dense-analysis/ale) installed, a `salam` linter
is registered and runs the same check as `:SalamCheck`.

```vim
let g:ale_salam_executable = 'salam'
let g:ale_salam_options = ''
```

## What is generated

`syntax/salam.vim`, `indent/salam.vim`, `ftplugin/salam.vim` and
`UltiSnips/salam.snippets` are generated. Edit the generator, not these files,
and rerun it from the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam
```

The keyword lists come from `compiler/langpack.salam` and
`compiler/semantic/semantic.salam`, so highlighting cannot drift away from
what the compiler accepts.

## Help

`:help salam`

## License

MIT. See [LICENSE](LICENSE).

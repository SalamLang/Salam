# Salam for Vim and Neovim

Syntax highlighting, indentation, snippets, formatting and diagnostics for the
[Salam programming language](https://github.com/SalamLang/Salam).

One plugin covers both editors. Vim 8.0 and later get the vimscript half;
Neovim additionally loads a Lua layer with asynchronous `vim.diagnostic`
support and `:checkhealth salam`.

Salam keywords are spelled in English, Persian or Arabic. All three are
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
| `:SalamRun`                                    | Write and run the buffer (`salam exec`)               |
| `:SalamBuild`                                  | Compile to an executable, errors in the quickfix list |
| `:SalamCheck`                                  | Type-check without running, into the quickfix list    |
| `:SalamFmt`                                    | Reformat the buffer, keeping undo and the cursor      |
| `:SalamFmtCheck`                               | Report whether the file is already formatted          |
| `:SalamTokens` / `:SalamAst` / `:SalamSymbols` | Open the compiler's XML dumps                         |
| `:SalamVersion`                                | Echo the compiler version                             |

## Options

```vim
let g:salam_compiler       = 'salam'
let g:salam_lang           = 'auto'    " auto | en | fa | ar
let g:salam_run_command    = 'exec'    " exec | run
let g:salam_format_indent  = '4'       " tab, or 1-16
let g:salam_format_on_save = 0
let g:salam_check_on_save  = 0
let g:salam_stdlib_path    = ''
```

## Neovim

```lua
require('salam').setup({
  compiler = 'salam',
  lang = 'auto',
  indent = '4',
  format_on_save = false,
  diagnostics = true,
})
```

`diagnostics = true` type-checks the file after every write and on read, and
publishes the results through `vim.diagnostic`, so they show up in whatever
diagnostic UI you already use. `:checkhealth salam` reports what was found.

## Snippets

`UltiSnips/salam.snippets` is picked up automatically by
[UltiSnips](https://github.com/SirVer/ultisnips). Triggers: `main`, `func`,
`funcv`, `if`, `ifelse`, `until`, `each`, `eachi`, `eachkv`, `repeat`,
`repeati`, `repeatrange`, `var`, `mut`, `const`, `struct`, `enum`,
`interface`, `impl`, `import`, `extern`, `layout`, `component`, `println`.

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

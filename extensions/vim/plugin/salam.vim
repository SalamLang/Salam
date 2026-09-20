" Salam support for Vim and Neovim: commands and options.
" Maintainer: The Salam Team <https://github.com/SalamLang/Salam>
" License:    MIT

if exists('g:loaded_salam') || &compatible
  finish
endif
let g:loaded_salam = 1

let s:cpo_save = &cpo
set cpo&vim

" Compiler executable. A bare name is looked up on $PATH.
if !exists('g:salam_compiler')
  let g:salam_compiler = 'salam'
endif

" Keyword pack passed as --lang: 'auto', 'en', 'fa' or 'ar'. 'auto' reads a
" '//! lang:' directive, then falls back to detecting the script in use.
if !exists('g:salam_lang')
  let g:salam_lang = 'auto'
endif

" Which subcommand :SalamRun uses. 'exec' is the tree-walking interpreter and
" needs no C toolchain; 'run' compiles first.
if !exists('g:salam_run_command')
  let g:salam_run_command = 'exec'
endif

" Indentation handed to `salam format --indent`: 'tab', or 1-16 spaces.
if !exists('g:salam_format_indent')
  let g:salam_format_indent = '4'
endif

" Reformat the buffer with `salam format` on every write.
if !exists('g:salam_format_on_save')
  let g:salam_format_on_save = 0
endif

" Root directory for std/... imports, passed as --stdlib-path. Empty lets
" the compiler find it.
if !exists('g:salam_stdlib_path')
  let g:salam_stdlib_path = ''
endif

" Populate the quickfix list from `:SalamCheck` whenever a buffer is written.
if !exists('g:salam_check_on_save')
  let g:salam_check_on_save = 0
endif

command! -nargs=* -complete=file SalamRun     call salam#Run(<q-args>)
command! -nargs=* -complete=file SalamBuild   call salam#Build(<q-args>)
command! -nargs=0                SalamCheck   call salam#Check()
command! -nargs=0 -bar           SalamFmt     call salam#Format()
command! -nargs=0 -bar           SalamFmtCheck call salam#FormatCheck()
command! -nargs=0                SalamTokens  call salam#Inspect('--emit-tokens-xml')
command! -nargs=0                SalamAst     call salam#Inspect('--emit-ast-xml')
command! -nargs=0                SalamSymbols call salam#Inspect('--emit-symbol-xml')
command! -nargs=0 -bar           SalamVersion call salam#Version()

augroup salam_plugin
  autocmd!
  autocmd BufWritePre *.salam if g:salam_format_on_save | call salam#Format() | endif
  autocmd BufWritePost *.salam if g:salam_check_on_save | call salam#Check() | endif
augroup END

let &cpo = s:cpo_save
unlet s:cpo_save

" Vim compiler file
" Compiler:   salam
" Maintainer: The Salam Team <https://github.com/SalamLang/Salam>
"
" --error-style=gcc turns the compiler's Rust-style diagnostics into the
" one-line "file:line:col: error: message" form 'errorformat' reads.

if exists('current_compiler')
  finish
endif
let current_compiler = 'salam'

let s:cpo_save = &cpo
set cpo&vim

if exists(':CompilerSet') != 2
  command -nargs=* CompilerSet setlocal <args>
endif

CompilerSet makeprg=salam\ exec\ --error-style=gcc\ %:S

CompilerSet errorformat=
      \%f:%l:%c:\ %trror:\ %m,
      \%f:%l:%c:\ %tarning:\ %m,
      \%f:%l:%c:\ %tote:\ %m,
      \%f:%l:%c:\ %m,
      \%-G%.%#

let &cpo = s:cpo_save
unlet s:cpo_save

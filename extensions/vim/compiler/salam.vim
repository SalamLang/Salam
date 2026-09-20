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

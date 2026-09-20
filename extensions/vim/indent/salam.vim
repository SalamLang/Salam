" Vim indent file
" Language:   Salam
" GENERATED FILE - do not edit by hand.
" Regenerate with: salam exec extensions/tools/gen_syntaxes.salam

if exists('b:did_indent')
  finish
endif
let b:did_indent = 1

let s:cpo_save = &cpo
set cpo&vim

setlocal indentexpr=GetSalamIndent()
setlocal indentkeys=0{,0},!^F,o,O,e,0=end,<:>
setlocal nolisp
setlocal nosmartindent

let b:undo_indent = 'setlocal indentexpr< indentkeys< lisp< smartindent<'

if exists('*GetSalamIndent')
  let &cpo = s:cpo_save
  unlet s:cpo_save
  finish
endif

" A block opens with a trailing ':' and closes with 'end'; 'else' sits one
" level out and opens the next block itself.
let s:open_block = ':\s*\%(//.*\)\=$'
let s:close_block = '^\s*\%(نهاية\|تمام\|end\)\s*$'
let s:else_block = '^\s*\%(وگرنه\|else\|وإلا\)\%(\s\|:\)'

" Strips a trailing line comment and any string literal, so a ':' or a
" keyword quoted inside one cannot move the indent.
function! s:Code(lnum) abort
  let l:line = getline(a:lnum)
  let l:line = substitute(l:line, '"""\%(.\{-}\)"""', '""', 'g')
  let l:line = substitute(l:line, '"\%(\\.\|[^"\\]\)*"', '""', 'g')
  let l:line = substitute(l:line, "'\\%(\\\\.\\|[^'\\\\]\\)*'", "''", 'g')
  let l:line = substitute(l:line, '//.*$', '', '')
  return substitute(l:line, '\s\+$', '', '')
endfunction

" The closest line above that carries code, skipping blanks and comments.
function! s:PrevCode(lnum) abort
  let l:lnum = prevnonblank(a:lnum - 1)
  while l:lnum > 0 && s:Code(l:lnum) =~# '^\s*$'
    let l:lnum = prevnonblank(l:lnum - 1)
  endwhile
  return l:lnum
endfunction

function! GetSalamIndent() abort
  let l:prev = s:PrevCode(v:lnum)
  if l:prev == 0
    return 0
  endif

  let l:shift = shiftwidth()
  let l:indent = indent(l:prev)
  let l:prevcode = s:Code(l:prev)

  " One level in after a line that opened a block.
  if l:prevcode =~# s:open_block
    let l:indent += l:shift
  endif

  " An 'else' or an 'end' on the previous line already dedented itself, so
  " nothing extra is owed here.
  let l:curcode = s:Code(v:lnum)
  if l:curcode =~# s:close_block || l:curcode =~# s:else_block
    let l:indent -= l:shift
  endif

  return l:indent < 0 ? 0 : l:indent
endfunction

let &cpo = s:cpo_save
unlet s:cpo_save

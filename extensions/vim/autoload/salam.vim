" Salam support for Vim and Neovim: the work behind the commands.
" Maintainer: The Salam Team <https://github.com/SalamLang/Salam>
" License:    MIT

let s:cpo_save = &cpo
set cpo&vim

" Letters Persian uses and Arabic does not. One of them settles the keyword
" pack without needing a directive.
let s:persian_only = '[پچژگکی]'
let s:arabic_script = '[؀-ۿݐ-ݿࢠ-ࣿﭐ-﷿ﹰ-﻿]'

" Read an option without depending on plugin/salam.vim having run first.
function! s:Opt(name, default) abort
  return get(g:, a:name, a:default)
endfunction

function! s:Error(message) abort
  echohl ErrorMsg
  echomsg 'salam: ' . a:message
  echohl None
endfunction

function! s:NullDevice() abort
  return has('win32') || has('win64') ? 'NUL' : '/dev/null'
endfunction

" Which keyword pack the current buffer is written in.
function! salam#Lang() abort
  if index(['en', 'fa', 'ar'], s:Opt('salam_lang', 'auto')) >= 0
    return s:Opt('salam_lang', 'auto')
  endif

  let l:head = join(getline(1, min([line('$'), 200])), "\n")

  let l:directive = matchlist(l:head, '\%(lang\|LANG\|زبان\)\s*:\s*\([A-Za-z][A-Za-z]\)')
  if !empty(l:directive)
    return tolower(l:directive[1])
  endif

  if l:head =~# s:persian_only
    return 'fa'
  endif
  if strchars(substitute(l:head, '\%(' . s:arabic_script . '\)\@!.', '', 'g')) >= 3
    return 'ar'
  endif
  return 'en'
endfunction

" The options every invocation carries.
function! s:CommonArgs() abort
  let l:args = ['--lang=' . salam#Lang()]
  let l:stdlib = s:Opt('salam_stdlib_path', '')
  if !empty(l:stdlib)
    call add(l:args, '--stdlib-path=' . l:stdlib)
  endif
  return l:args
endfunction

function! s:Shell(args) abort
  return join(map([s:Opt('salam_compiler', 'salam')] + a:args, 'shellescape(v:val)'), ' ')
endfunction

" Runs the compiler and returns its combined output. v:shell_error holds the
" exit status afterwards.
function! s:Capture(args) abort
  return system(s:Shell(a:args))
endfunction

function! s:CurrentFile() abort
  let l:path = expand('%:p')
  if empty(l:path)
    call s:Error('the buffer has no file name; save it first')
    return ''
  endif
  return l:path
endfunction

" --- running -------------------------------------------------------------

function! salam#Run(extra) abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif

  let l:args = [s:Opt('salam_run_command', 'exec'), l:path, '--error-style=gcc'] + s:CommonArgs()
  if !empty(a:extra)
    let l:args += split(a:extra)
  endif

  " :! keeps the program's own stdin and stdout, which matters for a
  " language whose 'input' statement reads from the terminal.
  execute '!' . s:Shell(l:args)
endfunction

function! salam#Build(extra) abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif

  let l:args = ['build', l:path, '--error-style=gcc'] + s:CommonArgs()
  if !empty(a:extra)
    let l:args += split(a:extra)
  endif

  let l:saved = [&l:makeprg, &l:errorformat]
  try
    let &l:makeprg = s:Shell(l:args)
    compiler salam
    let &l:makeprg = s:Shell(l:args)
    silent make!
    redraw!
    if empty(getqflist())
      echo 'salam: build succeeded'
    else
      copen
    endif
  finally
    let [&l:makeprg, &l:errorformat] = l:saved
  endtry
endfunction

" --- checking ------------------------------------------------------------

" Lex, parse and analyze without running or emitting anything, then put the
" diagnostics in the quickfix list.
function! salam#Check() abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif

  let l:args = [
        \ 'inspect', l:path,
        \ '--emit-symbol-xml',
        \ '--xml-out=' . s:NullDevice(),
        \ '--error-style=gcc',
        \ '--log-level=error',
        \ ] + s:CommonArgs()

  let l:output = s:Capture(l:args)
  let l:saved = &errorformat
  try
    let &errorformat = '%f:%l:%c: %trror: %m,%f:%l:%c: %tarning: %m,%f:%l:%c: %tote: %m,%-G%.%#'
    cgetexpr l:output
  finally
    let &errorformat = l:saved
  endtry

  if empty(getqflist())
    cclose
    echo 'salam: no problems found'
  else
    copen
    cfirst
  endif
endfunction

" --- formatting ----------------------------------------------------------

" `salam format` rewrites files in place, so the buffer goes through a
" scratch file and comes back as a line replacement, which keeps undo and
" the cursor position intact.
function! salam#Format() abort
  let l:tmp = tempname() . '.salam'
  let l:view = winsaveview()

  try
    call writefile(getline(1, '$'), l:tmp)
    let l:args = [
          \ 'format', l:tmp,
          \ '--indent=' . s:Opt('salam_format_indent', '4'),
          \ '--error-style=gcc',
          \ ] + s:CommonArgs()
    let l:output = s:Capture(l:args)

    if v:shell_error != 0
      call s:Error('formatting failed')
      echo l:output
      return
    endif

    let l:formatted = readfile(l:tmp)
    if l:formatted ==# getline(1, '$')
      return
    endif

    " Replace only what changed so a no-op format leaves undo alone.
    call setline(1, l:formatted)
    if line('$') > len(l:formatted)
      silent execute (len(l:formatted) + 1) . ',$delete _'
    endif
  finally
    call delete(l:tmp)
    call winrestview(l:view)
  endtry
endfunction

function! salam#FormatCheck() abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  let l:output = s:Capture(['format', '--check', l:path, '--error-style=gcc'] + s:CommonArgs())
  if v:shell_error == 0
    echo 'salam: already formatted'
  else
    call s:Error('needs formatting')
    echo l:output
  endif
endfunction

" --- inspecting ----------------------------------------------------------

" Opens the token stream, AST or symbol table in a scratch window.
function! salam#Inspect(flag) abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif

  let l:output = s:Capture(['inspect', l:path, a:flag, '--error-style=gcc'] + s:CommonArgs())

  new
  setlocal buftype=nofile bufhidden=wipe noswapfile nobuflisted
  setlocal filetype=xml
  silent put =split(l:output, "\n")
  silent 1delete _
  normal! gg
endfunction

function! salam#Version() abort
  echo substitute(s:Capture(['version']), '\n\+$', '', '')
endfunction

let &cpo = s:cpo_save
unlet s:cpo_save

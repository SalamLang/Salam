call ale#Set('salam_executable', 'salam')
call ale#Set('salam_options', '')

function! ale_linters#salam#salam#GetCommand(buffer) abort
  let l:null = has('win32') ? 'NUL' : '/dev/null'
  return '%e inspect %s --emit-symbol --xml-out=' . l:null
        \ . ' --error-style=gcc --log-level=error '
        \ . ale#Var(a:buffer, 'salam_options')
endfunction

function! ale_linters#salam#salam#Handle(buffer, lines) abort
  let l:pattern = '^\(.\+\):\(\d\+\):\(\d\+\): \(error\|warning\|note\): \(.*\)$'
  let l:output = []

  for l:match in ale#util#GetMatches(a:lines, l:pattern)
    call add(l:output, {
          \ 'lnum': str2nr(l:match[2]),
          \ 'col': str2nr(l:match[3]),
          \ 'type': l:match[4] ==# 'error' ? 'E' : 'W',
          \ 'text': l:match[5],
          \})
  endfor

  return l:output
endfunction

call ale#linter#Define('salam', {
      \ 'name': 'salam',
      \ 'executable': {b -> ale#Var(b, 'salam_executable')},
      \ 'command': function('ale_linters#salam#salam#GetCommand'),
      \ 'callback': 'ale_linters#salam#salam#Handle',
      \ 'lint_file': 1,
      \})

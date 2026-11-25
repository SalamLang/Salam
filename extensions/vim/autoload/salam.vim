let s:cpo_save = &cpo
set cpo&vim

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

function! salam#Lang() abort
  let l:lang = s:Opt('salam_lang', 'auto')
  return index(['en', 'fa'], l:lang) >= 0 ? l:lang : ''
endfunction

function! s:CommonArgs() abort
  let l:args = []
  if !empty(salam#Lang())
    call add(l:args, '--lang=' . salam#Lang())
  endif
  let l:stdlib = s:Opt('salam_stdlib_path', '')
  if !empty(l:stdlib)
    call add(l:args, '--stdlib-path=' . l:stdlib)
  endif
  return l:args
endfunction

function! s:SwitchLintArgs() abort
  let l:mode = s:Opt('salam_switch_lint', 'off')
  if index(['warn', 'error', 'fix'], l:mode) < 0
    return []
  endif
  let l:args = ['--switch-lint=' . l:mode]
  let l:minimum = s:Opt('salam_switch_lint_min', '4')
  return l:args + ['--switch-lint-min=' . string(l:minimum)]
endfunction

function! s:Shell(args) abort
  return join(map([s:Opt('salam_compiler', 'salam')] + a:args, 'shellescape(v:val)'), ' ')
endfunction

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

function! s:Cwd() abort
  return expand('%:p:h')
endfunction

function! s:StateFile() abort
  if has('nvim')
    let l:dir = stdpath('data')
  elseif has('win32') || has('win64')
    let l:dir = empty($LOCALAPPDATA) ? expand('~') : $LOCALAPPDATA
  else
    let l:dir = empty($XDG_DATA_HOME) ? expand('~/.local/share') : $XDG_DATA_HOME
  endif
  let l:dir .= '/salam'
  if !isdirectory(l:dir)
    call mkdir(l:dir, 'p')
  endif
  return l:dir . '/platforms.json'
endfunction

function! s:LoadState() abort
  if !exists('s:state')
    let s:state = {}
    try
      let l:text = join(readfile(s:StateFile()), "\n")
      let l:data = json_decode(l:text)
      if type(l:data) == type({})
        let s:state = l:data
      endif
    catch
    endtry
  endif
  return s:state
endfunction

function! s:SaveState() abort
  try
    call writefile([json_encode(s:LoadState())], s:StateFile())
  catch
  endtry
endfunction

function! salam#ProjectRoot() abort
  let l:dir = expand('%:p:h')
  if empty(l:dir)
    let l:dir = getcwd()
  endif
  let l:found = findfile('salam.toml', fnameescape(l:dir) . ';')
  if empty(l:found)
    return ''
  endif
  return fnamemodify(l:found, ':p:h')
endfunction

function! salam#ProjectPlatforms(root) abort
  if empty(a:root)
    return []
  endif
  try
    let l:lines = readfile(a:root . '/salam.toml')
  catch
    return []
  endtry
  let l:in_project = 0
  let l:collect = 0
  let l:text = ''
  for l:line in l:lines
    if !l:collect
      if l:line =~# '^\s*\['
        let l:in_project = l:line =~# '^\s*\[project\]'
        continue
      endif
      if !l:in_project || l:line !~# '^\s*platforms\s*='
        continue
      endif
      let l:collect = 1
      let l:line = substitute(l:line, '^[^=]*=', '', '')
    endif
    let l:text .= l:line
    if l:text =~# '\]'
      break
    endif
  endfor
  let l:out = []
  let l:at = 0
  while 1
    let l:m = matchstrpos(l:text, '"\zs[^"]*\ze"', l:at)
    if l:m[1] < 0
      break
    endif
    call add(l:out, l:m[0])
    let l:at = l:m[2] + 1
  endwhile
  return l:out
endfunction

let s:all_platforms = ['web', 'js', 'android', 'ios', 'linux', 'macos', 'windows', 'all']

function! salam#PlatformChoices() abort
  let l:listed = salam#ProjectPlatforms(salam#ProjectRoot())
  if empty(l:listed)
    return copy(s:all_platforms)
  endif
  return l:listed + (len(l:listed) > 1 ? ['all'] : [])
endfunction

function! salam#Platform() abort
  let l:key = salam#ProjectRoot()
  if empty(l:key)
    let l:key = '*'
  endif
  return get(s:LoadState(), l:key, '')
endfunction

function! salam#SetPlatform(name) abort
  let l:key = salam#ProjectRoot()
  if empty(l:key)
    let l:key = '*'
  endif
  let l:state = s:LoadState()
  if empty(a:name) || a:name ==# 'auto'
    silent! call remove(l:state, l:key)
    echo 'salam: platform follows salam.toml'
  elseif index(s:all_platforms, a:name) < 0
    call s:Error('unknown platform ' . a:name . ' (' . join(s:all_platforms, ', ') . ')')
    return
  else
    let l:state[l:key] = a:name
    echo 'salam: platform ' . a:name
  endif
  call s:SaveState()
endfunction

function! salam#CompletePlatform(lead, line, pos) abort
  return filter(['auto'] + salam#PlatformChoices(), 'v:val =~# "^" . a:lead')
endfunction

function! salam#PickPlatform(name) abort
  if !empty(a:name)
    call salam#SetPlatform(a:name)
    return
  endif
  let l:choices = ['auto'] + salam#PlatformChoices()
  let l:current = salam#Platform()
  let l:menu = ['Salam platform (now ' . (empty(l:current) ? 'auto' : l:current) . '):']
  let l:i = 1
  for l:name in l:choices
    call add(l:menu, l:i . '. ' . l:name)
    let l:i += 1
  endfor
  let l:pick = inputlist(l:menu)
  if l:pick >= 1 && l:pick <= len(l:choices)
    call salam#SetPlatform(l:choices[l:pick - 1])
  endif
endfunction

function! s:PlatformArg(verb) abort
  let l:name = salam#Platform()
  if empty(l:name) || (a:verb ==# 'run' && (l:name ==# 'all' || l:name ==# 'js'))
    return []
  endif
  return ['--platform=' . l:name]
endfunction

function! s:RunArgs(path, extra) abort
  let l:root = salam#ProjectRoot()
  let l:platform = s:PlatformArg('run')
  if !empty(l:root)
    let l:args = ['run'] + l:platform
    let l:cwd = l:root
  elseif !empty(l:platform)
    let l:args = ['run', a:path] + l:platform
    let l:cwd = s:Cwd()
  else
    let l:args = [s:Opt('salam_run_command', 'exec'), a:path]
    let l:cwd = s:Cwd()
  endif
  let l:args += ['--error-style=gcc'] + s:CommonArgs()
  if !empty(a:extra)
    let l:args += split(a:extra)
  endif
  return [l:args, l:cwd]
endfunction

function! s:BuildArgs(path, extra) abort
  let l:root = salam#ProjectRoot()
  let l:args = ['build']
  let l:cwd = s:Cwd()
  if !empty(l:root)
    let l:cwd = l:root
  else
    call add(l:args, a:path)
  endif
  let l:args += s:PlatformArg('build') + ['--error-style=gcc'] + s:CommonArgs()
  if !empty(a:extra)
    let l:args += split(a:extra)
  endif
  return [l:args, l:cwd]
endfunction

function! s:InTerminal(args, cwd) abort
  let l:cmd = 'cd ' . shellescape(a:cwd) . ' && ' . s:Shell(a:args)
  if has('win32') || has('win64')
    let l:cmd = 'cd /d ' . shellescape(a:cwd) . ' && ' . s:Shell(a:args)
  endif
  if has('nvim')
    execute 'botright 15split | terminal ' . l:cmd
    startinsert
  elseif has('terminal')
    call term_start(&shell . ' ' . &shellcmdflag . ' ' . shellescape(l:cmd), {'term_rows': 15})
  else
    execute '!' . l:cmd
  endif
endfunction

function! salam#Run(extra) abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif
  let [l:args, l:cwd] = s:RunArgs(l:path, a:extra)
  if l:args[0] ==# 'run' && !empty(salam#ProjectRoot())
    call s:InTerminal(l:args, l:cwd)
    return
  endif
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

  let [l:args, l:cwd] = s:BuildArgs(l:path, a:extra)

  let l:saved = [&l:makeprg, &l:errorformat]
  let l:dir = getcwd()
  try
    compiler salam
    let &l:makeprg = s:Shell(l:args)
    execute 'lcd ' . fnameescape(l:cwd)
    silent make!
    redraw!
    if empty(getqflist())
      echo 'salam: build succeeded'
    else
      copen
    endif
  finally
    execute 'lcd ' . fnameescape(l:dir)
    let [&l:makeprg, &l:errorformat] = l:saved
  endtry
endfunction

function! salam#New(...) abort
  let l:dir = a:0 && !empty(a:1) ? a:1 : ''
  let l:args = ['init'] + (empty(l:dir) ? [] : [l:dir])
  call s:InTerminal(l:args, getcwd())
endfunction

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
        \ '--emit-symbol',
        \ '--xml-out=' . s:NullDevice(),
        \ '--error-style=gcc',
        \ '--log-level=error',
        \ ] + s:CommonArgs() + s:SwitchLintArgs()

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

function! salam#Inspect(flag) abort
  let l:path = s:CurrentFile()
  if empty(l:path)
    return
  endif
  if &modified
    write
  endif

  let l:output = s:Capture(['inspect', l:path, a:flag, '--error-style=gcc', '--log-level=error'] + s:CommonArgs())
  let l:lines = split(l:output, "\n")

  new
  setlocal buftype=nofile bufhidden=wipe noswapfile nobuflisted
  call setline(1, l:lines)
  setlocal filetype=xml
  setlocal nomodified
  normal! gg
endfunction

function! salam#Version() abort
  echo substitute(s:Capture(['version']), '\n\+$', '', '')
endfunction


function! s:IsEnd(text) abort
  return a:text =~# '^\s*\%(end\|پایان\)\s*\%(//.*\)\=$'
endfunction

function! s:IsBlank(text) abort
  return a:text =~# '^\s*$' || a:text =~# '^\s*\%(//\|/\*\|\*\)'
endfunction

function! s:HeaderLabel(text) abort
  let l:t = substitute(a:text, '^\s\+', '', '')
  if l:t =~# ':\s*$'
    let l:t = substitute(l:t, '\s*:\s*$', '', '')
  elseif l:t =~# ':\s*//'
    let l:t = substitute(l:t, '\s*:\s*//.*$', '', '')
  else
    return ''
  endif
  let l:cut = match(l:t, '(')
  if l:cut > 0
    let l:t = substitute(strpart(l:t, 0, l:cut), '\s\+$', '', '')
  endif
  if strchars(l:t) > 32
    let l:t = strcharpart(l:t, 0, 31) . '...'
  endif
  return l:t
endfunction

function! salam#Blocks(...) abort
  let l:lnum = a:0 && a:1 > 0 ? a:1 : line('.')
  let l:text = getline(l:lnum)
  let l:limit = -1
  if s:IsBlank(l:text)
    let l:p = prevnonblank(l:lnum)
    while l:p > 0 && s:IsBlank(getline(l:p))
      let l:p = prevnonblank(l:p - 1)
    endwhile
    if l:p == 0
      return []
    endif
    let l:lnum = l:p
    let l:text = getline(l:lnum)
    if s:IsEnd(l:text)
      let l:limit = indent(l:lnum)
    endif
  elseif s:IsEnd(l:text)
    let l:limit = indent(l:lnum) + 1
  endif

  let l:chain = []
  if l:limit < 0
    let l:label = s:HeaderLabel(l:text)
    let l:limit = indent(l:lnum) + 1
    if !empty(l:label)
      call add(l:chain, [l:lnum, l:label])
      let l:limit = indent(l:lnum)
    endif
  endif
  let l:n = l:lnum - 1
  while l:n >= 1 && l:limit > 0
    let l:line = getline(l:n)
    if !s:IsBlank(l:line) && indent(l:n) < l:limit
      let l:label = s:HeaderLabel(l:line)
      if !empty(l:label)
        call insert(l:chain, [l:n, l:label])
        let l:limit = indent(l:n)
      endif
    endif
    let l:n -= 1
  endwhile
  return l:chain
endfunction

function! salam#BlockChain(...) abort
  if exists('g:statusline_winid') && g:statusline_winid != win_getid()
    return ''
  endif
  let l:lnum = a:0 ? a:1 : line('.')
  return join(map(salam#Blocks(l:lnum), 'v:val[1]'), ' > ')
endfunction

function! salam#GotoBlock(count) abort
  let l:cur = line('.')
  let l:blocks = filter(salam#Blocks(l:cur), 'v:val[0] != ' . l:cur)
  let l:steps = a:count > 0 ? a:count : 1
  if empty(l:blocks)
    echo 'salam: not inside a block'
    return
  endif
  let l:target = l:blocks[max([len(l:blocks) - l:steps, 0])]
  normal! m'
  call cursor(l:target[0], 1)
  normal! ^
endfunction

function! salam#PickBlock() abort
  let l:blocks = salam#Blocks()
  if empty(l:blocks)
    echo 'salam: not inside a block'
    return
  endif
  let l:menu = ['Jump to block:']
  let l:i = 1
  for l:b in l:blocks
    call add(l:menu, l:i . '. ' . l:b[1] . '  (line ' . l:b[0] . ')')
    let l:i += 1
  endfor
  let l:pick = inputlist(l:menu)
  if l:pick >= 1 && l:pick <= len(l:blocks)
    normal! m'
    call cursor(l:blocks[l:pick - 1][0], 1)
    normal! ^
  endif
endfunction
function! salam#Setup() abort
  if !get(g:, 'salam_no_mappings', 0)
    nnoremap <buffer> <silent> <LocalLeader>r :SalamRun<CR>
    nnoremap <buffer> <silent> <LocalLeader>b :SalamBuild<CR>
    nnoremap <buffer> <silent> <LocalLeader>c :SalamCheck<CR>
    nnoremap <buffer> <silent> <LocalLeader>f :SalamFmt<CR>
    nnoremap <buffer> <silent> <LocalLeader>n :SalamNew<CR>
    nnoremap <buffer> <silent> <LocalLeader>p :SalamPlatform<CR>
    nnoremap <buffer> <silent> <LocalLeader>u :<C-u>execute 'SalamGotoBlock ' . v:count1<CR>
    nnoremap <buffer> <silent> <LocalLeader>j :SalamBlocks<CR>
  endif
  if has('nvim-0.8') && get(g:, 'salam_winbar', 0)
    setlocal winbar=%{salam#BlockChain()}
  endif
endfunction

let &cpo = s:cpo_save
unlet s:cpo_save

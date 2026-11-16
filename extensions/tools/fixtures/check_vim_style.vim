edit extensions/tools/fixtures/style_values.salam
let want = [
  \ [3, 17, 'salamPercent'],
  \ [4, 28, 'salamColor'],
  \ [7, 18, 'salamDimension'],
  \ [8, 22, 'salamPercent'],
  \ [9, 15, 'salamDimension'],
  \ [12, 8, 'salamOperator']]
let bad = []
for [l, c, g] in want
  let got = synIDattr(synID(l, c, 1), 'name')
  if got !=# g
    call add(bad, l . ':' . c . ' want ' . g . ' got ' . got)
  endif
endfor
call writefile(bad, '/tmp/vim-style-bad')
qa!

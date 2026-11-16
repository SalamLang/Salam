edit extensions/tools/fixtures/style_values.salam
let want = [
  \ [3, 9, 'salamStyleName'], [3, 17, 'salamPercent'],
  \ [4, 9, 'salamStyleName'], [4, 28, 'salamColor'],
  \ [5, 9, 'salamStyleName'], [5, 25, 'salamStyleValue'],
  \ [6, 9, 'salamStyleName'],
  \ [7, 18, 'salamDimension'],
  \ [8, 9, 'salamStyleName'], [8, 22, 'salamPercent'],
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

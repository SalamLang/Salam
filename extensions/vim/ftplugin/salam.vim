" Vim filetype plugin
" Language:   Salam
" Maintainer: The Salam Team <https://github.com/SalamLang/Salam>
" License:    MIT
"
" GENERATED FILE - do not edit by hand.
" Regenerate with: salam exec extensions/tools/gen_syntaxes.salam

if exists('b:did_ftplugin')
  finish
endif
let b:did_ftplugin = 1

let s:cpo_save = &cpo
set cpo&vim

" Identifiers may be written in Arabic script, so w, b, *, # and gd have to
" treat those letters, the Arabic-Indic digits and ZWNJ/ZWJ as word
" characters. The syntax file sets the same list for its keyword matching.
setlocal iskeyword=@,48-57,_,192-255,1536-1791,1872-1919,2208-2303,64336-65023,65136-65279,8204,8205

setlocal commentstring=//\ %s
setlocal comments=s1:/*,mb:*,ex:*/,://
setlocal suffixesadd=.salam
setlocal includeexpr=substitute(v:fname,'\\.','/','g')
setlocal formatoptions-=t
setlocal formatoptions+=croqlj

" The compiler's own formatter uses four spaces per level.
setlocal expandtab
setlocal shiftwidth=4
setlocal softtabstop=4
setlocal tabstop=4

compiler salam

" % jumps between the keyword that opened a block and the 'end' that closes
" it, in whichever of the three languages the file is written in.
if exists('loaded_matchit') && !exists('b:match_words')
  let b:match_ignorecase = 0
  let b:match_words = '\<\%(پیاده‌سازی\|interface\|component\|struct\|layout\|extern\|repeat\|تاوقتی\|ساختار\|until\|match\|while\|شمارش\|خارجی\|مولفه\|تکرار\|تطبیق\|بينما\|تعداد\|خارجي\|واجهة\|تنفيذ\|func\|type\|enum\|impl\|each\|تابع\|صفحه\|واسط\|دالة\|بنية\|صفحة\|مكون\|طابق\|اگر\|نوع\|إذا\|على\|كرر\|if\|on\|بر\|هر\|كل\)\>:\<\%(وگرنه\|else\|وإلا\)\>:\<\%(نهاية\|تمام\|end\)\>'
endif

let b:undo_ftplugin = 'setlocal iskeyword< commentstring< comments< suffixesadd<'
      \ . ' includeexpr< formatoptions< expandtab< shiftwidth< softtabstop< tabstop<'
      \ . ' | unlet! b:match_words b:match_ignorecase'

let &cpo = s:cpo_save
unlet s:cpo_save

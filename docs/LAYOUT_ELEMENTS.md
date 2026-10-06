# Layout elements

Every element the layout DSL knows, generated from `std/layout/elements/`. The HTML tag name is accepted as an English name too, except for the table parts that come from context: inside `table`, write `header`/`main`/`footer` and `column`, never `thead`/`tbody`/`tfoot`/`th`/`td`. Elements that share a name are context variants: the compiler picks one by where it sits (see `docs/LAYOUT_SCHEMA.md`).

| element              | English names                         | Persian          | HTML           | kind    | must be inside                                        | required     |
| -------------------- | ------------------------------------- | ---------------- | -------------- | ------- | ----------------------------------------------------- | ------------ |
| `abbreviation`       | `abbreviation`, `abbr`                | کوته‌نوشت        | `<abbr>`       | normal  |                                                       |              |
| `address`            | `address`                             | آدرس             | `<address>`    | normal  |                                                       |              |
| `area`               | `area`                                | ناحیه            | `<area>`       | void    | anywhere inside `image map`                           |              |
| `article`            | `article`                             | مقاله            | `<article>`    | normal  |                                                       |              |
| `aside`              | `aside`, `sidebar`                    | کناری            | `<aside>`      | normal  |                                                       |              |
| `audio`              | `audio`                               | صدا              | `<audio>`      | normal  |                                                       |              |
| `base`               | `base`                                | پایه نشانی       | `<base>`       | void    | `layout`                                              |              |
| `block quote`        | `block quote`, `blockquote`           | نقل قول بلند     | `<blockquote>` | normal  |                                                       |              |
| `bold`               | `bold`, `b`                           | پررنگ            | `<b>`          | normal  |                                                       |              |
| `box`                | `box`, `div`                          | جعبه             | `<div>`        | normal  |                                                       |              |
| `break`              | `break`, `br`                         | شکست             | `<br>`         | void    |                                                       |              |
| `button`             | `button`                              | دکمه             | `<button>`     | normal  |                                                       |              |
| `canvas`             | `canvas`                              | بوم              | `<canvas>`     | normal  |                                                       |              |
| `caption`            | `caption`                             | عنوان جدول       | `<caption>`    | normal  | `table`                                               |              |
| `cite`               | `cite`                                | نام اثر          | `<cite>`       | normal  |                                                       |              |
| `code`               | `code`                                | کد               | `<code>`       | normal  |                                                       |              |
| `column`             | `column`, `cell`                      | ستون, سلول       | `<td>`         | normal  | `row`                                                 |              |
| `column definition`  | `column definition`, `col`            | تعریف ستون       | `<col>`        | void    | `column group`                                        |              |
| `column group`       | `column group`, `colgroup`            | گروه ستون        | `<colgroup>`   | normal  | `table`                                               |              |
| `data`               | `data`                                | داده             | `<data>`       | normal  |                                                       | `value`      |
| `data list`          | `data list`, `datalist`               | فهرست پیشنهاد    | `<datalist>`   | normal  |                                                       |              |
| `definition`         | `definition`, `dfn`                   | تعریف            | `<dfn>`        | normal  |                                                       |              |
| `deleted`            | `deleted`, `del`                      | حذف شده          | `<del>`        | normal  |                                                       |              |
| `description`        | `description`, `dd`                   | شرح              | `<dd>`         | normal  | `description list`                                    |              |
| `description in box` | `description`, `dd`                   | شرح              | `<dd>`         | normal  | `description list` > `box`                            |              |
| `description list`   | `description list`, `dl`              | فهرست تعریف      | `<dl>`         | normal  |                                                       |              |
| `details`            | `details`                             | جزئیات           | `<details>`    | normal  |                                                       |              |
| `dialog`             | `dialog`                              | پنجره گفتگو      | `<dialog>`     | normal  |                                                       |              |
| `direction override` | `direction override`, `bdo`           | جهت اجباری       | `<bdo>`        | normal  |                                                       | `dir`        |
| `embed`              | `embed`                               | جاسازی           | `<embed>`      | void    |                                                       | `url`        |
| `emphasis`           | `emphasis`, `em`                      | تاکید            | `<em>`         | normal  |                                                       |              |
| `field set`          | `field set`, `fieldset`               | گروه فیلد        | `<fieldset>`   | normal  |                                                       |              |
| `figure`             | `figure`                              | شکل              | `<figure>`     | normal  |                                                       |              |
| `figure caption`     | `figure caption`, `figcaption`        | زیرنویس شکل      | `<figcaption>` | normal  | `figure`                                              |              |
| `font`               | `font`                                | قلم              | `<span>`       | normal  |                                                       |              |
| `footer`             | `footer`                              | پاورقی           | `<footer>`     | normal  |                                                       |              |
| `form`               | `form`                                | فرم              | `<form>`       | normal  |                                                       |              |
| `global`             | `global`, `global style`              | سراسری           | (none)         | virtual |                                                       |              |
| `h1`                 | `h1`                                  |                  | `<h1>`         | normal  |                                                       |              |
| `h2`                 | `h2`                                  |                  | `<h2>`         | normal  |                                                       |              |
| `h3`                 | `h3`                                  |                  | `<h3>`         | normal  |                                                       |              |
| `h4`                 | `h4`                                  |                  | `<h4>`         | normal  |                                                       |              |
| `h5`                 | `h5`                                  |                  | `<h5>`         | normal  |                                                       |              |
| `h6`                 | `h6`                                  |                  | `<h6>`         | normal  |                                                       |              |
| `head link`          | `head link`, `head_link`              | پیوند سر         | `<link>`       | void    | `layout`                                              | `rel`, `url` |
| `header`             | `header`                              | سرصفحه           | `<header>`     | normal  |                                                       |              |
| `header column`      | `column`, `cell`                      | ستون, سلول       | `<th>`         | normal  | `table header` > `row`                                |              |
| `heading`            | `heading`                             | سرتیتر           | `<h1>`         | normal  |                                                       |              |
| `heading column`     | `column`, `cell`                      | ستون, سلول       | `<th>`         | normal  | `row`                                                 |              |
| `heading group`      | `heading group`, `hgroup`             | گروه سرتیتر      | `<hgroup>`     | normal  |                                                       |              |
| `iframe`             | `iframe`                              | قاب درونی        | `<iframe>`     | normal  |                                                       |              |
| `image`              | `image`, `img`                        | تصویر            | `<img>`        | void    |                                                       | `url`        |
| `image map`          | `image map`, `map`                    | نقشه تصویر       | `<map>`        | normal  |                                                       | `name`       |
| `input`              | `input`                               | ورودی            | `<input>`      | void    |                                                       |              |
| `inserted`           | `inserted`, `ins`                     | افزوده           | `<ins>`        | normal  |                                                       |              |
| `isolate`            | `isolate`, `bdi`                      | جداساز جهت       | `<bdi>`        | normal  |                                                       |              |
| `italic`             | `italic`, `i`                         | کج               | `<i>`          | normal  |                                                       |              |
| `item`               | `item`, `li`                          | مورد             | `<li>`         | normal  | `list`, `ordered list`, `menu`                        |              |
| `keyboard`           | `keyboard`, `kbd`                     | صفحه کلید        | `<kbd>`        | normal  |                                                       |              |
| `label`              | `label`                               | برچسب            | `<label>`      | normal  |                                                       |              |
| `layout`             | `layout`                              | صفحه             | (none)         | virtual |                                                       |              |
| `legend`             | `legend`                              | عنوان گروه       | `<legend>`     | normal  | `field set`                                           |              |
| `line`               | `line`, `hr`                          | خط               | `<hr>`         | void    |                                                       |              |
| `link`               | `link`, `a`                           | پیوند            | `<a>`          | normal  |                                                       | `url`        |
| `list`               | `list`, `ul`                          | فهرست            | `<ul>`         | normal  |                                                       |              |
| `main`               | `main`                                | اصلی             | `<main>`       | normal  |                                                       |              |
| `mark`               | `mark`, `highlight`                   | برجسته           | `<mark>`       | normal  |                                                       |              |
| `math`               | `math`                                | ریاضی            | `<math>`       | rawtext |                                                       |              |
| `media`              | `media`                               | رسانه            | (none)         | virtual |                                                       |              |
| `menu`               | `menu`                                | منو              | `<menu>`       | normal  |                                                       |              |
| `meta`               | `meta`                                | متا              | `<meta>`       | void    | `layout`                                              |              |
| `meter`              | `meter`, `gauge`                      | سنجه             | `<meter>`      | normal  |                                                       | `value`      |
| `nav`                | `nav`                                 | ناوبری           | `<nav>`        | normal  |                                                       |              |
| `noscript`           | `noscript`, `no script`               | بدون اسکریپت     | `<noscript>`   | normal  |                                                       |              |
| `object`             | `object`                              | شیء              | `<object>`     | normal  |                                                       | `url`        |
| `option`             | `option`                              | گزینه            | `<option>`     | normal  | `select`, `option group`, `data list`                 |              |
| `option group`       | `option group`, `optgroup`            | گروه گزینه       | `<optgroup>`   | normal  | `select`                                              | `label`      |
| `ordered list`       | `ordered list`, `numbered list`, `ol` | فهرست شماره‌دار  | `<ol>`         | normal  |                                                       |              |
| `output`             | `output`                              | خروجی            | `<output>`     | normal  |                                                       |              |
| `paragraph`          | `paragraph`, `p`                      | پاراگراف         | `<p>`          | normal  |                                                       |              |
| `picture`            | `picture`                             | عکس              | `<picture>`    | normal  |                                                       |              |
| `preformatted`       | `preformatted`, `pre`                 | پیش‌قالب         | `<pre>`        | normal  |                                                       |              |
| `progress`           | `progress`                            | پیشرفت           | `<progress>`   | normal  |                                                       |              |
| `quote`              | `quote`, `q`                          | نقل قول          | `<q>`          | normal  |                                                       |              |
| `row`                | `row`, `tr`                           | ردیف             | `<tr>`         | normal  | `table`, `table header`, `table body`, `table footer` |              |
| `ruby`               | `ruby`                                | روبی             | `<ruby>`       | normal  |                                                       |              |
| `ruby parenthesis`   | `ruby parenthesis`, `rp`              | پرانتز روبی      | `<rp>`         | normal  | `ruby`                                                |              |
| `ruby text`          | `ruby text`, `rt`                     | متن روبی         | `<rt>`         | normal  | `ruby`                                                |              |
| `sample`             | `sample`, `samp`                      | نمونه خروجی      | `<samp>`       | normal  |                                                       |              |
| `script`             | `script`                              | اسکریپت سفارشی   | `<script>`     | rawtext |                                                       |              |
| `search`             | `search`                              | جستجو            | `<search>`     | normal  |                                                       |              |
| `section`            | `section`                             | بخش              | `<section>`    | normal  |                                                       |              |
| `select`             | `select`, `dropdown`                  | انتخاب           | `<select>`     | normal  |                                                       |              |
| `slot`               | `slot`                                | جایگاه           | `<slot>`       | normal  |                                                       |              |
| `small`              | `small`                               | ریز              | `<small>`      | normal  |                                                       |              |
| `source`             | `source`                              | منبع             | `<source>`     | void    | `picture`, `video`, `audio`                           |              |
| `span`               | `span`                                | اسپن             | `<span>`       | normal  |                                                       |              |
| `strikethrough`      | `strikethrough`, `s`                  | خط خورده         | `<s>`          | normal  |                                                       |              |
| `strong`             | `strong`                              | قوی              | `<strong>`     | normal  |                                                       |              |
| `style`              | `style`, `style_tag`                  | ظاهر سفارشی, سبک | `<style>`      | rawtext |                                                       |              |
| `subscript`          | `subscript`, `sub`                    | زیرنویس          | `<sub>`        | normal  |                                                       |              |
| `summary`            | `summary`                             | خلاصه            | `<summary>`    | normal  | `details`                                             |              |
| `superscript`        | `superscript`, `sup`                  | بالانویس         | `<sup>`        | normal  |                                                       |              |
| `svg`                | `svg`                                 | اس‌وی‌جی         | `<svg>`        | rawtext |                                                       |              |
| `table`              | `table`                               | جدول             | `<table>`      | normal  |                                                       |              |
| `table body`         | `main`                                | اصلی             | `<tbody>`      | normal  | `table`                                               |              |
| `table footer`       | `footer`                              | پاورقی           | `<tfoot>`      | normal  | `table`                                               |              |
| `table header`       | `header`                              | سرصفحه           | `<thead>`      | normal  | `table`                                               |              |
| `template`           | `template`                            | قالب             | `<template>`   | normal  |                                                       |              |
| `term`               | `term`, `dt`                          | واژه             | `<dt>`         | normal  | `description list`                                    |              |
| `term in box`        | `term`, `dt`                          | واژه             | `<dt>`         | normal  | `description list` > `box`                            |              |
| `text area`          | `text area`, `textarea`               | متن بلند         | `<textarea>`   | normal  |                                                       |              |
| `textarea input`     | `input`                               | ورودی            | `<textarea>`   | normal  |                                                       |              |
| `time`               | `time`                                | زمان             | `<time>`       | normal  |                                                       |              |
| `track`              | `track`                               | شیار             | `<track>`      | void    | `video`, `audio`                                      | `url`        |
| `underline`          | `underline`, `u`                      | زیرخط            | `<u>`          | normal  |                                                       |              |
| `variable`           | `variable`, `var`                     | متغیر            | `<var>`        | normal  |                                                       |              |
| `video`              | `video`                               | ویدیو            | `<video>`      | normal  |                                                       |              |
| `word break`         | `word break`, `wbr`                   | شکست واژه        | `<wbr>`        | void    |                                                       |              |

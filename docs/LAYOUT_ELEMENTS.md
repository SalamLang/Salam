# Layout elements

Every element the layout DSL knows (the HTML elements and the shadcn design system components such as `card`, `dialog` and `tabs`, see `docs/design/DESIGN_SYSTEMS.md`), generated from `std/layout/elements/`. The HTML tag name is accepted as an English name too, except for the table parts that come from context: inside `table`, write `header`/`main`/`footer` and `column`, never `thead`/`tbody`/`tfoot`/`th`/`td`. Elements that share a name are context variants: the compiler picks one by where it sits (see `docs/LAYOUT_SCHEMA.md`). The ruby elements were `روبی`, `متن روبی` and `پرانتز روبی` in Persian, and those names still work.

Every element works on the web. For the native target (`salam check --ui=native`, see `docs/design/NATIVE_UI.md`), `meta`, `head link`, `base`, `noscript` and `track` are web metadata that a native window ignores (W166), and `script`, `style`, `iframe`, `embed`, `object`, `template`, `slot`, `svg`, `math`, `image map` and `area` need a browser (E165). Put them inside `if SALAM_UI_WEB:` to keep one page for both targets.

| element                    | English names                         | Persian                | HTML           | kind    | must be inside                                        | required      |
| -------------------------- | ------------------------------------- | ---------------------- | -------------- | ------- | ----------------------------------------------------- | ------------- |
| `abbreviation`             | `abbreviation`, `abbr`                | کوته‌نوشت              | `<abbr>`       | normal  |                                                       |               |
| `accordion`                | `accordion`                           | آکاردئون               | `<div>`        | normal  |                                                       |               |
| `accordion content`        | `accordion content`                   | محتوای آکاردئون        | `<div>`        | normal  |                                                       |               |
| `accordion item`           | `accordion item`                      | بخش آکاردئون           | `<div>`        | normal  |                                                       |               |
| `accordion trigger`        | `accordion trigger`                   | دکمه آکاردئون          | `<div>`        | normal  |                                                       |               |
| `address`                  | `address`                             | آدرس                   | `<address>`    | normal  |                                                       |               |
| `alert`                    | `alert`                               | هشدار                  | `<div>`        | normal  |                                                       |               |
| `alert description`        | `alert description`                   | شرح هشدار              | `<div>`        | normal  |                                                       |               |
| `alert dialog`             | `alert dialog`                        | پنجره هشدار            | `<div>`        | normal  |                                                       |               |
| `alert dialog action`      | `alert dialog action`                 | دکمه تایید هشدار       | `<div>`        | normal  |                                                       |               |
| `alert dialog cancel`      | `alert dialog cancel`                 | دکمه انصراف هشدار      | `<div>`        | normal  |                                                       |               |
| `alert dialog description` | `alert dialog description`            | توضیح هشدار            | `<div>`        | normal  |                                                       |               |
| `alert dialog footer`      | `alert dialog footer`                 | پایین هشدار            | `<div>`        | normal  |                                                       |               |
| `alert dialog header`      | `alert dialog header`                 | سرایند هشدار           | `<div>`        | normal  |                                                       |               |
| `alert dialog media`       | `alert dialog media`                  | تصویر هشدار            | `<div>`        | normal  |                                                       |               |
| `alert dialog title`       | `alert dialog title`                  | عنوان هشدار            | `<div>`        | normal  |                                                       |               |
| `alert title`              | `alert title`                         | تیتر هشدار             | `<div>`        | normal  |                                                       |               |
| `area`                     | `area`                                | ناحیه                  | `<area>`       | void    | anywhere inside `image map`                           |               |
| `article`                  | `article`                             | مقاله                  | `<article>`    | normal  |                                                       |               |
| `aside`                    | `aside`, `sidebar`                    | کناری                  | `<aside>`      | normal  |                                                       |               |
| `aspect ratio`             | `aspect ratio`                        | نسبت تصویر             | `<div>`        | normal  |                                                       |               |
| `audio`                    | `audio`                               | صدا                    | `<audio>`      | normal  |                                                       |               |
| `avatar`                   | `avatar`                              | آواتار                 | `<span>`       | normal  |                                                       |               |
| `avatar badge`             | `avatar badge`                        | نشان آواتار            | `<span>`       | normal  |                                                       |               |
| `avatar fallback`          | `avatar fallback`                     | جایگزین آواتار         | `<span>`       | normal  |                                                       |               |
| `avatar group`             | `avatar group`                        | گروه آواتار            | `<div>`        | normal  |                                                       |               |
| `avatar group count`       | `avatar group count`                  | شمار گروه آواتار       | `<div>`        | normal  |                                                       |               |
| `avatar image`             | `avatar image`                        | تصویر آواتار           | `<img>`        | void    |                                                       | `url`         |
| `badge`                    | `badge`                               | نشان                   | `<span>`       | normal  |                                                       |               |
| `base`                     | `base`                                | پایه نشانی             | `<base>`       | void    | `page`                                                |               |
| `block quote`              | `block quote`, `blockquote`           | نقل قول بلند           | `<blockquote>` | normal  |                                                       |               |
| `bold`                     | `bold`, `b`                           | پررنگ                  | `<b>`          | normal  |                                                       |               |
| `box`                      | `box`, `div`                          | جعبه                   | `<div>`        | normal  |                                                       |               |
| `breadcrumb`               | `breadcrumb`                          | مسیر                   | `<nav>`        | normal  |                                                       |               |
| `breadcrumb ellipsis`      | `breadcrumb ellipsis`                 | سه نقطه مسیر           | `<li>`         | normal  |                                                       |               |
| `breadcrumb item`          | `breadcrumb item`                     | مورد مسیر              | `<li>`         | normal  |                                                       |               |
| `breadcrumb page`          | `breadcrumb page`                     | صفحه مسیر              | `<li>`         | normal  |                                                       |               |
| `breadcrumb separator`     | `breadcrumb separator`                | جداکننده مسیر          | `<li>`         | normal  |                                                       |               |
| `break`                    | `break`, `br`                         | شکست                   | `<br>`         | void    |                                                       |               |
| `button`                   | `button`                              | دکمه                   | `<button>`     | normal  |                                                       |               |
| `button group`             | `button group`                        | گروه دکمه              | `<div>`        | normal  |                                                       |               |
| `button group separator`   | `button group separator`              | جداکننده گروه دکمه     | `<div>`        | normal  |                                                       |               |
| `button group text`        | `button group text`                   | متن گروه دکمه          | `<div>`        | normal  |                                                       |               |
| `calendar`                 | `calendar`                            | تقویم                  | `<div>`        | normal  |                                                       |               |
| `canvas`                   | `canvas`                              | بوم                    | `<canvas>`     | normal  |                                                       |               |
| `caption`                  | `caption`                             | عنوان جدول             | `<caption>`    | normal  | `table`                                               |               |
| `card`                     | `card`                                | کارت                   | `<div>`        | normal  |                                                       |               |
| `card action`              | `card action`                         | کنش کارت               | `<div>`        | normal  |                                                       |               |
| `card content`             | `card content`                        | محتوای کارت            | `<div>`        | normal  |                                                       |               |
| `card description`         | `card description`                    | توضیح کارت             | `<div>`        | normal  |                                                       |               |
| `card footer`              | `card footer`                         | پاورقی کارت            | `<div>`        | normal  |                                                       |               |
| `card header`              | `card header`                         | سربرگ کارت             | `<div>`        | normal  |                                                       |               |
| `card title`               | `card title`                          | عنوان کارت             | `<div>`        | normal  |                                                       |               |
| `carousel`                 | `carousel`                            | چرخ‌فلک                | `<div>`        | normal  |                                                       |               |
| `carousel content`         | `carousel content`                    | محتوای چرخ‌فلک         | `<div>`        | normal  |                                                       |               |
| `carousel item`            | `carousel item`                       | اسلاید                 | `<div>`        | normal  |                                                       |               |
| `carousel next`            | `carousel next`                       | بعدی چرخ‌فلک           | `<button>`     | normal  |                                                       |               |
| `carousel previous`        | `carousel previous`                   | قبلی چرخ‌فلک           | `<button>`     | normal  |                                                       |               |
| `chart`                    | `chart`                               | نمودار                 | `<div>`        | normal  |                                                       |               |
| `chart point`              | `chart point`                         | نقطه نمودار            | `<span>`       | normal  |                                                       |               |
| `chart series`             | `chart series`                        | سری نمودار             | `<div>`        | normal  |                                                       |               |
| `cite`                     | `cite`                                | نام اثر                | `<cite>`       | normal  |                                                       |               |
| `code`                     | `code`                                | کد                     | `<code>`       | normal  |                                                       |               |
| `collapsible`              | `collapsible`                         | جمع شونده              | `<div>`        | normal  |                                                       |               |
| `collapsible content`      | `collapsible content`                 | محتوای جمع شونده       | `<div>`        | normal  |                                                       |               |
| `collapsible trigger`      | `collapsible trigger`                 | دکمه جمع شونده         | `<div>`        | normal  |                                                       |               |
| `column`                   | `column`, `cell`                      | ستون, سلول             | `<td>`         | normal  | `row`                                                 |               |
| `column definition`        | `column definition`, `col`            | تعریف ستون             | `<col>`        | void    | `column group`                                        |               |
| `column group`             | `column group`, `colgroup`            | گروه ستون              | `<colgroup>`   | normal  | `table`                                               |               |
| `combobox`                 | `combobox`                            | جعبه ترکیبی            | `<div>`        | normal  |                                                       |               |
| `combobox item`            | `combobox item`                       | گزینه جعبه ترکیبی      | `<div>`        | normal  |                                                       |               |
| `command`                  | `command`                             | فرمان                  | `<div>`        | normal  |                                                       |               |
| `command dialog`           | `command dialog`                      | پنجره فرمان            | `<dialog>`     | normal  |                                                       |               |
| `command empty`            | `command empty`                       | خالی فرمان             | `<div>`        | normal  |                                                       |               |
| `command group`            | `command group`                       | گروه فرمان             | `<div>`        | normal  |                                                       |               |
| `command input`            | `command input`                       | ورودی فرمان            | `<div>`        | normal  |                                                       |               |
| `command item`             | `command item`                        | گزینه فرمان            | `<div>`        | normal  |                                                       |               |
| `command list`             | `command list`                        | فهرست فرمان            | `<div>`        | normal  |                                                       |               |
| `command separator`        | `command separator`                   | جداکننده فرمان         | `<div>`        | normal  |                                                       |               |
| `command shortcut`         | `command shortcut`                    | میانبر فرمان           | `<span>`       | normal  |                                                       |               |
| `context menu`             | `context menu`                        | منوی راست کلیک         | `<div>`        | normal  |                                                       |               |
| `context menu content`     | `context menu content`                | محتوای منوی راست کلیک  | `<div>`        | normal  |                                                       |               |
| `context menu trigger`     | `context menu trigger`                | ناحیه راست کلیک        | `<div>`        | normal  |                                                       |               |
| `data`                     | `data`                                | داده                   | `<data>`       | normal  |                                                       | `value`       |
| `data list`                | `data list`, `datalist`               | فهرست پیشنهاد          | `<datalist>`   | normal  |                                                       |               |
| `date picker`              | `date picker`                         | انتخابگر تاریخ         | `<div>`        | normal  |                                                       |               |
| `definition`               | `definition`, `dfn`                   | تعریف                  | `<dfn>`        | normal  |                                                       |               |
| `deleted`                  | `deleted`, `del`                      | حذف شده                | `<del>`        | normal  |                                                       |               |
| `description`              | `description`, `dd`                   | شرح                    | `<dd>`         | normal  | `description list`                                    |               |
| `description in box`       | `description`, `dd`                   | شرح                    | `<dd>`         | normal  | `description list` > `box`                            |               |
| `description list`         | `description list`, `dl`              | فهرست تعریف            | `<dl>`         | normal  |                                                       |               |
| `details`                  | `details`                             | جزئیات                 | `<details>`    | normal  |                                                       |               |
| `dialog`                   | `dialog`                              | پنجره گفتگو            | `<dialog>`     | normal  |                                                       |               |
| `dialog close`             | `dialog close`                        | دکمه بستن پنجره        | `<div>`        | normal  |                                                       |               |
| `dialog description`       | `dialog description`                  | توضیح پنجره            | `<div>`        | normal  |                                                       |               |
| `dialog footer`            | `dialog footer`                       | پایین پنجره            | `<div>`        | normal  |                                                       |               |
| `dialog header`            | `dialog header`                       | سرایند پنجره           | `<div>`        | normal  |                                                       |               |
| `dialog title`             | `dialog title`                        | عنوان پنجره            | `<div>`        | normal  |                                                       |               |
| `dialog trigger`           | `dialog trigger`                      | دکمه پنجره             | `<div>`        | normal  |                                                       |               |
| `direction override`       | `direction override`, `bdo`           | جهت اجباری             | `<bdo>`        | normal  |                                                       | `dir`         |
| `drawer`                   | `drawer`                              | کشو                    | `<div>`        | normal  |                                                       |               |
| `drawer description`       | `drawer description`                  | توضیح کشو              | `<div>`        | normal  |                                                       |               |
| `drawer footer`            | `drawer footer`                       | پایین کشو              | `<div>`        | normal  |                                                       |               |
| `drawer header`            | `drawer header`                       | سرایند کشو             | `<div>`        | normal  |                                                       |               |
| `drawer title`             | `drawer title`                        | عنوان کشو              | `<div>`        | normal  |                                                       |               |
| `dropdown menu`            | `dropdown menu`                       | منوی کشویی             | `<div>`        | normal  |                                                       |               |
| `embed`                    | `embed`                               | جاسازی                 | `<embed>`      | void    |                                                       | `url`         |
| `emphasis`                 | `emphasis`, `em`                      | تاکید                  | `<em>`         | normal  |                                                       |               |
| `empty`                    | `empty`                               | خالی                   | `<div>`        | normal  |                                                       |               |
| `empty content`            | `empty content`                       | محتوای خالی            | `<div>`        | normal  |                                                       |               |
| `empty description`        | `empty description`                   | توضیح خالی             | `<div>`        | normal  |                                                       |               |
| `empty header`             | `empty header`                        | سربرگ خالی             | `<div>`        | normal  |                                                       |               |
| `empty media`              | `empty media`                         | رسانه خالی             | `<div>`        | normal  |                                                       |               |
| `empty title`              | `empty title`                         | عنوان خالی             | `<div>`        | normal  |                                                       |               |
| `entry`                    | `entry`                               | مدخل                   | `<div>`        | normal  |                                                       |               |
| `entry actions`            | `entry actions`                       | کنش های مدخل           | `<div>`        | normal  |                                                       |               |
| `entry content`            | `entry content`                       | محتوای مدخل            | `<div>`        | normal  |                                                       |               |
| `entry description`        | `entry description`                   | توضیح مدخل             | `<p>`          | normal  |                                                       |               |
| `entry footer`             | `entry footer`                        | پاورقی مدخل            | `<div>`        | normal  |                                                       |               |
| `entry group`              | `entry group`                         | گروه مدخل              | `<div>`        | normal  |                                                       |               |
| `entry header`             | `entry header`                        | سربرگ مدخل             | `<div>`        | normal  |                                                       |               |
| `entry media`              | `entry media`                         | رسانه مدخل             | `<div>`        | normal  |                                                       |               |
| `entry separator`          | `entry separator`                     | جداکننده مدخل          | `<div>`        | normal  |                                                       |               |
| `entry title`              | `entry title`                         | عنوان مدخل             | `<div>`        | normal  |                                                       |               |
| `field`                    | `field`                               | فیلد                   | `<div>`        | normal  |                                                       |               |
| `field content`            | `field content`                       | محتوای فیلد            | `<div>`        | normal  |                                                       |               |
| `field description`        | `field description`                   | توضیح فیلد             | `<p>`          | normal  |                                                       |               |
| `field error`              | `field error`                         | خطای فیلد              | `<div>`        | normal  |                                                       |               |
| `field group`              | `field group`                         | گروه فیلدها            | `<div>`        | normal  |                                                       |               |
| `field label`              | `field label`                         | برچسب فیلد             | `<label>`      | normal  |                                                       |               |
| `field separator`          | `field separator`                     | جداکننده فیلد          | `<div>`        | normal  |                                                       |               |
| `field set`                | `field set`, `fieldset`               | گروه فیلد              | `<fieldset>`   | normal  |                                                       |               |
| `field title`              | `field title`                         | عنوان فیلد             | `<div>`        | normal  |                                                       |               |
| `figure`                   | `figure`                              | شکل                    | `<figure>`     | normal  |                                                       |               |
| `figure caption`           | `figure caption`, `figcaption`        | زیرنویس شکل            | `<figcaption>` | normal  | `figure`                                              |               |
| `font`                     | `font`                                | قلم                    | `@font-face`   | void    | `page`                                                | `name`, `url` |
| `footer`                   | `footer`                              | پاورقی                 | `<footer>`     | normal  |                                                       |               |
| `form`                     | `form`                                | فرم                    | `<form>`       | normal  |                                                       |               |
| `global`                   | `global`, `global style`              | سراسری                 | (none)         | virtual |                                                       |               |
| `h1`                       | `h1`                                  |                        | `<h1>`         | normal  |                                                       |               |
| `h2`                       | `h2`                                  |                        | `<h2>`         | normal  |                                                       |               |
| `h3`                       | `h3`                                  |                        | `<h3>`         | normal  |                                                       |               |
| `h4`                       | `h4`                                  |                        | `<h4>`         | normal  |                                                       |               |
| `h5`                       | `h5`                                  |                        | `<h5>`         | normal  |                                                       |               |
| `h6`                       | `h6`                                  |                        | `<h6>`         | normal  |                                                       |               |
| `head link`                | `head link`, `head_link`              | پیوند سر               | `<link>`       | void    | `page`                                                | `rel`, `url`  |
| `header`                   | `header`                              | سرصفحه                 | `<header>`     | normal  |                                                       |               |
| `header column`            | `column`, `cell`                      | ستون, سلول             | `<th>`         | normal  | `table header` > `row`                                |               |
| `heading`                  | `heading`                             | سرتیتر                 | `<h1>`         | normal  |                                                       |               |
| `heading column`           | `column`, `cell`                      | ستون, سلول             | `<th>`         | normal  | `row`                                                 |               |
| `heading group`            | `heading group`, `hgroup`             | گروه سرتیتر            | `<hgroup>`     | normal  |                                                       |               |
| `hover card`               | `hover card`                          | کارت شناور             | `<div>`        | normal  |                                                       |               |
| `hover card content`       | `hover card content`                  | محتوای کارت شناور      | `<div>`        | normal  |                                                       |               |
| `hover card trigger`       | `hover card trigger`                  | دکمه کارت شناور        | `<div>`        | normal  |                                                       |               |
| `iframe`                   | `iframe`                              | قاب درونی              | `<iframe>`     | normal  |                                                       |               |
| `image`                    | `image`, `img`                        | تصویر                  | `<img>`        | void    |                                                       | `url`         |
| `image map`                | `image map`, `map`                    | نقشه تصویر             | `<map>`        | normal  |                                                       | `name`        |
| `input`                    | `input`                               | ورودی                  | `<input>`      | void    |                                                       |               |
| `input group`              | `input group`                         | گروه ورودی             | `<div>`        | normal  |                                                       |               |
| `input group addon`        | `input group addon`                   | افزودنی گروه ورودی     | `<div>`        | normal  |                                                       |               |
| `input group text`         | `input group text`                    | متن گروه ورودی         | `<span>`       | normal  |                                                       |               |
| `input otp`                | `input otp`                           | کد یکبار مصرف          | `<div>`        | normal  |                                                       |               |
| `inserted`                 | `inserted`, `ins`                     | افزوده                 | `<ins>`        | normal  |                                                       |               |
| `isolate`                  | `isolate`, `bdi`                      | جداساز جهت             | `<bdi>`        | normal  |                                                       |               |
| `italic`                   | `italic`, `i`                         | کج                     | `<i>`          | normal  |                                                       |               |
| `item`                     | `item`, `li`                          | مورد                   | `<li>`         | normal  | `list`, `ordered list`, `menu`                        |               |
| `keyboard`                 | `keyboard`, `kbd`                     | صفحه کلید              | `<kbd>`        | normal  |                                                       |               |
| `keyboard group`           | `keyboard group`                      | گروه صفحه کلید         | `<kbd>`        | normal  |                                                       |               |
| `label`                    | `label`                               | برچسب                  | `<label>`      | normal  |                                                       |               |
| `page`                     | `page`                                | صفحه                   | (none)         | virtual |                                                       |               |
| `legend`                   | `legend`                              | عنوان گروه             | `<legend>`     | normal  | `field set`                                           |               |
| `line`                     | `line`, `hr`                          | خط                     | `<hr>`         | void    |                                                       |               |
| `link`                     | `link`, `a`                           | پیوند                  | `<a>`          | normal  |                                                       | `url`         |
| `list`                     | `list`, `ul`                          | فهرست                  | `<ul>`         | normal  |                                                       |               |
| `main`                     | `main`                                | اصلی                   | `<main>`       | normal  |                                                       |               |
| `mark`                     | `mark`, `highlight`                   | برجسته                 | `<mark>`       | normal  |                                                       |               |
| `math`                     | `math`                                | ریاضی                  | `<math>`       | rawtext |                                                       |               |
| `media`                    | `media`                               | رسانه                  | (none)         | virtual |                                                       |               |
| `menu`                     | `menu`                                | منو                    | `<menu>`       | normal  |                                                       |               |
| `menu checkbox item`       | `menu checkbox item`                  | گزینه تیکی منو         | `<div>`        | normal  |                                                       |               |
| `menu content`             | `menu content`                        | محتوای منو             | `<div>`        | normal  |                                                       |               |
| `menu group`               | `menu group`                          | گروه منو               | `<div>`        | normal  |                                                       |               |
| `menu item`                | `menu item`                           | گزینه منو              | `<div>`        | normal  |                                                       |               |
| `menu label`               | `menu label`                          | برچسب منو              | `<div>`        | normal  |                                                       |               |
| `menu radio group`         | `menu radio group`                    | گروه رادیویی منو       | `<div>`        | normal  |                                                       |               |
| `menu radio item`          | `menu radio item`                     | گزینه رادیویی منو      | `<div>`        | normal  |                                                       |               |
| `menu separator`           | `menu separator`                      | جداکننده منو           | `<div>`        | normal  |                                                       |               |
| `menu shortcut`            | `menu shortcut`                       | میانبر منو             | `<div>`        | normal  |                                                       |               |
| `menu sub`                 | `menu sub`                            | زیرمنو                 | `<div>`        | normal  |                                                       |               |
| `menu sub content`         | `menu sub content`                    | محتوای زیرمنو          | `<div>`        | normal  |                                                       |               |
| `menu sub trigger`         | `menu sub trigger`                    | دکمه زیرمنو            | `<div>`        | normal  |                                                       |               |
| `menu trigger`             | `menu trigger`                        | دکمه منو               | `<div>`        | normal  |                                                       |               |
| `menubar`                  | `menubar`                             | نوار منو               | `<div>`        | normal  |                                                       |               |
| `menubar menu`             | `menubar menu`                        | منوی نوار              | `<div>`        | normal  |                                                       |               |
| `menubar trigger`          | `menubar trigger`                     | دکمه منوی نوار         | `<div>`        | normal  |                                                       |               |
| `meta`                     | `meta`                                | متا                    | `<meta>`       | void    | `page`                                                |               |
| `meter`                    | `meter`, `gauge`                      | سنجه                   | `<meter>`      | normal  |                                                       | `value`       |
| `nav`                      | `nav`                                 | ناوبری                 | `<nav>`        | normal  |                                                       |               |
| `navigation menu`          | `navigation menu`                     | منوی ناوبری            | `<div>`        | normal  |                                                       |               |
| `navigation menu content`  | `navigation menu content`             | محتوای منوی ناوبری     | `<div>`        | normal  |                                                       |               |
| `navigation menu item`     | `navigation menu item`                | مورد منوی ناوبری       | `<div>`        | normal  |                                                       |               |
| `navigation menu link`     | `navigation menu link`                | پیوند منوی ناوبری      | `<div>`        | normal  |                                                       |               |
| `navigation menu list`     | `navigation menu list`                | فهرست منوی ناوبری      | `<div>`        | normal  |                                                       |               |
| `navigation menu trigger`  | `navigation menu trigger`             | دکمه منوی ناوبری       | `<div>`        | normal  |                                                       |               |
| `noscript`                 | `noscript`, `no script`               | بدون اسکریپت           | `<noscript>`   | normal  |                                                       |               |
| `object`                   | `object`                              | شیء                    | `<object>`     | normal  |                                                       | `url`         |
| `option`                   | `option`                              | گزینه                  | `<option>`     | normal  | `select`, `option group`, `data list`                 |               |
| `option group`             | `option group`, `optgroup`            | گروه گزینه             | `<optgroup>`   | normal  | `select`                                              | `label`       |
| `ordered list`             | `ordered list`, `numbered list`, `ol` | فهرست شماره‌دار        | `<ol>`         | normal  |                                                       |               |
| `output`                   | `output`                              | خروجی                  | `<output>`     | normal  |                                                       |               |
| `pagination`               | `pagination`                          | صفحه بندی              | `<nav>`        | normal  |                                                       |               |
| `pagination ellipsis`      | `pagination ellipsis`                 | سه نقطه صفحه           | `<li>`         | normal  |                                                       |               |
| `pagination link`          | `pagination link`                     | پیوند صفحه             | `<a>`          | normal  |                                                       |               |
| `pagination next`          | `pagination next`                     | صفحه بعد               | `<a>`          | normal  |                                                       |               |
| `pagination previous`      | `pagination previous`                 | صفحه قبل               | `<a>`          | normal  |                                                       |               |
| `paragraph`                | `paragraph`, `p`                      | پاراگراف               | `<p>`          | normal  |                                                       |               |
| `picture`                  | `picture`                             | عکس                    | `<picture>`    | normal  |                                                       |               |
| `popover`                  | `popover`                             | پنجره شناور            | `<div>`        | normal  |                                                       |               |
| `popover content`          | `popover content`                     | محتوای پنجره شناور     | `<div>`        | normal  |                                                       |               |
| `popover description`      | `popover description`                 | توضیح پنجره شناور      | `<div>`        | normal  |                                                       |               |
| `popover header`           | `popover header`                      | سرایند پنجره شناور     | `<div>`        | normal  |                                                       |               |
| `popover title`            | `popover title`                       | عنوان پنجره شناور      | `<div>`        | normal  |                                                       |               |
| `popover trigger`          | `popover trigger`                     | دکمه پنجره شناور       | `<div>`        | normal  |                                                       |               |
| `preformatted`             | `preformatted`, `pre`                 | پیش‌قالب               | `<pre>`        | normal  |                                                       |               |
| `progress`                 | `progress`                            | پیشرفت                 | `<progress>`   | normal  |                                                       |               |
| `quote`                    | `quote`, `q`                          | نقل قول                | `<q>`          | normal  |                                                       |               |
| `radio group`              | `radio group`                         | گروه رادیویی           | `<div>`        | normal  |                                                       |               |
| `resizable`                | `resizable`                           | قابل تغییر             | `<div>`        | normal  |                                                       |               |
| `resizable handle`         | `resizable handle`                    | دستگیره قابل تغییر     | `<div>`        | normal  |                                                       |               |
| `resizable panel`          | `resizable panel`                     | بخش قابل تغییر         | `<div>`        | normal  |                                                       |               |
| `row`                      | `row`, `tr`                           | ردیف                   | `<tr>`         | normal  | `table`, `table header`, `table body`, `table footer` |               |
| `ruby`                     | `ruby`                                | آوانما                 | `<ruby>`       | normal  |                                                       |               |
| `ruby parenthesis`         | `ruby parenthesis`, `rp`              | پرانتز آوانما          | `<rp>`         | normal  | `ruby`                                                |               |
| `ruby text`                | `ruby text`, `rt`                     | متن آوانما             | `<rt>`         | normal  | `ruby`                                                |               |
| `sample`                   | `sample`, `samp`                      | نمونه خروجی            | `<samp>`       | normal  |                                                       |               |
| `script`                   | `script`                              | اسکریپت سفارشی         | `<script>`     | rawtext |                                                       |               |
| `scroll area`              | `scroll area`                         | ناحیه پیمایش           | `<div>`        | normal  |                                                       |               |
| `search`                   | `search`                              | جستجو                  | `<search>`     | normal  |                                                       |               |
| `section`                  | `section`                             | بخش                    | `<section>`    | normal  |                                                       |               |
| `select`                   | `select`, `dropdown`                  | انتخاب                 | `<select>`     | normal  |                                                       |               |
| `select box`               | `select box`                          | جعبه انتخاب            | `<div>`        | normal  |                                                       |               |
| `select content`           | `select content`                      | محتوای جعبه انتخاب     | `<div>`        | normal  |                                                       |               |
| `select group`             | `select group`                        | گروه جعبه انتخاب       | `<div>`        | normal  |                                                       |               |
| `select item`              | `select item`                         | گزینه جعبه انتخاب      | `<div>`        | normal  |                                                       |               |
| `select separator`         | `select separator`                    | جداکننده جعبه انتخاب   | `<div>`        | normal  |                                                       |               |
| `select trigger`           | `select trigger`                      | دکمه جعبه انتخاب       | `<button>`     | normal  |                                                       |               |
| `separator`                | `separator`                           | جداکننده               | `<div>`        | normal  |                                                       |               |
| `sheet`                    | `sheet`                               | برگه کناری             | `<div>`        | normal  |                                                       |               |
| `sheet description`        | `sheet description`                   | توضیح برگه             | `<div>`        | normal  |                                                       |               |
| `sheet footer`             | `sheet footer`                        | پایین برگه             | `<div>`        | normal  |                                                       |               |
| `sheet header`             | `sheet header`                        | سرایند برگه            | `<div>`        | normal  |                                                       |               |
| `sheet title`              | `sheet title`                         | عنوان برگه             | `<div>`        | normal  |                                                       |               |
| `sidebar content`          | `sidebar content`                     | محتوای نوار کناری      | `<div>`        | normal  |                                                       |               |
| `sidebar footer`           | `sidebar footer`                      | پاورقی نوار کناری      | `<div>`        | normal  |                                                       |               |
| `sidebar group`            | `sidebar group`                       | گروه نوار کناری        | `<div>`        | normal  |                                                       |               |
| `sidebar group content`    | `sidebar group content`               | محتوای گروه نوار کناری | `<div>`        | normal  |                                                       |               |
| `sidebar group label`      | `sidebar group label`                 | برچسب گروه نوار کناری  | `<div>`        | normal  |                                                       |               |
| `sidebar header`           | `sidebar header`                      | سرایند نوار کناری      | `<div>`        | normal  |                                                       |               |
| `sidebar inset`            | `sidebar inset`                       | محتوای اصلی نوار کناری | `<main>`       | normal  |                                                       |               |
| `sidebar menu`             | `sidebar menu`                        | منوی نوار کناری        | `<ul>`         | normal  |                                                       |               |
| `sidebar menu badge`       | `sidebar menu badge`                  | نشان منوی نوار کناری   | `<span>`       | normal  |                                                       |               |
| `sidebar menu button`      | `sidebar menu button`                 | دکمه منوی نوار کناری   | `<button>`     | normal  |                                                       |               |
| `sidebar menu item`        | `sidebar menu item`                   | آیتم منوی نوار کناری   | `<li>`         | normal  |                                                       |               |
| `sidebar panel`            | `sidebar panel`                       | پنل نوار کناری         | `<aside>`      | normal  |                                                       |               |
| `sidebar provider`         | `sidebar provider`                    | فراهم کننده نوار کناری | `<div>`        | normal  |                                                       |               |
| `sidebar separator`        | `sidebar separator`                   | جداکننده نوار کناری    | `<div>`        | normal  |                                                       |               |
| `sidebar trigger`          | `sidebar trigger`                     | دکمه نوار کناری        | `<button>`     | normal  |                                                       |               |
| `skeleton`                 | `skeleton`                            | اسکلت                  | `<div>`        | normal  |                                                       |               |
| `slot`                     | `slot`                                | جایگاه                 | `<slot>`       | normal  |                                                       |               |
| `small`                    | `small`                               | ریز                    | `<small>`      | normal  |                                                       |               |
| `source`                   | `source`                              | منبع                   | `<source>`     | void    | `picture`, `video`, `audio`                           |               |
| `span`                     | `span`                                | تکه                    | `<span>`       | normal  |                                                       |               |
| `spinner`                  | `spinner`                             | چرخنده                 | `<svg>`        | normal  |                                                       |               |
| `strikethrough`            | `strikethrough`, `s`                  | خط خورده               | `<s>`          | normal  |                                                       |               |
| `strong`                   | `strong`                              | قوی                    | `<strong>`     | normal  |                                                       |               |
| `style`                    | `style`, `style_tag`                  | ظاهر سفارشی, سبک       | `<style>`      | rawtext |                                                       |               |
| `subscript`                | `subscript`, `sub`                    | زیرنویس                | `<sub>`        | normal  |                                                       |               |
| `summary`                  | `summary`                             | خلاصه                  | `<summary>`    | normal  | `details`                                             |               |
| `superscript`              | `superscript`, `sup`                  | بالانویس               | `<sup>`        | normal  |                                                       |               |
| `svg`                      | `svg`                                 | اس‌وی‌جی               | `<svg>`        | rawtext |                                                       |               |
| `switch`                   | `switch`                              | کلید                   | `<button>`     | normal  |                                                       |               |
| `tab content`              | `tab content`                         | محتوای زبانه           | `<div>`        | normal  |                                                       |               |
| `tab list`                 | `tab list`                            | فهرست زبانه            | `<div>`        | normal  |                                                       |               |
| `tab trigger`              | `tab trigger`                         | دکمه زبانه             | `<div>`        | normal  |                                                       |               |
| `table`                    | `table`                               | جدول                   | `<table>`      | normal  |                                                       |               |
| `table body`               | `main`                                | اصلی                   | `<tbody>`      | normal  | `table`                                               |               |
| `table footer`             | `footer`                              | پاورقی                 | `<tfoot>`      | normal  | `table`                                               |               |
| `table header`             | `header`                              | سرصفحه                 | `<thead>`      | normal  | `table`                                               |               |
| `tabs`                     | `tabs`                                | زبانه ها               | `<div>`        | normal  |                                                       |               |
| `template`                 | `template`                            | قالب                   | `<template>`   | normal  |                                                       |               |
| `term`                     | `term`, `dt`                          | واژه                   | `<dt>`         | normal  | `description list`                                    |               |
| `term in box`              | `term`, `dt`                          | واژه                   | `<dt>`         | normal  | `description list` > `box`                            |               |
| `text area`                | `text area`, `textarea`               | متن بلند               | `<textarea>`   | normal  |                                                       |               |
| `textarea input`           | `input`                               | ورودی                  | `<textarea>`   | normal  |                                                       |               |
| `theme toggle`             | `theme toggle`                        | تغییر پوسته            | `<button>`     | normal  |                                                       |               |
| `time`                     | `time`                                | زمان                   | `<time>`       | normal  |                                                       |               |
| `toaster`                  | `toaster`                             | اعلان ها               | `<ol>`         | normal  |                                                       |               |
| `toggle`                   | `toggle`                              | تغییر وضعیت            | `<button>`     | normal  |                                                       |               |
| `toggle group`             | `toggle group`                        | گروه تغییر وضعیت       | `<div>`        | normal  |                                                       |               |
| `tooltip`                  | `tooltip`                             | راهنما                 | `<div>`        | normal  |                                                       |               |
| `tooltip content`          | `tooltip content`                     | محتوای راهنما          | `<div>`        | normal  |                                                       |               |
| `tooltip trigger`          | `tooltip trigger`                     | دکمه راهنما            | `<div>`        | normal  |                                                       |               |
| `track`                    | `track`                               | شیار                   | `<track>`      | void    | `video`, `audio`                                      | `url`         |
| `typography`               | `typography`                          | نگارش                  | `<div>`        | normal  |                                                       |               |
| `underline`                | `underline`, `u`                      | زیرخط                  | `<u>`          | normal  |                                                       |               |
| `variable`                 | `variable`, `var`                     | متغیر                  | `<var>`        | normal  |                                                       |               |
| `video`                    | `video`                               | ویدیو                  | `<video>`      | normal  |                                                       |               |
| `word break`               | `word break`, `wbr`                   | شکست واژه              | `<wbr>`        | void    |                                                       |               |

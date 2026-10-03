# Site

The source of [www.salamlang.ir](https://www.salamlang.ir). The generator is written in
Salam (`build.salam`) and turns shared templates plus per-language content into
static HTML. All text is in the generated HTML; the small `site.js` only
toggles the theme, the mobile menu and the copy buttons.

```
website/
├── build.salam          generator
├── build.sh             checks the examples, then runs the generator
├── templates/
│   ├── base.html        page shell: head, header, footer
│   └── pages/*.html     one file per page; the file name is the URL
├── content/<lang>/
│   ├── *.txt            strings for that language
│   └── examples/        Salam programs shown on the site, with their .out
└── assets/              css, js, fonts (Vazirmatn, Vazir Code), images
```

## Build

```sh
sh website/build.sh          # every language in content/
sh website/build.sh fa       # one language
```

The output goes to `website/dist/<lang>/`. Preview it with
`salam serve website/dist/fa`.

`build.sh` first runs every example and compares its output with the `.out`
file next to it, so the site never shows code that does not compile or output
that is out of date. The generator fails on any `{{key}}` a template uses that
the content does not define.

## Content files

```
# comment
key = one line of text
key <<
several lines,
HTML allowed
>>
```

Values are inserted as HTML. Keys ending in `_cmd` are shell commands and are
escaped automatically. `<page>.title` and `<page>.description` fill the
`<title>` and meta tags. `site.dir` (`rtl` or `ltr`) and `site.lang` set the
page direction and language, and `site.digits = fa` prints the version in
Persian digits.

SEO: each page's `<page>.title` is the full `<title>` (write the whole
phrase, including the brand), `<page>.description` the meta description and
`<page>.keywords` its keywords (falling back to `site.keywords`). The
generator builds the JSON-LD graph (`Organization`, `WebSite`,
`ComputerLanguage`, `SoftwareApplication`, `WebPage`, `BreadcrumbList`) and
`site.webmanifest` from the
`site.*` keys. `faq.N.q` / `faq.N.a` become both the visible FAQ on the home
page (`{{faq.html}}`) and its `FAQPage` structured data, so the two never
disagree. Put a Search Console token in `site.google_verification`.

Examples: `examples/name.salam` becomes `{{example.name}}`, a code window with
the highlighted code and, when `name.out` exists, its output. The highlighter
takes its word lists from `code.keywords`, `code.types` and `code.builtins`.
Code in Persian script is shown right to left, other code left to right,
whatever the page direction.

## Lessons

Each file in `content/<lang>/learn/` is one lesson, published at
`/learn/<name>/` (the file `07-loops.txt` becomes `/learn/loops/`; the number
only sets the order). A lesson defines `lesson.part`, `lesson.title`,
`lesson.summary`, `lesson.seo_title`, `lesson.description`, `lesson.lead` and a
`lesson.body` block. The body is HTML: every `<h2 id="...">` becomes an entry
in the lesson's contents list, and `{{example.name}}` inserts a checked
example. The generator builds the `/learn/` overview, the sidebar and the
previous/next links from the lesson files, grouped by `lesson.part`.

An example without a `.out` file is not run: a `layout:` page is built with
`salam layout build`, anything else is type-checked with `salam inspect`.

## Adding English (salamlang.org)

1. Copy `content/fa` to `content/en` and translate the `.txt` files.
2. Set `site.lang = en`, `site.dir = ltr`, `site.locale = en_US`,
   `site.digits = en` and `site.url = https://salamlang.org`.
3. Replace the Persian examples with English ones and regenerate their `.out`
   files (`salam run x.salam > x.out`).
4. Add a second deploy job for `dist/en` in
   `.github/workflows/website-deploy.yml`.

The CSS only uses logical properties (`margin-inline-start`, `inset-inline-end`
and so on), so the same design works in both directions.

## Deploy

`.github/workflows/website-deploy.yml` builds on every pull request that
touches the site. On `main` it also uploads `dist/fa` to `SSH_PATH_SITE` on the
server through `.github/actions/deploy-ssh`, using the `SSH_*` secrets.

The site is served from `www.salamlang.ir`; the bare `salamlang.ir` redirects
there. On the server, nginx needs a `www` block rooted at the deploy folder and
a redirect for the bare domain, for example:

```nginx
server { listen 80; server_name salamlang.ir; return 301 https://www.salamlang.ir$request_uri; }
server {
    listen 80;
    server_name www.salamlang.ir;
    root /hosts/salamlang.ir;
    error_page 404 /404.html;
    location / { try_files $uri $uri/ =404; }
    location /assets/ { add_header Cache-Control "public, max-age=2592000"; }
}
```

CSS and JS URLs carry a content hash (`?v=...`), so long caching is safe.

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
│   ├── examples/        Salam programs shown on the site, with their .out
│   └── static/          folders copied as-is into that language's output
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
that is out of date. It also refuses a std package that has no page in both
languages, with `check-std-coverage.py`. The generator fails on any `{{key}}` a
template uses that the content does not define.

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
escaped automatically. Keys containing `_code_<lang>`, for example `ai.api_code_python`, are
syntax highlighted at build time (`python`, `javascript`, `typescript`, `bash`, `json`, `yaml`, `toml`, `c`, `cpp`, `java`, `go`, `rust`, `php`). `<page>.title` and `<page>.description` fill the
`<title>` and meta tags. `site.dir` (`rtl` or `ltr`) and `site.lang` set the
page direction and language, and `site.digits = fa` prints the version in
Persian digits.

The Agent page links to the desktop builds on the Salam release itself:
`agent.release` is the release page and `agent.download` the filename prefix
the download links append to, for example
`{{agent.download}}windows-x86_64-setup.exe`. The agent carries the same
version as `VERSION` (kept in sync by `tools/salam/update-version.salam`), and
the compiler release workflow builds it and publishes its files in the same
release as the compiler. The deploy workflow looks up the newest published
release that actually carries the agent files and passes it in as
`SITE_AGENT_TAG` and `SITE_AGENT_VERSION`; without them the links fall back to
`v<VERSION>`. Releases up to 0.5.6 kept the agent in a separate `agent-v<VERSION>`
release, and the lookup still finds those until a newer one ships it.

SEO: each page's `<page>.title` is the full `<title>` (write the whole
phrase, including the brand), `<page>.description` the meta description and
`<page>.keywords` its keywords (falling back to `site.keywords`). The
generator builds the JSON-LD graph (`Organization`, `WebSite`,
`ComputerLanguage`, `SoftwareApplication`, `WebPage`, `BreadcrumbList`) and
`site.webmanifest` from the
`site.*` keys. `faq.N.q` / `faq.N.a` become both the visible FAQ on the home
page (`{{faq.html}}`) and its `FAQPage` structured data, so the two never
disagree. Put a Search Console token in `site.google_verification`.

For AI tools, each language also gets an `llms.txt` (the site description, then
every page, lesson and std package as a link with its summary, plus links to
the agent skill, the Claude Code plugin and the MCP server) and an
`llms-full.txt` with the text of every lesson and std page in Markdown. Their
headings and descriptions come from the `llms.*` keys, the agent links from
`link.skill`, `link.claude_plugin` and `link.mcp`.

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

An example without a `.out` file is not run: a `page:` program is built with
`salam page build`, anything else is type-checked with `salam inspect`.

A `page:` example (one whose source has a line that is exactly `page:` or
`صفحه:`) also gets a live preview: the generator builds it with `$SALAM page build
--inline` and puts the result in a sandboxed `<iframe srcdoc>` under the code,
sized to its content. Scripts run in the preview, but links and form submits
are blocked, so a click can never load the site into the frame. The compiler
is taken from `SALAM`, which `build.sh` always exports.

### Reference snippets

The web lessons (`page-*`, `css-*`, `events-*`) end with a reference of every
element, attribute, CSS property and event. Each row has a short example in an
element with `class="snip"` (`<code class="snip">` in a table cell,
`<pre class="code snip">` for a block). `check-snippets.py` compiles all of
them in both languages, so a renamed attribute or a changed Persian name fails
the build instead of shipping a wrong example. A snippet is an element
expression such as `link: url = "/" content = "Home" end`; the checker wraps it
in `page: ... end` and adds `import dom` when it calls `dom.`. A snippet that
contains its own `page:` line is built as it stands.

`SALAM=./salam python3 website/check-snippets.py [lesson.txt ...]` runs the
check by hand.

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

---
title: Full-stack development with one language: can Salam bring the frontend and the backend together?
seo_title: Full-Stack Development With One Language: Salam | Salam Magazine
description: Can one language cover frontend and backend? How Salam combines an HTTP server, typed server-rendered pages, browser code and databases, and where it stops.
keywords: full stack development, one language frontend backend, full stack language, server-side rendering, Salam web framework, isomorphic code, compile to JavaScript, Salam net/http, typed templates, web app in one language
category: build
summary: Salam can write the server, the database layer, the HTML and CSS, and the browser handlers in one language and one toolchain. This is how the pieces connect, with working code, and what is still missing.
date: 2026-11-11
---

Full-stack development with one language means writing the server, the data layer, the pages and the code that runs in the browser without switching languages at the boundary. Salam gets closer to that than most compiled languages: **one Salam program can serve HTTP, query a database, render typed HTML pages, and ship browser handlers that the same compiler translates to JavaScript.** It is not yet a replacement for a mature JavaScript stack on the client-side, and this article is precise about where the line is.

The question matters for small teams and for learners. Every language boundary in a web stack costs something: two sets of types that drift apart, validation written twice, two toolchains, two package managers, and a mental switch every time a bug crosses from the server to the page. Node.js made "JavaScript everywhere" popular for exactly that reason. Salam takes the other direction: a compiled, statically typed language that reaches up to the browser.

## What "one language" covers in Salam {#layers}

A typical web application has five layers. This is what Salam uses for each, all of it shipped with the compiler:

| Layer                            | In Salam                                                      | Where to read more                                                                 |
| -------------------------------- | ------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| HTTP server, routing, middleware | `net.http`, `net.router`, `net.http.middleware`               | [net.http](/std/net/http/), [fast web servers](/blog/fast-web-servers-with-salam/) |
| Data and SQL                     | `db` with SQLite, MySQL, PostgreSQL drivers, `db.sql` checks  | [typed SQL](/blog/typed-sql-database-apps/)                                        |
| HTML and CSS                     | the page language, `page:` and `page Name(params):`           | [the layout lesson](/learn/layout/)                                                |
| Browser behavior                 | event handlers in Salam, compiled to JavaScript               | [UI components and state](/blog/interactive-ui-components-state-events/)           |
| Build and deploy                 | `salam build`, `salam page build`, `salam web`, `salam serve` | [salam serve](/learn/serve/)                                                       |

There is no web framework to choose, because the pieces above are the framework, and there is no separate frontend build: no bundler, no transpiler configuration, no `node_modules`. The compiler is one executable.

## The backend: an HTTP server that renders typed pages {#backend}

The center of a Salam web application is an ordinary program. Handlers take a context id, read the request with functions such as `http.Param`, `http.Query`, `http.Form` and `http.Body`, and answer with `http.Text`, `http.Json` or `http.Html`.

The interesting part is the page. A `page` with a name and typed parameters is a runtime layout: the compiler builds the document structure and its CSS at compile time and turns the layout into a function that returns HTML with the values filled in. There is no template interpreter at runtime and no template file that can drift from the code. Every value from a parameter is HTML-escaped automatically.

The program below is a complete notes application in one file. It serves the page, accepts a form post, validates it, stores the note, and offers the same data as JSON for other clients. To make it runnable here, `main` starts the server in the background on a free port and plays the client itself.

```salam
import net.http
import net.router
import encoding.json
import str

mut notes := Vector {} as Vector<Note>

struct Note:
    @json "id"
    pub id: int = 0
    @json "text"
    pub text: str = ""
end

page NotesPage(items: Vector<Note>, problem: str):
    title = "Notes"
    heading: size = 1 content = "Notes" end
    if problem != "":
        paragraph: class = "error" content = problem end
    end
    list:
        each n in items:
            item: content = n.text end
        end
    end
    form:
        method = "post"
        action = "/notes"
        input: name = "text" end
        button: type = submit content = "Add" end
    end
end

func text problem(text: str): str:
    if text.len() == 0:
        ret "write something first"
    end
    if text.len() > 140:
        ret "keep it under 140 characters"
    end
    ret ""
end

func show(ctx: i64):
    http.Html(ctx, NotesPage(notes, ""))
end

func add(ctx: i64):
    text := http.Form(ctx, "text")
    problem := text problem(text)
    if problem != "":
        http.SetStatus(ctx, 422)
        http.Html(ctx, NotesPage(notes, problem))
        ret
    end
    notes.Push(Note { id = notes.len() + 1, text = str.Clone(text) })
    http.Redirect(ctx, "/")
end

func api(ctx: i64):
    http.Json(ctx, json.Marshal(notes))
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/", show)
    router.Post(r, "/notes", add)
    router.Get(r, "/api/notes", api)
    mut s := http.NewServer(0)
    http.Use(s, r)
    port := http.RunBackgroundPort(s)
    base := "http://127.0.0.1:" + port
    println http.Status(http.Post(base + "/notes", "text=<b>buy milk</b>"))
    println http.Status(http.Post(base + "/notes", "text="))
    println http.Body(http.Get(base + "/api/notes"))
    page := http.Body(http.Get(base + "/"))
    println page.Includes("<li>&lt;b&gt;buy milk&lt;/b&gt;</li>")
end
```

```output
302
422
[{"id":1,"text":"<b>buy milk</b>"}]
true
```

Read the output from the top. The first post succeeds and redirects back to the page. The empty post fails validation and gets a 422 with the page re-rendered around an error message. The JSON route returns the stored note exactly as typed, and the HTML route shows it escaped: the `<b>` a visitor typed is text on the page, never markup. Nobody had to remember to escape it.

Two details are worth noticing. `NotesPage` takes a `Vector<Note>`, the same struct the JSON API marshals, so the page and the API cannot disagree about what a note is; rename a field and both fail to compile. And the note is stored with `str.Clone(text)`: a value read from a request belongs to that request and is freed when the handler returns, so anything kept longer must be copied. That is the price of a server without a garbage collector, and the [net.http reference](/std/net/http/) states the rule.

## Rules that keep server-rendered pages safe {#safety}

A runtime layout is deliberately restricted, and the restrictions are what make it safe to feed it user data:

- **It is a pure function.** A layout cannot read files, call the network, generate random numbers or write globals; the compiler rejects it (E012). The handler fetches the data and passes it in; reading a `Vector` parameter, such as its length or its items, is allowed, but changing it is not.
- **Text is always escaped.** There is no "raw HTML" type to reach for by accident.
- **URLs and CSS values are typed.** A plain `str` cannot go where a URL, a color or a length belongs (E135). You parse it first with `web.ParseUrl`, `web.ParseColor` or `web.ParseLength`, which reject values such as `javascript:` links.
- **The structure is fixed at compile time.** Only the values change, so a visitor cannot inject an element.

These are the same lessons that template engines such as Go's `html/template` or React's JSX escaping learned over the years, enforced by the type checker instead of by convention.

## Browser code written in Salam {#browser}

A server-rendered page with forms is already a complete application, and for many internal tools and content sites it is the right design. When the page needs to react before a round trip, the handlers are written in Salam too. This page checks the note before the form is sent, using a helper function that lives in the same file:

```salam
import dom

func title problem(title: str): str:
    if title.len() == 0:
        ret "write something first"
    end
    if title.len() > 140:
        ret "keep it under 140 characters"
    end
    ret ""
end

func check(e: i64):
    problem := title problem(dom.GetValue(dom.ById("text")))
    dom.SetText(dom.ById("hint"), problem)
    if problem != "":
        dom.PreventDefault(e)
    end
end

page:
    title = "Notes"
    form:
        method = "post"
        action = "/notes"
        submit = check
        input: id = "text" name = "text" end
        button: type = submit content = "Add" end
    end
    paragraph: id = "hint" color = #b91c1c end
end
```

`salam page build notes.salam` compiles `check` and `title problem` with Salam's JavaScript backend into the page's script. The check in the browser is a convenience for the visitor; the server still validates, because anything that runs in the browser can be changed by the person using it. The lesson on [state, the DOM and timers](/learn/events/dom/) ends with the same warning.

Beyond single handlers there are three tools for the client-side:

- **Components with state**, where handlers only change variables and the page redraws itself. The article [Interactive user interfaces in Salam](/blog/interactive-ui-components-state-events/) covers them.
- **`salam js app.salam`** compiles a whole Salam program to JavaScript, or to a self-contained HTML page when the output ends in `.html`.
- **`salam web`** builds a page plus any `script: src = "app.salam"` programs it references into one HTML file, or into separate `.html`, `.css` and `.js` files with `--split`.

## Sharing code between the server and the browser {#sharing}

The real promise of one language is writing a rule once. In Salam, a helper file can be compiled into the server and into a browser script. A small project might look like this:

```text
notes/
  rules/rules.salam     # package rules: TitleProblem and friends
  main.salam            # the server: include rules "rules/rules.salam"
  client.salam          # browser code: include rules "rules/rules.salam"
  index.salam           # page: ... script: src = "client.salam" end
```

The shared file is plain Salam:

```salam fragment
package rules

pub func TitleProblem(title: str): str:
    if title.len() == 0:
        ret "a title is required"
    end
    if title.len() > 80:
        ret "keep the title under 80 characters"
    end
    ret ""
end
```

The server includes it with `include rules "rules/rules.salam"` and calls `rules.TitleProblem` in its handler; `client.salam` does the same and wires the function to the form with `dom.OnEventE`. Then:

```bash
salam build main.salam --output=notes-server
salam web index.salam --output=public/index.html
```

When we tried this layout, both builds compiled the same function: the server binary natively, the browser copy into the page's script. One caveat from that test: a helper pulled into a plain `page:` file with `include` was not carried into its generated script, so put browser code that uses shared packages in a separate `script: src` program built by `salam web`, as above.

Sharing also has a subtle benefit in Salam. Both the native build and the JavaScript build count a string's length in UTF-8 bytes, so a length rule gives the same answer on both sides. That is not true of every "isomorphic" setup; JavaScript itself counts UTF-16 code units.

## Deploying a full-stack Salam application {#deploy}

A Salam web application builds to one native executable with no runtime to install. The usual production shapes are:

- **The program behind a TLS proxy.** Run the server on a local port and put Caddy or nginx in front for HTTPS, or call `http.RunTLS` with a certificate and key. Static files can be served by `router.Static` from the same program.
- **`salam serve --config=web.salam`** for the static side. The web server in the `net.httpd` package handles virtual hosts, locations, redirects, caching and PHP through FastCGI. It has no TLS and no reverse proxy of its own in the current version, so the [salam serve lesson](/learn/serve/) pairs it with Caddy.
- **Cloudflare Workers.** A `net/http` and `net/router` program builds to WebAssembly with `--target=wasm32-wasi` and runs unchanged at the edge, with limits: one process per request, no file system, and no outbound sockets from the module.

And because the whole stack is one language, the same programs can be type-checked in CI with a single `salam check`, and their SQL strings with [`salam sql check`](/blog/typed-sql-database-apps/).

## What is still missing {#limits}

An honest comparison with a JavaScript or TypeScript full stack has to list what Salam does not have yet:

- **No client-side data fetching helper.** The `dom` package wraps the DOM, timers and storage but not `fetch`. Rich single-page applications that load JSON in the background are not the natural shape of a Salam app today; server-rendered pages with forms are.
- **No component ecosystem** like npm's. You write your components, or bridge to existing JavaScript through untyped `dom.Call` functions.
- **A young toolchain.** The page language, the JavaScript backend and the native UI runtime change between releases. Some language features behave differently on the JavaScript backend; strings counting bytes is one of them.
- **Small community and few production deployments.** Answers to unusual problems are less likely to exist already.

The comparison with the most popular "one language" stack is in [Salam vs TypeScript](/blog/salam-vs-typescript/).

## When the one-language approach fits {#fit}

Salam's full-stack story is strongest for server-rendered applications with focused interactivity: admin panels, internal dashboards, forms-heavy business tools, documentation and content sites, small SaaS back offices, and teaching projects where students should see the whole request cycle in one language. You get compiled performance on the server, typed pages that cannot inject markup, one toolchain and one set of types. For an application whose value is mostly in a large client-side interface, a JavaScript framework remains the more practical frontend today, and a Salam server behind it is still a reasonable backend.

## Frequently asked questions {#faq}

### Can Salam replace JavaScript in the browser?

For handlers, small interactive pages and components with state, yes: you write Salam and the compiler produces the JavaScript. For applications that need many third-party frontend libraries or background data fetching, not yet.

### Does Salam have a web framework?

The standard library is the framework: `net.http` and `net.router` for the server, the page language for HTML and CSS, `net.http.session`, `net.http.validate` and `net.http.middleware` for the usual concerns, and `db` for data. There is nothing extra to install.

### Is server-side rendering in Salam safe against XSS?

Values passed to a runtime layout are always HTML-escaped, URLs and CSS values must be parsed into safe types first, and the structure of the page is fixed at compile time. That closes the common injection paths, but security headers, CSRF tokens and sessions are still your job; `net.http.middleware` and `net.http.session` help with them.

### How do I start a full-stack Salam project?

Install the compiler from the [install page](/install/), work through [the layout lesson](/learn/layout/) and the [web projects lesson](/learn/web-projects/), then add a server with the examples on the [net.http page](/std/net/http/).

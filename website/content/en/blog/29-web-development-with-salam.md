---
title: Web development with Salam: a built-in language that generates HTML, CSS and JavaScript
seo_title: Web Development with Salam: A Checked HTML and CSS Language
description: Web development with Salam: a built-in page language that compiles to checked HTML, CSS and JavaScript, with components, events and server-side rendering.
keywords: web development with Salam, HTML CSS DSL, Salam page language, salam page build, generate HTML from code, compile-time checked HTML, server-side rendering, web components, right-to-left web pages, Persian web development
category: build
summary: Salam has a page language inside its compiler. You describe a page as a tree of elements, the compiler checks every element, attribute and CSS value, and you get HTML, CSS and JavaScript back. This article walks from a static page to components, events and pages rendered by a Salam server.
date: 2026-11-11
---

Web development with Salam starts with a feature most languages leave to templates and frameworks. **Salam's page language is a declarative language built into the compiler: you describe a web page as a tree of elements with attributes and styles, the compiler checks every element, attribute and CSS value against the HTML and CSS rules, and `salam page build` turns it into HTML, CSS and, when the page has event handlers, JavaScript.** The same language describes pages that a Salam web server renders for each request, and it works in English and in Persian.

This article covers the whole path: a static page, what the compiler checks, components, interactivity, server-side rendering, building and deploying, and the limits you should know before you choose it. Every program shown here is compiled when this site is built: the static pages with `salam page build`, the others with the normal compiler.

## A first page {#first-page}

A `page:` block is a tree. Each element opens with its name and a colon and closes with `end`; attributes and CSS properties are written inside it as `name = value` pairs, with no commas. Constants declared above the page can be used in any value, and a `component` is a reusable piece of page with parameters:

```salam
const BRAND := "#0f766e"

component Feature(name, text):
    box:
        padding = "16px"
        border = "1px solid #e2e8f0"
        border radius = "12px"
        heading: size = 3 content = name color = BRAND end
        paragraph: content = text end
    end
end

page:
    title = "Acme Notes"
    global: selector = "body" margin = "0" font family = "system-ui, sans-serif" end
    header:
        background = BRAND
        color = "white"
        padding = "24px"
        heading: size = 1 content = "Acme Notes" end
        nav:
            link: url = "/" content = "Home" color = "white" end
            link: url = "/pricing" content = "Pricing" color = "white" margin = "0 12px" end
        end
    end
    main:
        padding = "24px"
        display = "grid"
        grid template columns = "repeat(auto-fit, minmax(220px, 1fr))"
        gap = "16px"
        Feature: name = "Fast" text = "Opens in a blink." end
        Feature: name = "Private" text = "Your notes stay on your device." end
        Feature: name = "Bilingual" text = "Write in English or Persian." end
    end
    footer:
        padding = "24px"
        paragraph: content = "Built with Salam" end
    end
end
```

`salam page build site.salam` writes `site.html` and `site.css`. The HTML it produced for this page is:

```html
<!doctype html>
<html lang="en" dir="ltr">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Acme Notes</title>
    <link rel="stylesheet" href="site.css" />
  </head>
  <body>
    <header class="b">
      <h1>Acme Notes</h1>
      <nav>
        <a href="/" class="c">Home</a><a href="/pricing" class="d">Pricing</a>
      </nav>
    </header>
    <main class="e">
      <div class="f">
        <h3 class="g">Fast</h3>
        <p>Opens in a blink.</p>
      </div>
      <div class="f">
        <h3 class="g">Private</h3>
        <p>Your notes stay on your device.</p>
      </div>
      <div class="f">
        <h3 class="g">Bilingual</h3>
        <p>Write in English or Persian.</p>
      </div>
    </main>
    <footer class="h"><p>Built with Salam</p></footer>
  </body>
</html>
```

and the style sheet:

```css
body {
  margin: 0px;
  font-family: system-ui, sans-serif;
}
.b {
  background: #0f766e;
  color: white;
  padding: 24px;
}
.c {
  color: white;
}
.d {
  color: white;
  margin: 0px 12px;
}
.e {
  padding: 24px;
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 16px;
}
.f {
  padding: 16px;
  border: 1px solid #e2e8f0;
  border-radius: 12px;
}
.g {
  color: #0f766e;
}
.h {
  padding: 24px;
}
```

A few things happened on the way. `box` became a `div`, `link` became an `a`, and `url` became `href`. The styles did not stay on the elements: each styled element got a short generated class, and identical styles share one, which is why the three feature cards all use `.f`. The constant `BRAND` was folded into the CSS at compile time. With `--inline` the CSS goes into the HTML file, giving one self-contained page any static host can serve. The [layout lesson](/learn/layout/) introduces all of this step by step.

## Checked at compile time {#checked}

Templates in most web stacks produce text, and a typo in a tag or a CSS property shows up only in the browser, if anyone notices. Salam's page language knows every HTML element and every CSS property, and it checks them while compiling. Here is a page with four mistakes:

```salam fragment
page:
    title = "Broken"
    image: alt = "logo" end
    input: id = "q" hint = "search" end
    item: content = "orphan" end
    box: colour = "red" end
end
```

`salam page build` refuses it and writes no files:

```text
E005: element 'image' is missing required attribute 'url' (bad.salam:3:5)
E112: unknown attribute 'hint' on 'input'; did you mean 'enterkeyhint'? (bad.salam:4:21)
E004: element 'item' cannot be placed inside 'page' (bad.salam:5:5)
E112: unknown attribute 'colour' on 'box'; did you mean 'color'? (bad.salam:6:10)
```

The checks go further than names. Elements are checked against where they may appear (an `item` needs a `list`, a link cannot contain a link), required attributes must be present, IDs referenced by `for` must exist and must not repeat, and almost every CSS property validates its value as a length, color, time, angle or keyword, including shorthands such as `border` and `transition`. URL attributes reject `javascript:` and similar schemes. The full table of elements, with their Persian names and placement rules, is in [LAYOUT_ELEMENTS.md](https://github.com/SalamLang/Salam/blob/main/docs/LAYOUT_ELEMENTS.md).

## Readable names for HTML and CSS {#names}

The page language uses readable names where HTML is terse: `paragraph`, `heading` with a `size`, `list` and `item`, `ordered list`, `text area`, `block quote`. The HTML tag names work too (`div`, `p`, `a`, `img`, `li`), so you can write whichever you know. Some names depend on context: inside a `table`, `header`, `main` and `footer` become `thead`, `tbody` and `tfoot`, and `column` is a header cell in a header row and a data cell elsewhere. CSS properties are written with spaces instead of dashes (`border radius`, `font family`), and a prefix such as `hover` or `focus` applies a style in that state.

Lessons [25](/learn/page/text/) to [30](/learn/page/head/) teach the elements group by group: text, page structure, media, tables, forms, and the document head with scripts and global attributes. Lessons [31](/learn/css/basics/) to [38](/learn/css/more/) do the same for CSS, from the box model through [layout with flex and grid](/learn/css/layout/), [responsive pages](/learn/css/responsive/) and [animation](/learn/css/motion/).

Every name also has a Persian spelling, and a page written in Persian is right to left with `lang="fa"` without you saying so. This is the same kind of page in Persian:

```salam
بخش ویژگی(نام، شرح):
    جعبه:
        فاصله داخلی = "16px"
        کادر = "1px solid #e2e8f0"
        گردی کادر = "12px"
        سرتیتر: اندازه = ۳ محتوا = نام پایان
        پاراگراف: محتوا = شرح پایان
    پایان
پایان

صفحه:
    عنوان = "یادداشت‌های من"
    سرتیتر: اندازه = ۱ محتوا = "یادداشت‌های من" پایان
    ویژگی: نام = "سریع" شرح = "در یک چشم به هم زدن باز می‌شود." پایان
    ویژگی: نام = "دوزبانه" شرح = "به فارسی یا انگلیسی بنویسید." پایان
پایان
```

It builds to `<html lang="fa" dir="rtl">` with the same generated classes. Logical CSS properties such as `border inline start` follow the direction of the page, so most styles need no change between the two directions. The article [Right-to-left programming](/blog/right-to-left-programming-unicode/) covers bidirectional text in more depth.

## Components and shared parts {#components}

A `component` is defined once and used like a new element. Its parameters are set like attributes, and a `content: end` line inside it marks a slot where whatever you nest at the use site lands. To share components across pages, put them in their own file and write `include = "parts.salam"` at the top of the page.

Components keep the generated CSS small, because two uses with identical styles share a class. They also bring one rule worth knowing: if two elements carry the same `class` but different styles, the compiler stops with error E136 rather than letting one silently override the other. Leave `class` off styled elements, or style a shared class once with a `global` rule.

## Making pages interactive {#interactive}

An attribute named after a browser event, such as `click`, `input`, `submit` or `key down`, takes a Salam function or an inline lambda instead of a value. The handler is compiled through Salam's JavaScript backend into the page's script, so the behavior is written in Salam, not in hand-written JavaScript. The `dom` package gives handlers access to the page (`dom.ById`, `dom.SetText`, `dom.SetStyle` and many more), and IDs written as plain text in `dom.ById("...")` are checked against the IDs in the same page at compile time.

Components can also hold state. A variable declared at the top of a component body belongs to each instance; when a handler changes it, the compiler re-renders only the instances that read it:

```salam
component Choice(name: str):
    votes := 0
    box:
        button: click = (): votes++ end content = "Vote for " + name end
        span: content = " " + votes end
    end
end

page:
    title = "Quick poll"
    heading: size = 2 content = "Tea or coffee?" end
    Choice: name = "Tea" end
    Choice: name = "Coffee" end
end
```

Each `Choice` counts its own votes, and clicking one redraws only that box, so focus and typed input elsewhere survive. The same page can run in a native window drawn by Salam with `salam run poll.salam --ui=native`, and `salam check --ui=native` reports the parts a native window cannot show. Lessons [39](/learn/events/mouse/) to [43](/learn/events/dom/) cover every event and the state model, and the article [Interactive user interfaces in Salam](/blog/interactive-ui-components-state-events/) goes deeper into components, state and events.

## Pages with parameters: server-side rendering {#server-rendering}

A `page:` block is fixed when you compile it. Give the page a name and typed parameters, and it becomes a function that returns the whole document as a string of HTML. The structure and the CSS are still worked out at compile time; at runtime the function only fills in values, and every value is HTML-escaped:

```salam
page Profile(name: str, skills: Vector<str>, admin: bool):
    title = name + " | Profile"
    heading: size = 1 content = name end
    if admin:
        paragraph: content = "Administrator" end
    end
    list:
        each i, skill in skills:
            item: content = (i + 1) + ". " + skill end
        end
    end
end

func main:
    mut skills := Vector {} as Vector<str>
    defer skills.Free()
    skills.Push("Salam")
    skills.Push("SQL & <HTML>")
    println Profile("Sara", skills, true)
end
```

```output
<!doctype html><html lang="en" dir="ltr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>Sara | Profile</title></head><body><h1>Sara</h1><p>Administrator</p><ul><li>1. Salam</li><li>2. SQL &amp; &lt;HTML&gt;</li></ul></body></html>
```

There is no template interpreter at runtime, and no raw-HTML type to forget to escape. The layout function is `pure`: it cannot read files, call the network or change globals, so the handler gathers the data and passes it in. A value that goes into a URL or a CSS property cannot be a plain string; it must be parsed first into a `web.Url`, `web.Color` or `web.Length`, which reject unsafe input. Because the page is a function, a Salam web server can call it for each request:

```salam
import net.http
import net.router

page Greeting(name: str):
    title = "Hello " + name
    heading: size = 1 content = "Hello, " + name end
    paragraph: content = "Rendered by a Salam server." end
end

func home(ctx: i64):
    mut name := http.Query(ctx, "name")
    if name == "":
        name = "guest"
    end
    http.Html(ctx, Greeting(name))
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/", home)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

The server runs until it is stopped, so this one is type-checked rather than run. Opening `http://localhost:8080/?name=Ada` returns the greeting, and a visitor who sends `?name=<b>` sees the characters, not bold text. The server-side is covered in [Building fast web servers with Salam](/blog/fast-web-servers-with-salam/), and [Full-stack development with Salam](/blog/full-stack-development-with-salam/) puts the two halves together.

## Building, serving and deploying {#deploy}

The commands are few:

```bash
salam page build site.salam            # site.html + site.css (+ site.js with handlers)
salam page build site.salam --inline   # one self-contained HTML file
salam page build home.salam about.salam  # one HTML file per page, one shared style.css
salam web app.salam                    # a page plus the Salam scripts it uses
salam serve                            # serve the current folder over HTTP
```

A static page is complete as it is: copy the HTML and CSS to any web host. A site that needs a server is an ordinary Salam program built with `salam build` into one executable. For hosting several sites, redirects and FastCGI backends such as PHP, `salam serve --config` runs a web server described in a Salam configuration file ([the serve lesson](/learn/serve/)). A `net/http` program can also be built to WebAssembly and deployed to Cloudflare Workers.

## Limits and when to use something else {#limits}

The page language is young, and it is not a replacement for every web stack:

- **No framework ecosystem.** There are no third-party component libraries, design systems or routing libraries for it. For a large single-page application with complex client state, a TypeScript framework is the practical choice (see [Salam vs TypeScript](/blog/salam-vs-typescript/)).
- **The structure of a server-rendered page is fixed at compile time.** `if` and `each` choose and repeat parts, but you cannot build arbitrary HTML from strings, by design.
- **Client-side state needs JavaScript.** Components with state are drawn by the page script, so they appear only with JavaScript enabled; static content does not depend on it.
- **The JavaScript backend has differences from native Salam.** For example, an `i64` keeps only 53 bits of precision in the browser, because JavaScript numbers are doubles.
- **Escape hatches exist but are unchecked.** A `style` or `script` element can hold raw CSS or JavaScript, and a handler can be a plain JavaScript string, but the compiler cannot check what is inside them.

Where it fits well: documentation sites, landing pages, dashboards, forms, admin pages and server-rendered applications where correctness of the markup and safe escaping matter more than client-side richness.

## A learning path {#learn}

The free course on this site teaches the page language in twenty-one lessons after the general language:

1. [The layout language](/learn/layout/): the tour, from a first page to server rendering.
2. [Pages and text](/learn/page/text/), [page structure](/learn/page/structure/), [media](/learn/page/media/), [tables](/learn/page/tables/), [forms](/learn/page/forms/) and [the head](/learn/page/head/): every element.
3. [Styles and the box model](/learn/css/basics/) through [more CSS](/learn/css/more/): every property.
4. [Mouse events](/learn/events/mouse/) through [state, the DOM and timers](/learn/events/dom/): interactivity.
5. [Six whole web projects](/learn/web-projects/): a landing page, a blog post, a sign-up form, a dashboard, a task list and a gallery.

## Frequently asked questions {#faq}

### Is Salam's page language a template engine?

No. A template engine fills text into text at runtime. Salam's page language is part of the compiler: it knows HTML and CSS, checks the page while compiling, and turns a page with parameters into a pure function that only fills in escaped values.

### Do I need to know HTML and CSS to use it?

It helps, because the page language maps directly onto HTML elements and CSS properties and everything you know about them still applies. The course teaches both from zero through the page language, so you can learn them together.

### Can I build a single-page application with Salam?

You can build interactive pages with components, state and events, compiled to JavaScript. For large applications with complex client-side routing and state, the ecosystems around React, Vue or Svelte are more mature.

### Can I write web pages in Persian with Salam?

Yes. Every element, attribute and CSS property has a Persian name, a page written in Persian is right to left with `lang="fa"` automatically, and Persian digits work in values.

---
title: Interactive user interfaces in Salam: components, state and event handling in the page language
seo_title: UI Components, State and Events in Salam | Salam Magazine
description: How Salam's page language builds interactive UI components: event handling in Salam, reactive state management, callbacks and events, checked at compile time.
keywords: UI components, state management, event handling, reactive UI, Salam page language, component state, compile-time UI checks, web components in Salam, native UI, declarative UI
category: build
summary: A practical guide to interactivity in Salam pages: handlers written in Salam, components with their own state, lists with keys, callbacks, events that bubble, and the compile-time checks around them.
date: 2026-11-11
---

Salam's page language handles UI components, state management and event handling without a separate front-end framework. You declare the page as a tree of elements, write event handlers as ordinary Salam functions, and the compiler turns the whole thing into HTML, CSS and JavaScript. **A component in Salam is a named piece of page with parameters; when it declares a variable that a handler changes and the view shows, that variable becomes state, and the page redraws only what depends on it.** The same page can also run in a native window drawn by Salam itself.

This article walks through the model step by step, from a single click handler to components that keep their own state and talk to each other, and ends with the checks the compiler runs and the limits you should know about. Every program below is a complete page that builds with `salam page build`.

## How interactivity works in a Salam page {#model}

A Salam page is a `page:` block. Elements open with a colon and close with `end`, and attributes and CSS properties are written as `name = value` inside them. That is enough for static HTML and CSS; the article [Web development with Salam](/blog/web-development-with-salam/) covers that part.

Interactivity enters through attributes named after browser events. `click`, `submit`, `input`, `change`, `key down` and the rest take a function instead of a value. A plain function name, or an inline function written in place, is compiled by Salam's JavaScript backend and placed in the page's own script. You never write JavaScript by hand, and the handler is type-checked like any other Salam code.

There are two ways to make a page change after an event, and Salam supports both:

- **The direct way.** A handler finds an element with the `dom` package and changes its text, class, style or attributes. This is the familiar DOM style, and the lessons on [mouse events](/learn/events-mouse/), [keyboard and forms](/learn/events-keys-forms/), [drag and touch](/learn/events-drag-touch/) and [media events](/learn/events-media-more/) use it to cover each event family.
- **The reactive way.** A handler only changes variables. The compiler knows which parts of the page read those variables and redraws exactly those parts. This is how components with state work, and it is the only style that also runs in a native window.

The rest of the article starts with the first and moves to the second.

## Event handling with the dom package {#events}

Here is the smallest useful interactive page: a number and two buttons. The state is one `mut` variable above the page. Each handler changes it and calls `show`, the one function that writes to the page.

```salam
import dom

mut count := 0

func show:
    dom.SetText(dom.ById("value"), "" + count)
end

func more:
    count++
    show()
end

func start_over:
    count = 0
    show()
end

page:
    title = "Counter"
    box:
        heading: id = "value" content = "0" end
        button: click = more content = "+1" end
        button: click = start_over content = "Reset" end
    end
end
```

Three details matter. First, `click = more` names the function; it does not call it. Writing `click = more()` is rejected with error E137, because the attribute wants the function itself. Second, `dom.ById("value")` is checked against the ids that really exist in the same file, so a typo such as `dom.ById("valeu")` stops the build with E138 before the page ever reaches a browser. Third, the page has no `main`. A page program is driven entirely by its handlers, and adding a `main` makes the compiler report the handlers as unused functions.

The `dom` package covers what a handler normally needs: finding elements (`ById`, `Query`, `QueryAll`), reading and writing text and values, classes and styles, creating and removing elements, timers (`SetTimeout`, `SetInterval`, `RequestFrame`), the event itself (`EventTarget`, `EventKey`, `PreventDefault`), and browser storage (`StoreSetStr`, `StoreGetStr`). The full list is on the [dom package page](/std/dom/), and the lesson on [state, the DOM and timers](/learn/events-dom/) builds a todo list, tabs, a stopwatch and a quiz with it.

The direct style works well for small pages. Its weakness is the one every jQuery-era developer remembers: as soon as two handlers change the same element, you have to keep the variables and the page in step by hand. Salam's lesson recommends a discipline for that (state in variables, one drawing function), and the reactive model below removes the need for it.

## Components: reusable pieces of page {#components}

A `component` is a piece of page you define once and use like a new element. Its parameters are set like attributes. Inside it, `content: end` marks a slot: whatever you nest inside the component where you use it lands there.

```salam
component Plan(name: str, price: int):
    box:
        border = "1px solid #ddd"
        border radius = 12
        padding = 16
        heading: size = 2 content = name end
        paragraph: content = "$" + price + " a month" end
        content: end
    end
end

page:
    title = "Plans"
    Plan: name = "Basic" price = 4 end
    Plan: name = "Pro" price = 9
        list:
            item: content = "Private projects" end
            item: content = "Email support" end
        end
    end
end
```

Both plans share one generated CSS class, because their styles are identical. Components can live in a file of their own and be pulled in with `include = "parts.salam"` at the top of a page, which is how a site shares a header or a card between pages. In Persian the keyword is `بخش`, and every element and attribute has a Persian name too; the article [One language, two scripts](/blog/one-language-two-scripts/) explains how both spellings compile to the same thing.

A component with typed parameters is also a function. Called from ordinary Salam code, `Plan("Pro", 9)` returns its HTML and CSS as a string, which is how a server can render the same piece of UI per request.

## State inside components {#state}

A component becomes interactive when it declares a variable at the top of its body. There is no `useState`, no signal constructor and no keyword. The compiler looks at how each variable is used and classifies it:

| Written by some code? | Shown by the view? | What it becomes |
|---|---|---|
| yes | yes | **state**: kept per copy, and the copy redraws when it changes |
| yes | no | an instance variable: kept per copy, never causes a redraw |
| no | either | a derived value, worked out again every time the copy draws |

"Written" means an assignment, `++`, a method such as `push`, or a change made inside a handler or a callback. "Shown" means it appears in an attribute, a text, an `if` condition, an `each` collection or a value passed to a child component.

```salam
component Choice(name: str):
    votes := 0
    label := "Vote for " + name
    box:
        class = "choice"
        button: click = (): votes++ end content = label end
        span: content = "" + votes end
        if votes >= 3:
            span: class = "hot" content = "popular" end
        end
    end
end

page:
    title = "Poll"
    heading: size = 2 content = "Your favourite drink?" end
    Choice: name = "Tea" end
    Choice: name = "Coffee" end
end
```

`votes` is written by the click handler and shown in the `span` and the `if`, so it is state: each `Choice` has its own count, and clicking one redraws only that copy. `label` is never written, so it is derived from `name`. Nothing else on the page is touched, which means focus stays on the button and text typed into an input of another copy stays where it is.

A written variable needs a type the compiler can see: a literal, a typed parameter, or `value as Type`, as in `items := Vector {} as Vector<str>`. The `page:` block can declare variables the same way, and the page may also read top-level `mut` variables; every element, `if` or `each` that reads one is drawn again after a handler changes it.

## Lists, keys and conditional rendering {#lists}

`if` and `each` inside a page are where a declarative UI pays off. The next page keeps a list of tasks in a `mut` vector, shows a message while it is empty, and renders one `Task` component per entry. Each task has its own `done` state.

```salam
mut tasks := Vector {} as Vector<str>
mut next := 1

component Task(name: str):
    done := false
    item:
        key = name
        span: content = name end
        button: click = (): done = not done end content = done ? "Undo" : "Done" end
    end
end

page:
    title = "Tasks"
    button:
        content = "Add a task"
        click = ():
            tasks.push("Task " + next)
            next++
        end
    end
    if tasks.len() == 0:
        paragraph: content = "Nothing to do yet" end
    end
    list:
        each t in tasks:
            Task: name = t end
        end
    end
    paragraph: content = "" + tasks.len() + " tasks" end
end
```

The `key = name` line is the same idea as a key in React: when rows are inserted, removed or reordered, each component copy follows its key and keeps its own state, instead of being matched by position. Without a key, copies follow their position in the list. A fixed `id` inside `each` would repeat, so the compiler rejects it (E134); use a key, a class, or an id built from the row.

## Components talking to each other: callbacks and events {#communication}

A child component often has to tell its parent that something happened. Salam offers two mechanisms, and both are resolved at compile time.

The first is a **callback**: the parent passes a function as a parameter and the child calls it from a handler. The lambda is written in the parent, so it changes the parent's state. A child whose parent passed nothing simply skips the call.

The second is an **event that bubbles**. The child writes `emit("added", price)`; the place where the component is used handles it with `on added = (p: int): ... end`. The event goes to the nearest handler and then to every level above it, up to an `on added` line at the top of the page block, until a handler calls `stop()`.

```salam
mut total := 0

component Stepper(label: str, change):
    count := 0
    box:
        span: content = label + ": " + count end
        button:
            content = "+1"
            click = ():
                count++
                change(1)
            end
        end
    end
end

component Product(name: str, price: int):
    box:
        span: content = name + ", " + price + " coins" end
        button: click = (): emit("added", price) end content = "Add to basket" end
    end
end

page:
    title = "Basket"
    on added = (p: int): total += p end
    heading: size = 2 content = "Basket: " + total + " coins" end
    Product: name = "Tea" price = 3 end
    Product: name = "Cake" price = 5 end
    Stepper: label = "Extra cups" change = (n: int): total += n end end
end
```

Because the compiler works out the route of every event while it builds the page, an `emit` costs no more than calling the handlers yourself, and only the components whose state changed are drawn again. In Persian, `emit`, `on` and `stop` are `برانگیز`, `هنگام` and `بایست`.

## Checked at compile time {#checks}

A UI that is wrong in the browser is usually wrong silently: a misspelled id returns `null`, a handler is never attached, a style is ignored. Salam moves as many of those mistakes as it can to the build. A few of the checks that apply to interactive pages:

| Code | What it catches |
|---|---|
| E137 | an event attribute given an expression such as `more()` instead of a function |
| E138 | `dom.ById("x")` where no element in the file has the id `x` |
| E111 / E112 | an unknown element or attribute, with a "did you mean" suggestion |
| E133 | `if` or `each` in a static layout, where no runtime value exists |
| E134 | a fixed `id` inside `each`, which would repeat |
| E136 | one class given different styles on two elements |
| E018 | a CSS value of the wrong type, such as a color where a length belongs |

Here is what the first two look like on a page that calls its handler by mistake and looks up an id that does not exist:

```bash
$ salam page build counter.salam
E137: event attribute 'click' must be a function name, an inline function, or a JS string, not an expression (counter.salam:9:21)
E138: no element with id 'counter' in this file's layout (counter.salam:4:26)
page build failed with 2 errors; no output written
```

The id check has a limit worth knowing: it only sees ids written as plain text. `dom.ById("votes-" + name)` is built at run time and cannot be checked, so a typo there still shows up in the browser console. Handlers themselves are Salam code, so a wrong type, an unknown function or a missing argument is an ordinary compile error, exactly as in a server program.

## One page, two targets: the web and a native window {#native}

The reactive model has a second payoff. A page whose handlers only change state does not depend on the browser, so Salam can run it in a native window:

```bash
salam page build tasks.salam              # tasks.html, tasks.css, tasks.js
salam run tasks.salam --ui=native         # the same page in a native window
salam build tasks.salam --ui=native       # a native executable
salam check tasks.salam --ui=native       # report what a window cannot show
```

The native runtime is the `gui.ui` package, built on Salam's own CSS engine, box layout, text shaping and painter, so no browser engine is shipped or required. After every handler it runs the view again and reconciles the result with what is on screen, so hover, focus, scroll positions and typed text survive. The project's design notes compare the approach to Svelte and Solid: the compiler finds what each part of the page reads, marks every write, and only the parts whose state changed run again.

Not every element makes sense in a window. `salam check --ui=native` reports E165 for things that need a browser, such as `script`, `iframe` and `svg`, and warning W166 for web metadata a window ignores. Wrapping those parts in `if SALAM_UI_WEB:` keeps one source file for both targets. The direct `dom` style from the first example is browser-only by nature. The article on [desktop and GUI applications](/blog/desktop-gui-apps-with-salam/) goes further into windows, WebView apps and installers.

## How it compares, and the honest limits {#limits}

If you know React, Vue or Svelte, the concepts map closely: components with parameters (props), local state, keyed lists, callbacks passed down, events passed up. The differences are in where the work happens. Salam has no hooks and no explicit signals; the compiler decides what is state from how a variable is used. There is no separate build tool, bundler or package of UI code to install: the page, its CSS and its handlers come out of one compiler. And the same type checker that checks your server code checks your handlers.

The limits are real, and you should weigh them:

- **No component ecosystem.** There is nothing like npm's thousands of ready-made date pickers, data grids and chart libraries. You build components yourself or wrap existing JavaScript through `dom.Global` and `dom.Call...`, which is untyped.
- **Stateful parts need JavaScript on the web.** Component copies and parts that read `mut` variables are drawn by the page script, so they appear only with JavaScript enabled. Static parts are plain HTML.
- **Strings count bytes in the browser.** A Salam `str` compiled to JavaScript counts UTF-8 bytes, so the Persian word `سلام` has a length of 8. For what a reader sees, read the browser's own properties such as `textLength`.
- **No built-in fetch helper.** The `dom` package does not wrap `fetch`, so a page talks to a server through forms and server-rendered pages rather than client-side API calls; see [full-stack development with Salam](/blog/full-stack-development-with-salam/).
- **A young language.** The page language and the native runtime are recent and still changing between releases.

For a learning project, an internal tool, a landing page with a few interactive pieces or a small app that should also run as a native window, the model is pleasant and the compile-time checks catch real mistakes. For a large client-heavy single-page application that depends on a rich component library, a mainstream JavaScript framework is still the practical choice today.

## Frequently asked questions {#faq}

### Does Salam have a virtual DOM like React?

On the web, each stateful component copy renders itself in the page script, and after a handler only the copies whose state changed are rendered again and patched into the existing elements in place. The compiler knows at build time which parts read which variables, so a click re-renders one component copy rather than the whole page.

### How do I manage global state in a Salam page?

Use top-level `mut` variables above the `page:` block, or variables declared at the top of the page block. Every element, `if` or `each` that reads one is redrawn after a handler changes it. Pass values down to components as parameters, and send changes up with callbacks or `emit`.

### Can I use Salam components with an existing JavaScript app?

A Salam page compiles to ordinary HTML, CSS and JavaScript, so it can sit next to other scripts. Calling into existing JavaScript from a handler goes through `dom.Global` and the `dom.Call` functions, or a plain string handler such as `click = "alert(42)"`.

### Where can I learn the page language step by step?

The free [Salam course](/learn/) has a full part on web pages, from [the first layout](/learn/layout/) and [page structure](/learn/page-structure/) through CSS to the event lessons and [six complete web projects](/learn/web-projects/), each with live previews.

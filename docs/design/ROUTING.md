# File routes: the pages folder is the route table

Status: implemented (2026-11-22). Replaced the `routes:` block of
`UI_BEHAVIOUR.md` section 7 and of #3757, #3791 and #3801.

## Why

The `routes:` block is a second small language inside the page language: a
table of quoted patterns, with `guard` and `title` settings on each
line. A folder already is a tree of paths, and every project has one. Making the
folder the table adds no syntax, keeps one place to look, and gives guards and
layouts a natural scope (a folder).

## The tree

```
main.salam
pages/
  index.salam                 /
  about.salam                 /about
  login.salam                 /login
  docs/
    index.salam               /docs
    [topic].salam             /docs/:topic
    archive/
      [[year]].salam          /docs/archive/:year?
  user/
    [id].salam                /user/:id
  admin/
    _layout.salam             wraps every page below /admin
    index.salam               /admin
    users.salam               /admin/users
  _404.salam                  *  (any other path)
```

| file name | pattern segment |
| --- | --- |
| `index.salam` | the folder itself |
| `name.salam` | `name` |
| `[id].salam` | `:id` |
| `[[id]].salam` | `:id?` (optional, as `/docs/:topic?` today) |
| `[...rest].salam` | trailing `*rest` (the component parameter `rest` receives the remaining path) |
| `_404.salam` | `*` fallback |
| `_layout.salam` | not a route; wraps the folder (see below) |

Segment names are ASCII letters, digits, `-` and `_`. A Persian URL uses the
`route` setting (below), the file name stays ASCII.

## A route file

A route file declares exactly one `pub component`. Its parameters are the path
parameters, typed from the component, as today (`str` and `int`, defaults make
a parameter optional).

```salam
pub component User(id: int):
    title = "User " + id
    guard = signed_in()
    heading: content = "User " + id end
    Link: to = "/" content = "Back" end
end
```

The settings sit at the top of the component body and are the same three the
route line took: `title`, `guard` (a bool expression, re-checked when its state
changes). A route whose guard is false is skipped, so the next matching route
shows (usually `_404`). There is no `redirect` setting: code that wants to send
the visitor elsewhere calls `page.route.Redirect(path)`. One more is new: `route`,
a text that replaces the path derived from the file name (`route = "/کاربر/:id"`).

## Layouts

`_layout.salam` declares one `pub component` with an `Outlet`. It wraps every
route file in its folder and below, nested from the outside in. Its `guard`
applies to the whole subtree, so a section is protected once.

```salam
pub component AdminLayout:
    guard = signed_in()
    nav: Link: to = "/admin/users" content = "Users" end end
    Outlet: end
end
```

## What the compiler does

1. Finds `pages/` next to the entry file (`main.salam`). No folder, no routes,
   and `Outlet`, `Link`, `page.route.GoTo` keep their current meaning only when a folder
   exists.
2. Walks it in sorted order and builds the same `LoRoute` table that
   `compiler/sal_layout/layout_routes.salam` builds from the block today
   (pattern, component, guard, title, rank). Everything after that,
   matching with `nav.Match`, ranking with `nav.Rank`, history and hash modes,
   the native stack and prerender per static route, is unchanged.
3. Project wide settings that were `mode` and `base` inside the block move to
   the entry page: `page: mode = hash base = "/app" ... end`.

Two files that can match the same path with the same rank are E280. A route that
is fully shadowed by a more specific one (same match set) is reported as E280 too,
so every file in the tree is reachable.

## Diagnostics

| code | kind | message |
| --- | --- | --- |
| E280 | error | `two files give the same route: 'pages/a.salam' and 'pages/a/index.salam'` |
| E281 | error | `file 'pages/user/[id].salam' has no parameter 'id' in its component` |
| E282 | error | `route file 'pages/x.salam' must declare exactly one pub component` |
| E283 | error | `'_layout.salam' in 'pages/admin' has no Outlet` |
| E284 | error | `'pages/a b.salam' is not a valid route file name` |

The router codes E264 to E267 and E263 keep their meaning where a check still
applies (parameter types, optional defaults, unknown setting); E265 (`Outlet`
or `page.route.GoTo` without routes) now says "without a pages folder". They were
E256 to E259 until those numbers were found to belong to the ownership checks.

## Project components

Convention: `components/` holds the `pub` components of a page project,
`pages/` holds the routes, and everything else is included explicitly.

```
shop/
  main.salam
  pages/
    index.salam
    product/[id].salam
  components/
    card.salam
    shop/badge.salam
  lib/
    format.salam
```

In a project (entry named `main.salam`) the compiler looks for a folder named
`components/` next to it. If it exists, every `.salam` file in it, in sorted
order and recursively, is read. Files without a component are ignored. Every
`pub component` of the others is imported without `include`; a component
without `pub` stays private to its file (E232). Hidden folders,
`node_modules`, `build`, `dist`, `out`, `target`, `vendor`, `tests`, `test`,
`__tests__`, folders with their own `main.salam`, `*_test.salam` and
`*.test.salam` files are not read. Folders nested deeper than 12 levels are
not followed and a folder reached twice through links is read once, which ends
symlink loops.

Without a `components/` folder nothing is imported by name. A file anywhere
else (`lib/`, `ui/`, next to `main.salam`) is used only through an explicit
`include = "ui/card.salam"`. Using a component that lives there without
including it is the usual unknown element error (E111), with the hint
`a pub component 'Card' exists in the project outside components/; move it to
components/ or include it`. `pages/` is route files only.

The duplicate check is `E291`; it covers `components/`, `main.salam` and the
route files together, so a `pub component` in `pages/about.salam` and another
`About` in `components/` clash:
`two pub components are named 'Card': 'components/shop/card.salam' line 13 and 'components/ui/card.salam' line 13`.
A single file build, the playground and runtime layouts behave as before.

## Migration

The `routes:` block is gone: it is no longer parsed, so a program that still
writes it gets the ordinary syntax error. There is no warning release and no
migration tool; each route line becomes a file by hand: the pattern is the
filename, `title` and `guard` move into the component body (a `redirect`
becomes a `page.route.Redirect` call in code), and
`mode` and `base` move to the entry `page:` block. The tests, docs and learn
pages that used the block were converted to folders.

## Targets

- **web**: unchanged from #3757: History API or hash mode, a page per static
  route, a fallback `index.html` for parameter routes. With SSR (`SERVER.md`)
  the server answers every route itself.
- **native**: the folder is compiled into the in binary table; `page.route.GoTo`,
  `page.route.Back` and `page.route.Redirect` use the screen stack. There is no folder at
  run time.

## Decisions

| # | decision |
| --- | --- |
| FR1 | the `pages/` folder is the table; no route syntax in source |
| FR2 | `[id]`, `[[id]]`, `[...rest]`, `index`, `_layout`, `_404` |
| FR3 | one `pub component` per route file; its parameters are the path parameters |
| FR4 | `title`, `guard`, `route` are component settings; a layout's guard applies to its subtree; no `redirect` setting, code calls `page.route.Redirect` |
| FR5 | lowering reuses the existing route table, matcher and runtime |
| FR6 | the `routes:` block is removed outright |

## Implementation steps

1. Collect `pages/` into the route list in `layout_routes.salam`, E280 to E284 (done).
2. `route`, `guard`, `title` as component settings, layouts nested (done).
3. Convert the tests and docs, remove the block (done).

# File routes: the pages folder is the route table

Status: phase 1 implemented (2026-11-22): the pages folder works and the
`routes:` block still works. Replaces the `routes:` block of
`UI_BEHAVIOUR.md` section 7 and of #3757, #3791 and #3801.

## Why

The `routes:` block is a second small language inside the page language: a
table of quoted patterns, with `guard`, `redirect` and `title` settings on each
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
    [[section]].salam         /docs/:section?
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
| `[...rest].salam` | trailing `*` |
| `_404.salam` | `*` fallback (in a subfolder, `/folder/*`) |
| `_layout.salam` | not a route; wraps the folder (see below) |

Segment names are ASCII letters, digits, `-` and `_`. A Persian URL uses the
`route` setting (below), the file name stays ASCII. Folder names follow the
same rules and may be `[id]` or `[[id]]`; only a file can be `[...rest]`.
Files that do not end in `.salam` and names that start with `.` are ignored.

## A route file

A route file declares exactly one `pub component`. Its parameters are the path
parameters, typed from the component, as today (`str` and `int`, defaults make
a parameter optional).

```salam
pub component User(id: int):
    title = "User " + id
    guard = signed_in()
    redirect = "/login"
    heading: content = "User " + id end
    Link: to = "/" content = "Back" end
end
```

The settings sit at the top of the component body and are the same three the
route line took: `title`, `guard` (a bool expression, re-checked when its state
changes) and `redirect` (where a failed guard goes). One more is new: `route`,
a text that replaces the whole path derived from the file name
(`route = "/کاربر/:id"`). The settings are removed from the component before it
is expanded. A `title` may use the path parameters (`"User " + id`); `guard` and
`redirect` may use globals and functions of `main.salam`, not the parameters.

Persian words: `عنوان` (title), `نگهبان` (guard), `هدایت` (redirect), `مسیر`
(route), and on the entry page `حالت` (mode) and `پایه` (base).

## Layouts

`_layout.salam` declares one `pub component` with an `Outlet`. It wraps every
route file in its folder and below, nested from the outside in. Its `guard` and
`redirect` apply to the whole subtree, so a section is protected once.

```salam
pub component AdminLayout:
    guard = signed_in()
    redirect = "/login"
    nav: Link: to = "/admin/users" content = "Users" end end
    Outlet: end
end
```

## What the compiler does

1. Finds `pages/` next to the entry file. Only an entry file named
   `main.salam` with a `page:` block looks for the folder, so a lone page file
   next to a folder called `pages` is not affected. No folder and no `routes:`
   block, no routes, and `Outlet`, `Link`, `go` keep their current meaning only
   when a folder (or a block) exists. If both exist the two tables are merged and
   an equal pattern is E280.
2. Walks it in sorted order and builds the same `LoRoute` table that
   `compiler/sal_layout/layout_routes.salam` builds from the block today
   (pattern, component, guard, redirect, title, rank). Everything after that,
   matching with `nav.Match`, ranking with `nav.Rank`, history and hash modes,
   the native stack and prerender per static route, is unchanged.
   The route files are parsed once by the lowering and then reach the build
   through the same `include` path as `include = "x.salam"` on the page, so each
   file keeps its own private components. They are never separate packages and
   never separate pages: `[id]` is only a file name, and `resolve_dir_layout`
   (which does not recurse) and `salam page build` look at `main.salam` alone.
   Two route files must not declare public components with the same name.
3. Project wide settings that were `mode` and `base` inside the block move to
   the entry page: `page: mode = hash base = "/app" ... end`. The compiler reads
   them from the page and removes them before the page attributes are checked.

## Diagnostics

| code | kind | message |
| --- | --- | --- |
| E280 | error | `two files give the same route: 'pages/a.salam' and 'pages/a/index.salam'` |
| E281 | error | `file 'pages/user/[id].salam' has no parameter 'id' in its component` |
| E282 | error | `route file 'pages/x.salam' must declare exactly one pub component` |
| E283 | error | `'_layout.salam' in 'pages/admin' has no Outlet` |
| E284 | error | `'pages/a b.salam' is not a valid route file name` |
| W281 | warning | `the 'routes:' block is replaced by the pages folder; run 'salam fix'` (not emitted yet: `salam fix` does not exist) |

The earlier E256 to E259 and E263 keep their meaning where a check still
applies (parameter types, optional defaults, unknown setting); E257 (`Outlet`
without routes) now says "no routes block and no pages folder". A setting other
than `guard` and `redirect` in a `_layout.salam` is E256. An error in a route file
is reported against that file. E281 does not apply when `route` is given.

## Migration

1. Release N: `pages/` works, `routes:` still works and warns (W281).
   `salam fix` rewrites a block into files (one `pub component` per line,
   settings moved into the body, `mode` and `base` moved to the entry page).
2. Release N+1: the `routes:` block is removed from the parser; the tests and
   docs that use it (`tests/en/page/pages/routes_*.salam`, `tests/en/ui/routes.salam`,
   `tests/en/page/projects/shop.salam`, `notes.salam`, `docs/design/UI_BEHAVIOUR.md`
   section 7, `website/content/{en,fa}/learn/46-web-projects.txt`) are moved to
   folders.

## Targets

- **web**: unchanged from #3757: History API or hash mode, a page per static
  route, a fallback `index.html` for parameter routes. With SSR (`SERVER.md`)
  the server answers every route itself.
- **native**: the folder is compiled into the in binary table; `go`, `go_back`
  and `replace` use the screen stack. There is no folder at run time.

## Decisions

| # | decision |
| --- | --- |
| FR1 | the `pages/` folder is the table; no route syntax in source |
| FR2 | `[id]`, `[[id]]`, `[...rest]`, `index`, `_layout`, `_404` |
| FR3 | one `pub component` per route file; its parameters are the path parameters |
| FR4 | `title`, `guard`, `redirect`, `route` are component settings; a layout's apply to its subtree |
| FR5 | lowering reuses the existing route table, matcher and runtime |
| FR6 | the `routes:` block warns for one release, then goes |

## First implementation steps

1. Done: collect `pages/` into the route list (`layout_pages.salam`), E280 to E284.
2. Done: `route`, `guard`, `redirect`, `title` as component settings, layouts nested
   (a layout `Outlet` becomes the slot of the layout component, the table shows
   `Layout: Inner: Page: ... end end end`; the guards of every layout above a route
   are checked outside in, then the route's own).
3. Convert the tests; `salam fix` migration; W281.
4. Remove the block.

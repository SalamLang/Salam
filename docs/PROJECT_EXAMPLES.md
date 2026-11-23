# Project examples

Small apps written only with the Salam page language: a `main.salam`, a `pages/` folder that is the route table, shared `components/`, state, handlers and styles in the DSL. Each project exists twice, in English keywords under `tests/en/` and in Persian keywords with Persian text and right to left layout under `tests/fa/`, with the same structure and features.

## How they are checked

- `tests/<lang>/page/projects/NAME/main.salam` is built by the `page` section of `tools/bash/run-tests.sh` (`salam page build main.salam --inline`). The `// EXPECT:` lines at the top of `main.salam` (`// انتظار:` in Persian) must appear in the generated page, so they pin routes, texts and generated code. Run it with `sh tools/bash/run-tests.sh page/projects`.
- `tests/<lang>/ssr/projects/NAME/main.salam` are server rendered projects (`server func`, `salam page build --ssr`). A server func needs a backend, so the runner only builds them (`ssr` section, job `ssrbuild`): the build must produce `dist/server`.
- A project that needs a browser to check behaviour is not covered by the runner; see the notes in its row.

## Router and platform showcases

| project | what it shows | build check |
| --- | --- | --- |
| `router_trails` | nested layouts, `[region]/[trail]` params, `[...rest]` catch-all, query strings, a guarded area with a login redirect, active links, 404, `page.route.GoTo/Redirect/Back`, history mode | page runner, en and fa |
| `i18n_theme` | language switch with `dir` flip, a translation table, light/dark/auto theme, preferences kept in storage, hash mode | page runner, en and fa |
| `forms_showcase` | every input type, validation messages, `bind`, submit handling, disabled state, a review page | page runner, en and fa |
| `components_showcase` | a design kit in `components/ui`: button, card, modal, tabs, accordion, tooltip, toast | page runner, en and fa |

## Server rendered projects

| project | what it shows | build check |
| --- | --- | --- |
| `ssr_blog` | server rendered blog, `server func` data loading, `prerender`, hydration with a client like button | ssr runner job, en and fa |
| `ssr_catalog` | product catalog, query string filter, server func list, detail and a reservation call | ssr runner job, en and fa |
| `ssr_dashboard` | dashboard whose numbers come from server funcs, CSS bar chart, orders table, an action call | ssr runner job, en and fa |

## Apps

Rows for the other projects of this collection. Add a row for each new project here and keep every row when a rebase conflicts.

| project | what it shows | build check |
| --- | --- | --- |
| `notes` | notes app with a dynamic route and hash mode | page runner, en and fa |
| `shop` | product list, cart and checkout with routes | page runner, en and fa |
| `landing.salam` | one page landing page | page runner, en and fa |
| `auto_import*` | the `components/` folder convention | page runner, en and fa |
| `chat`, `feed`, `gallery`, `music`, `quiz`, `arcade`, `toolbox`, `nav_link`, `route_style` | small apps, see each `main.salam` | page runner |
| `todo`, `kanban`, `habits`, `pomodoro`, `expenses`, `planner` | productivity apps | page runner |

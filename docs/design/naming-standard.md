# Naming standard for public names

Status: enforced for **new** English public function/method names by
`tools/salam/check-naming.salam` (baseline-aware). Existing violations may
remain in the baseline while APIs are migrated; new public English
functions/methods must follow this rule.

Checked by `tools/salam/check-naming.salam` and its baseline
`tools/salam/std-naming-baseline.txt` (CI: Public naming standard).

## The rule

- **English public callable names** (package functions and methods): must be
  **UpperCamelCase / PascalCase** (first letter of each word capitalized, no
  underscores). Acronym-style names (e.g. `URL`, `HTTP`) are accepted.
  Legacy lowercase/snake_case spellings are allowed **only as compatibility
  aliases** when a PascalCase/Acronym canonical exists in the same scope
  (so older Salam code can keep compiling while callers migrate).
- **Persian identifiers**: public or otherwise, Persian names must **not
  contain Latin/English letters** (`A-Z`, `a-z`). Use Persian words/spaces or
  ZWNJ as appropriate.
- **Stored public values** (struct fields, constants, package-level vars):
  remain governed by the naming checker/baseline (snake_case/value naming
  conventions per kind), except where a specific domain/mirror API requires
  otherwise.

| Public thing                                  | Style                       | Example                          |
| --------------------------------------------- | --------------------------- | -------------------------------- |
| package function (English)                     | UpperCamelCase (PascalCase) | `str.StartsWith(s, p)`           |
| method (English)                              | UpperCamelCase (PascalCase) | `v.Push(x)`, `s.StartsWith(p)`   |
| compat alias (legacy English)                 | legacy spelling allowed     | `v.push(x)` when `v.Push` exists |
| Persian identifier                            | no Latin letters            | `تابع محاسبه` / `نام‌کاربر`      |
| struct field                                  | snake_case / per checker    | `info.is_dir`                    |
| constant                                      | per checker/baseline        | `const max_header_size := 8192`  |
| package level variable                        | snake_case / per checker    | `pub mut default_timeout := 30`  |
| type, interface, enum                         | PascalCase (already true)   | `HashMap`, `FileInfo`            |
| enum member                                   | PascalCase                  | `Color.Red`                      |

Private names (no `pub`) are free. The recommendation is snake_case for locals,
parameters and private functions, and a leading underscore for private helpers
that share a package with a public name. The lint never looks at them.

PascalCase is checked loosely on purpose: the first letter is upper case and
there is no underscore. `ParseURL`, `ParseUrl` and `UrlDecode` all pass.
Initialisms are inconsistent in std today (`URLEncode` 27, `UrlDecode` 6) and
renaming them gains nothing.

snake_case means lower case letters, digits and single underscores. A
one-word name such as `size` is snake_case.

### Constants: snake_case, as decided

The decision is snake_case for public constants, not UPPER_SNAKE. Reasons that
hold: one rule for every stored value (constants, variables, fields), no
second style to remember, and it matches the layout DSL, which is already all
snake_case. What it costs is in the measurements below: outside the layout
DSL, 1817 of 1817 public constants are not snake_case today.

Constants that form a closed set (`Inherit`, `BgRed`, `ScriptGaray`,
`FlagKindInt`) are really enum members. The migration should turn those into
enums (PascalCase members) where the values are never combined as bit flags,
rather than renaming each to snake_case.

### Exceptions that stay as written

1. `extern:` block declarations. They name C symbols (`salam_open`,
   `getenv`, raylib's `InitWindow`). About 570 indented `pub func` lines in
   `std/gui/raylib` and similar packages live in extern blocks and are never
   checked. `@external` names likewise.
2. Operators: `pub func +`, and the `operator_*` hooks (`operator_index`,
   `operator_eq`, and so on).
3. Protocol/operator hooks the compiler/runtime may require by specific name
   (e.g. `operator_*`, and language/protocol hooks where applicable): keep the
   required canonical/compat spellings so compiler and runtime dispatch remain
   correct. Prefer providing UpperCamelCase public forms alongside required
   legacy/protocol names when possible.
4. `main`.
5. Persian identifiers (see below).
6. Mirror packages: a package whose job is to look like an external API
   (`gui/raylib` mirrors raylib, `metatrader` mirrors the MT5 API, `llvm`
   mirrors the LLVM C API, `net/http2` mirrors RFC 7540 names, `unicode`
   mirrors Unicode script names). See open question 4.

## Measurements

All numbers come from `salam run tools/salam/check-naming.salam counts` over
`std/` (196 packages, 19398 public declarations) plus `git grep`.

### Public declarations in std

| Kind                | Total | Compliant | Violations | Notes                                                  |
| ------------------- | ----- | --------- | ---------- | ------------------------------------------------------ |
| package function    | 7393  | 7383      | 10         | 9 in `collections/vector_ops`, `jalali_date`         |
| method (not extern) | 1237  | 665       | 572        | 246 with underscores, 326 one lower case word          |
| field               | 5025  | 4935      | 90         | camelCase in raylib, GraphQL, protobuf (all mirrors)   |
| constant            | 4778  | 2961      | 1817       | all 2961 compliant ones are the layout DSL             |
| type                | 922   | 922       | 0          | PascalCase already                                     |
| enum member         | 43    | 43        | 0          | PascalCase already (only members of `pub enum`)        |

Constant styles outside the layout DSL: 1021 UPPER_CASE (`RAR_OK`, `ZIP_*`,
`KEY_SCROLL_LOCK`, `DEAL_ENTRY_IN`), 764 PascalCase (`BgRed`, `Inherit`,
`ScriptGaray`), 32 short acronyms (`TLS11`, `SQLITE`, `ZWJ`). There are no
public package level variables in std at all; the rule for them is for user
code.

The 10 function violations are `contains`, `index_of`, `reverse`, `clone`,
`slice`, `swap`, `count_of`, `last_index_of`, `extend` (all in
`std/collections/vector_ops.salam`),
`jalali_date` (`std/layoutgen`).

Violations by package, the biggest: `gui/raylib` 263, `unicode` 260,
`metatrader` 188, `ai/ml` 141, `ai/opencv` 118, `collections` 97, `graphql` 88,
`archive` 79, `excel` 68, `graph` 58, `term` 57, `encoding/protobuf` 57,
`net/http2` 51. 101 packages have at least one violation. The full list is
`tools/salam/std-naming-baseline.txt` (2489 lines, `class|package|name`).

The mirror packages from exception 6 account for about 1030 of the 2489
violations (`gui/raylib`, `unicode`, `metatrader`, `llvm`, `net/http2`,
`graphql`, `encoding/protobuf`, `db/typesense`, `net/socketio`).

Methods that ARE lower case and must be looked at one by one: `collections`
(97: `is_empty`, `push_back`, `pop_front`, `get_or`), `ai/ml` and `ai/nn`
(`fit`, `predict`, `forward`, `score`), `time` (`Timer.wait`), `graph`,
`excel`, `matrix`.

### Built-in method names

The compiler knows 42 built-in method names, each with a Persian spelling, in
one table (`k_sm_intr_methods` in `compiler/semantic/semantic.salam`, resolved
by `intrinsic_method_canon` in `compiler/semantic/sema_util.salam`):

```
push pop get ref set len cap free put has remove size iter has_next key value
next read readline write seek close concat substr find split trim to_int
to_float char_count char_at char_substr char_find starts_with ends_with
includes copy deep_copy bytes lower upper repeat
```

Every backend dispatches on the canonical English spelling after that table
runs: string literals of these names appear 238 times in `semantic/`, 48 in
`codegen/`, 63 in `interp/`, 49 in `jsgen/`, 39 in `llvmgen/` and 3 in
`nativegen/`. A second small table (`method_alias` in `sema_state.salam`)
already maps foreign spellings (`startsWith`, `toUpperCase`) to the canonical
one for the "did you mean" hint.

`Vector` and `HashMap` are ordinary std structs with the same lower case
methods (`std/collections/vector.salam` has `reserve`, `push`, `pop`, `get`,
`ref`, `set`, `len`, `cap`, `is_empty`, `first`, `last`, `clear`, `insert`,
`remove_at`), so renaming them is a std change as well as a table change.

### Call sites (all `.salam` files, 12164 files)

Counts of `.name(` for the 42 names, an upper bound because user and std
methods with the same name are included:

| Group                | Call sites |
| -------------------- | ---------- |
| `std/`               | 19690      |
| `compiler/`          | 15926      |
| `tests/`             | 11726      |
| `ai/`                | 4634       |
| `website/` examples  | 4273       |
| `tools/`             | 3251       |
| `agent/`, `extensions/`, `kids/`, `examples/` | 1395 |
| Total                | 62148      |

Another 555 are in Markdown, HTML and TeX (`website/`, `books/`, `docs/`).

Most common: `.len(` 16422, `.push(` 14377, `.get(` 11424, `.Free(` 7969, `.set(`
2211, `.concat(` 2206, `.substr(` 1913, `.find(` 1093, `.put(` 926, `.has(` 585,
`.to_int(` 457. Everything below `.pop(` is under 340 each.

Also keyed by public names: 57960 `doc.` and `ex.` lines in 344 site std and
guide content files, 13291 `@en` lines and 12768 `@fa` lines in std, and the
std coverage baseline. A rename must move all of them.

### Why a case-insensitive lookup is not the answer

A Nim style "ignore case and underscores" lookup would make `IsEmpty` and
`is_empty` the same name with no migration at all. Folding every public name
(lower case, underscores removed) and looking for two different names in the
same package or struct finds 11 collisions already, for example field `code`
beside method `Code` in `db/clickhouse` `Conn`, and the same for `Stats.up`,
`Stats.down`, `Stats.total` in `net/tunnel`, `Command.env` in `os/process`.
The new rule makes those pairs legal (field snake_case, method PascalCase), so
the lookup must stay case-sensitive. Run
`salam run tools/salam/check-naming.salam collisions` for the list.

## How built-in methods fit

Two lookups, both deterministic, both only on a failed exact match:

1. The front door. In `intrinsic_method_canon`, a PascalCase name that
   converts (`Push` to `push`, `IsEmpty` to `is_empty`, `StartsWith` to
   `starts_with`, `DeepCopy` to `deep_copy`) to one of the canonical names is
   accepted and rewritten to the canonical lower case form. Backends keep
   matching their roughly 440 literals unchanged, so no backend work at all.
2. Ordinary methods (std and user structs). If `x.Name` finds nothing, retry
   with the snake_case conversion; if `x.name` finds nothing, retry with the
   PascalCase conversion. This is what lets std rename a definition
   (`is_empty` to `IsEmpty`) while old callers keep compiling, and the other
   way around.

The old spelling is accepted for a deprecation window and reported as W164
when the lint is on. The canonical (internal) names do not change, so the
Persian table is untouched and `@fa` names keep working as they do now.

Case conversion rules: Pascal to snake inserts `_` before each upper case
letter that follows a lower case letter or digit and lower-cases everything;
snake to Pascal upper-cases the first letter and every letter after `_` and
drops the underscores. A name that does not round trip (`IPv4`, `Conv2D`)
gets no fallback, it must be spelled exactly.

## How Persian names map

Nothing changes for Persian. The rule applies to the English identifier only:

- A Persian `@fa` name has no case. Multi word Persian names keep spaces (or
  ZWNJ inside a word), functions and fields alike, never underscores. The
  English style, Pascal or snake, never leaks into the Persian column.
- The lint skips any identifier with a non ASCII letter, and skips all names in
  files written in Persian.
- `@en "Name"` must equal the declared identifier, so a rename updates both.
  The `@fa` line stays as is.
- Inside one struct, a field `@fa` must not equal a method `@fa`. Pascal
  methods beside snake fields do not change that, the Persian names were already
  distinct.
- Built-in methods keep their Persian spelling in the same table. Only the
  English column gains the Pascal front door.
- `tools/salam/check-fa-names.salam`, the std coverage tool and the `Practice`
  check in `ai/backend/policy.salam` key on identifiers and must learn the new
  English spelling in the same release as the rename (the Practice check also
  needs the new rule, since the AI writes Salam).

## Tooling

1. `tools/salam/check-naming.salam` (exists, informational). Modes `check`,
   `counts`, `collisions`, `list [package...]`, `update`. `check` compares with
   `tools/salam/std-naming-baseline.txt` and exits 0 unless `--strict` is
   given. CI runs it with `continue-on-error`.
2. Compiler lint W164 "public name does not follow the naming standard",
   patterned after the switch lint (W145): `--naming-lint=off|warn|error|fix`,
   off by default in 0.6.x, `warn` by default one minor later. The help text
   names the new spelling. Files from std and installed packages are never
   checked, so a package author is told when they publish, not their users.
   Error form later is E-coded, in the next free E number.
3. The fixer is `salam check --fix` with `--naming-lint=fix`, the same switch
   the existing lints use. It must run on resolved member calls, not on text:
   `.get(` is a built-in, a std method and a user method, `.ok` belongs to
   different structs, and a text rewrite cannot tell. The semantic pass already
   knows the receiver type at every member call, so it can emit exact edits
   (the switch lint does this today). It rewrites calls and declarations,
   keeps `@en` equal, and refuses to touch a name that would collide after the
   rename.
4. `salam fmt` is not needed; the fixer is a rename, not a layout change.

## Migration phases

Seed rule: CI builds the compiler with the previous release, so `compiler/`,
`std/` and `extensions/tools/` must compile under it. A new spelling can be
used in those trees only after a release that understands it is the seed. See
the history of the
`T[]` slice change, which failed nine build jobs at once.

| Phase | Release | What ships | Estimated cost |
| ----- | ------- | ---------- | -------------- |
| 0 | now | this document, `check-naming.salam`, baseline, informational CI step | done |
| 1a | 0.6.4 | built-in front door, implemented: the 42 built-in method names also accept PascalCase on Vector, HashMap, str and File receivers (rewritten to the canonical name, backends untouched); tests on four backends | done |
| 1 | 0.6.4 | second fallback for ordinary std and user methods (snake and Pascal retry on a miss); tests in `tests/en` and `tests/fa` on C, LLVM, interp and js; no deprecation message yet | 1 PR, 2 to 3 days (member lookup retry, tests for user struct, interface, generic) |
| 1b | 0.6.4 | W164 behind `--naming-lint=warn`, off by default; `fix` mode | 1 PR, 3 to 4 days (the rewrite is the hard part: edits on declarations, `@en`, imports of renamed functions) |
| 2 | one release after 1 (the 1 release is the seed) | std migrated with the fixer: 10 functions, 572 methods, then constants and fields outside mirror packages; `@en`, docs keys, tests, coverage baselines moved in the same PR per package | about 25 PRs (one per big package, a few batches for the small ones), 8 to 10 days; the 1817 constants are the bulk and are mostly mechanical |
| 3 | 0.7.0 | W164 on by default for user code; compiler, tests, site, books, `ai/`, `tools/` migrated with the fixer (62148 call sites, 6668 test files, 344 site content files, the books); Dockerfile pins (`ai/backend`, `tools/mcp`, `sandbox`) moved first because they compile repository std with an older compiler | 6 to 8 days, mostly review and the Docker pins; the AI prompt (`docs/ai/AGENTS.md`, `SKILL.md` files, `Practice` check) rewritten the same day or the assistant keeps writing the old names |
| 4 | 0.8.0 or 1.0 | old spelling an error, fallback removed | 1 PR, 1 day, plus a note in the release text |

Total: about 4 weeks of focused work spread over three releases. The big risk
is not the compiler, it is phase 2 and 3 volume and the order against the seed.

## Risks

1. Seed lag. Anything in `compiler/`, `std/`, `extensions/tools/` written with
   the new names before the seed knows them fails stage 1 with a misleading
   semantic error (E017 no method). The fix is the release ordering above.
   Pinned Dockerfiles that use repository std with an older compiler break the
   moment std renames a definition, even with the fallback in the new
   compiler.
2. Self hosting. The compiler uses lower case built-in methods about 16000
   times. It migrates in phase 3, after the seed is the phase 1 release, with
   the fixer, in one PR that CI proves by building twice.
3. Persian callers. A std rename that changes the English column does not
   change the Persian one, but the `doc.` and `ex.` keys, `check-fa-names` and
   coverage baselines are keyed on the English name; one missed key is a CI
   failure per language.
4. Third party code and `registry/` packages. They get W164 in phase 3 and an
   error in phase 4. The fixer is the answer; the window is two releases.
5. Mirror packages. Forcing `InitWindow`-style C names through the rule makes
   the raylib and MT5 packages harder to read next to their upstream docs.
6. Collisions after a rename: a package with both `Count` and `count` today
   (rare, the fixer detects it) or a field and a method that fold to the same
   name in a language with a case-insensitive step. The fixer refuses.
7. Documentation and AI. The books are stale already; the site `/learn`
   pages, and the AI prompt must move together or the
   assistant teaches the old style for months.
8. Performance and size: none. Names are resolved at compile time; the
   fallback adds a second lookup only after a miss.

## Open questions for the owner

1. Constants. snake_case is the decision. It reverses the whole of std outside
   the layout DSL (1021 UPPER, 764 Pascal). Confirm, or choose UPPER_SNAKE for
   constants only (then the lint exempts all-upper names and the 1021 stay).
2. Fields. snake_case is already 4935 of 5025. Confirm there is nothing to
   decide, and that the 90 camelCase mirror fields are exempt rather than
   renamed.
3. Built-in methods and protocol hooks. Rename (`v.Push`, `s.Len()`,
   `x.Free()`) with the old spelling kept as a deprecated alias for two
   releases, or treat the 42 built-ins plus the protocol hooks as language
   keywords that stay lower case, like `len` and `append` in Go, and apply the
   rule only to std and user code. The first is the stated rule and costs 62148
   call sites; the second leaves `.len()` and `.Free()` untouched and costs
   nothing. Also: should `iter`, `has_next`, `next`, `to_str`, `free` and the
   `operator_*` names rename too, since a user struct implements them?
4. Mirror packages. Exempt `gui/raylib`, `metatrader`, `llvm`, `net/http2`,
   `unicode`, `graphql`, `encoding/protobuf` (about 1030 names) by a package
   level marker, or rename them and add the upstream name to the docs.
5. Constant sets. Convert the 764 PascalCase constants to enums where the values
   are not bit flags (bigger change, better types), or rename in place.
6. Enum members. PascalCase proposed (all 43 pub members already are).
   Confirm.
7. Generic and interface method names. Interface methods are public API and
   follow the rule (`Tx.Ok`, `Tx.Exec` already do). Generic type parameters
   stay single capital letters. Confirm.
8. Timing and window. Proposed: accept in 0.6.4, warn by default at 0.7.0,
   error at 0.8.0. A shorter window means a smaller fallback to maintain; a
   longer one is kinder to the registry.
9. Package level variables. None exist in std, so the rule (snake_case) is only
   for users. Confirm, and whether `pub mut` globals should be discouraged
   instead.

## Status

Implemented: the measurement tool and baseline, and the built-in front door
(phase 1a: `v.Push(x)`, `s.StartsWith(p)`, `x.ToInt()` and the rest of the 42
names compile, the lower case spelling still works and is not reported). No
name in std was renamed and no existing behavior changed. Adding PascalCase twins to std
structs now (without touching the compiler) works technically, a struct may
hold `push` and `Push` and both resolve, but every new public symbol needs an
`@fa` name, tests in both languages and `doc.`/`ex.` keys in both site
languages to pass the std coverage check, so 572 twins would add about 3000
mandatory entries to be removed again in phase 2. It is not worth doing
before the compiler fallback exists.

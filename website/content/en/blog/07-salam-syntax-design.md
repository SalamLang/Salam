---
title: Designing Salam's syntax: readability, simplicity and power
seo_title: Salam Syntax Design: Readable Code Without Losing Power
description: Salam syntax explained: colon and end blocks, := and mut, until, repeat and each, match and multi-word names, compared with Python, Go, Ruby and Lua.
keywords: Salam syntax, programming language syntax design, readable syntax, language design, colon end blocks, immutable by default, match expression, multi-word identifiers, Python Go Ruby Lua syntax comparison, Salam language
category: language
summary: Why Salam code looks the way it does: the decisions behind its blocks, declarations, loops, pattern matching and names, what each one costs, and how they compare with the syntax of Python, Go, Ruby and Lua.
date: 2026-11-11
---

Salam's syntax is designed to be read more than written: blocks open with a colon and close with `end`, variables are declared with `:=` and are immutable unless marked `mut`, functions return with `ret`, and logic is written with the words `and`, `or` and `not`. **The aim is code that a beginner can read aloud, that an experienced programmer can scan quickly, and that maps one to one onto a second, Persian spelling, without giving up static types, pattern matching or low-level control.** Where it had to choose between brevity and clarity, Salam usually chose clarity, and where it had to choose between flexibility and one obvious way, it usually chose one obvious way.

This article goes through the main syntax decisions, shows them in runnable programs, compares them with Python, Go, Ruby and Lua, and is honest about what each decision costs.

## The principles behind the syntax {#principles}

Four ideas explain most of Salam's syntax:

1. **Blocks should be visible.** You should be able to see where a block ends without counting spaces or matching braces.
2. **The default should be the safe choice.** Values do not change unless you say so; conditions must be real booleans; numbers do not silently mean "true".
3. **One obvious way.** Where two spellings would do the same thing, Salam often allows only one and makes the other an error with a fix.
4. **Two spellings, one grammar.** Every construct must read naturally in English and in Persian, since the Persian spelling is a token-for-token mapping of the English one (see [one language, two scripts](/blog/one-language-two-scripts/)). That rules out syntax that relies on English word order or on symbols that behave badly in right-to-left text.

## Blocks: a colon to open, end to close {#blocks}

Every block in Salam opens with `:` and closes with `end`: functions, conditions, loops, structs, enums, match arms. A short block may sit on one line.

```salam
func sign of(n: int): str:
    if n < 0: ret "negative" end
    if n == 0: ret "zero" end
    ret "positive"
end

func main:
    each n in [-4, 0, 9]:
        println n, sign of(n)
    end
end
```

```output
-4 negative
0 zero
9 positive
```

This sits between Python and Ruby. Python opens blocks with a colon and closes them by dedenting, which is clean but makes the end of a block invisible and makes copied code fragile. Ruby and Lua close blocks with `end` but open them in several different ways (`do`, `then`, nothing). Salam takes the colon from Python and the `end` from Ruby and Lua, and uses the same pair everywhere. Indentation is conventional and `salam format` enforces it, but it carries no meaning, so a block pasted at the wrong depth still parses the same way.

Braces are not used for blocks; in Salam `{ }` belong to struct literals (`Point { x = 1 }`) and empty collection literals. The Persian spelling has the same shape: `:` and `پایان`.

## Declarations: :=, mut and const {#declarations}

```salam
const MAX_USERS := 100

func main:
    name := "Sara"
    mut visits := 0
    visits++
    visits += 2
    ratio := visits as f64 / MAX_USERS
    big := 3000000000 as i64
    println name, visits, ratio, big
end
```

```output
Sara 3 0.03 3000000000
```

`:=` declares and infers the type, as in Go. Unlike Go, the result is **immutable**: `name` can never be reassigned, and trying is a compile error, not a warning:

```text
error[E013]: cannot reassign immutable variable 'limit'; declare it 'mut' to reassign it, or assign to its elements or fields instead
```

`mut` opts into change, the same idea as Rust's `let mut` or Swift's `var`. A reader can trust that a name without `mut` keeps its value, which makes long functions much easier to follow.

There is no `x: int = 5` form for local variables. Salam used to have typed declarations and removed them: when you want a type other than the inferred one, you write `as`, as in `3000000000 as i64` above. That leaves exactly one way to declare a local. Type annotations remain where they carry information that cannot be inferred: parameters, return types and struct fields.

## Short keywords for frequent things {#keywords}

The words you type most are short: `func`, `ret`, `mut`, `pub`, `end`. The words you type less often are spelled out: `struct`, `enum`, `interface`, `defer`, `match`. That is the same trade-off Go made with `func`, and the reason `return` is `ret`: in a typical function it appears several times.

`println` and `print` are statements, not function calls, and they take their arguments without parentheses: `println name, visits`. The arguments are joined with spaces, which is what a beginner expects from a first program and what a debug line usually wants.

## Loops: until, repeat and each {#loops}

Salam has three loops and no `while` or C-style `for`:

```salam
func main:
    mut n := 27
    mut steps := 0
    until n != 1:
        n = n % 2 == 0 ? n / 2 : 3 * n + 1
        steps++
    end
    println "Collatz steps for 27:", steps
    repeat 2 in i:
        println "repeat", i
    end
    repeat 10 to 0 by 5 in i:
        println "down", i
    end
    each i, name in ["Sara", "Ali"]:
        println i, name
    end
end
```

```output
Collatz steps for 27: 111
repeat 0
repeat 1
down 10
down 5
down 0
0 Sara
1 Ali
```

- `repeat` covers counting: `repeat 3` runs three times, `repeat n in i` counts from 0, `repeat 1 to 5` is inclusive, and `by` sets the step. The bounds decide the direction, so `repeat 10 to 0` counts down.
- `each` iterates arrays, vectors, maps, strings and your own types, with an optional index or key.
- `until` is the general loop, and it is the one decision in this list that people trip over: **`until c:` runs while `c` is true.** The English word suggests the opposite. Its Persian spelling, `تا`, naturally means "as long as", so in Persian the same keyword reads correctly. The [designing for Persian article](/blog/designing-a-language-for-persian-and-arabic/) explains the trade-off; the practical advice is to read `until` as "while".

Dropping the three-part C `for` loop removes a classic source of off-by-one errors and makes every loop's intent visible in its first word. The [loops lesson](/learn/loops/) covers every form.

## match and switch {#match}

Salam has two branching constructs beyond `if`, and they do different jobs.

```salam
enum Shape:
    Circle(r: f64)
    Rect(w: f64, h: f64)
end

func describe(s: Shape): str:
    ret match s:
        Circle(r): "circle, radius " + r end
        Rect(w, h) if w == h: "square, side " + w end
        Rect(w, h): "rectangle, " + w + " by " + h end
    end
end

func grade(score: int): str:
    switch score:
        90 to 100: ret "A" end
        75 to 89: ret "B" end
        else: ret "C" end
    end
    ret "?"
end

func main:
    shapes := [Shape.Circle(1.5), Shape.Rect(2.0, 2.0), Shape.Rect(3.0, 1.0)]
    each s in shapes:
        println describe(s)
    end
    each score in [95, 80, 40]:
        println score, grade(score)
    end
end
```

```output
circle, radius 1.5
square, side 2
rectangle, 3 by 1
95 A
80 B
40 C
```

`match` is an expression: it produces a value, destructures enums that carry data, supports guards (`if w == h`) and must cover every case, or the program does not compile. It is the construct for working with data, in the spirit of Rust's `match` or Swift's `switch`.

`switch` is a statement for control flow over plain values, with ranges (`90 to 100`), comma lists and relational labels such as `> 60`. It has C-style fallthrough unless you `break` or return, which is useful for ported code and a known trap, so an opt-in lint flags long if-chains that should be a `switch`. Both use the same `label: ... end` shape as every other block. The [match lesson](/learn/match/) and [enums lesson](/learn/enums/) go further.

## Names with spaces {#multi-word-names}

A Salam name may contain spaces: `sign of`, `line total`, `is weekend`. A call then reads like a phrase: `sign of(n)`. This exists mainly for Persian, where many concepts are naturally two words and gluing them together looks wrong, but it works in English too.

It has two rules that keep it unambiguous. The words of a name may not include a keyword (`distance to origin` fails because `to` is a keyword), and `const` names must be a single word. Some teams will prefer `snake_case` for English code anyway, and that is fine; underscores are ordinary characters.

## Words for logic, and one way to update a value {#operators}

Logic is written with words: `and`, `or`, `not`. The symbols `&&`, `||` and `!` are not Salam operators, and the compiler says so and names the word to use:

```text
error: '&&' is not a Salam operator; write 'and'
```

Conditions must be real booleans. `if x:` with an integer `x` is error E021 ("if condition must be bool"), so there is no truthiness to remember. Comparisons can also be written as words (`gt`, `lt`, `eq`), which matters in Persian, where `<` and `>` are mirrored in right-to-left text.

Salam is strict about updating a variable in terms of itself. `x = x + 1` is an error with a fix:

```text
error[E162]: 'count = count + 1' is not allowed: write 'count++'
```

The rule is one spelling per operation: `x++` and `x--` for steps of one, `x += n` and friends for everything else. It feels fussy for a day and then disappears; the benefit is that every update in a codebase looks the same, which makes code review and search easier. Unused variables are also errors rather than warnings.

## Salam next to Python, Go, Ruby and Lua {#comparison}

The same function, summing the even numbers of a list, in five languages. Python:

```python
def sum_even(numbers):
    total = 0
    for n in numbers:
        if n % 2 == 0:
            total += n
    return total
```

Go:

```go
func sumEven(numbers []int) int {
    total := 0
    for _, n := range numbers {
        if n%2 == 0 {
            total += n
        }
    }
    return total
}
```

Ruby:

```ruby
def sum_even(numbers)
  total = 0
  numbers.each do |n|
    total += n if n.even?
  end
  total
end
```

Lua:

```lua
local function sum_even(numbers)
  local total = 0
  for _, n in ipairs(numbers) do
    if n % 2 == 0 then
      total = total + n
    end
  end
  return total
end
```

Salam:

```salam
func sum even(numbers: int[]): int:
    mut total := 0
    each n in numbers:
        if n % 2 == 0:
            total += n
        end
    end
    ret total
end

func main:
    println sum even([1, 2, 3, 4, 5, 6])
end
```

```output
12
```

| Feature                 | Python              | Go            | Ruby                     | Lua                      | Salam                     |
| ----------------------- | ------------------- | ------------- | ------------------------ | ------------------------ | ------------------------- |
| Block delimiters        | colon + indentation | braces        | `end` (various openers)  | `end` (`do`, `then`)     | colon + `end`             |
| Declaration             | assignment          | `:=` or `var` | assignment               | `local`                  | `:=`                      |
| Immutable by default    | no                  | no            | no                       | no                       | yes, `mut` to change      |
| Static types            | optional hints      | yes           | no                       | no                       | yes, inferred             |
| Loop forms              | `for`, `while`      | `for` only    | `each`, `while`, `until` | `for`, `while`, `repeat` | `each`, `repeat`, `until` |
| Logical operators       | `and or not`        | `&& \|\| !`   | both                     | `and or not`             | `and or not`              |
| Pattern matching        | `match` (3.10+)     | no            | `case/in`                | no                       | `match`, exhaustive       |
| Compiled to native code | no                  | yes           | no                       | no                       | yes                       |

The Salam version reads closest to Python and Lua, keeps Go's `:=` and static types, and adds an explicit `end` and explicit mutability. It is longer than the Ruby version, which leans on blocks and an implicit return; Salam prefers explicit `ret`.

## What the syntax costs {#trade-offs}

Every choice has a price, and some of Salam's are real:

- **`until` means while.** Familiar English words carry expectations, and this one goes against them. It is the most common porting mistake.
- **`end` is verbose.** Deeply nested code accumulates `end` lines that Python would not need.
- **Strictness slows the first hour.** Errors for unused variables, `x = x + 1` and non-boolean conditions surprise newcomers, even though each comes with a fix.
- **Pre-1.0 changes.** The syntax still moves between releases: typed local declarations and the `=>` lambda arrow were removed. Code written for an older release may need edits.
- **Names with spaces are unusual.** Tools that assume identifiers have no spaces, from grep patterns to other languages' highlighters, need care.

For the full set of rules, see the [rules lesson](/learn/rules/) and the [best practices lesson](/learn/best-practices/); for how the syntax grew out of the language's goals, read [why the world still needs new programming languages](/blog/why-new-programming-languages/).

## Frequently asked questions {#faq}

### Is Salam syntax like Python?

It is close in spirit: a colon opens a block, there are no semicolons, logic uses `and`, `or` and `not`, and code reads top to bottom. The differences are that blocks close with `end`, types are static and inferred, variables are immutable unless declared `mut`, and the code compiles to a native executable.

### Does Salam use curly braces?

Not for blocks. Every block opens with a colon and closes with `end`. Braces appear in struct literals such as `Point { x = 1, y = 2 }` and in empty collection literals.

### Why does Salam use until instead of while?

Salam has no `while` keyword; `until c:` loops while `c` is true. Its Persian spelling, `تا`, means "as long as" and reads naturally, but the English word reads against its usual meaning. Read it as "while".

### Can variable names in Salam contain spaces?

Yes. Names such as `line total` or `is weekend` are single identifiers, which helps especially in Persian. A multi-word name may not contain a keyword, and constants must be one word.

---
title: Salam vs Python: native speed with a simple syntax?
seo_title: Salam vs Python: Syntax, Speed, Typing and Ecosystem
description: Salam vs Python: similar readable syntax, but Salam compiles to native code and ran about 56x faster on the benchmark. Typing, ecosystem and when Python wins.
keywords: Salam vs Python, Python alternative, faster than Python, compiled language like Python, static typing vs dynamic typing, Python performance, native code, Python ecosystem, NumPy, learn programming language
category: compare
summary: Salam reads almost like Python but compiles to native code with static types and no garbage collector. Here are the same programs in both, the measured speed difference, and the many cases where Python still wins.
date: 2026-11-11
image: /assets/img/blog/salam-vs-python-en.png
image_alt: Cover image for "Salam vs Python: native speed with a simple syntax?"
---

Salam and Python share a goal: code that is short and easy to read. They reach it in opposite ways. **Python is dynamically typed and usually runs on an interpreter; Salam is statically typed and compiles to native machine code.** On the project's benchmark of 39 small programs that use the same algorithm in every language, Python took about 56 times as long as Salam. Python, in return, has one of the largest library ecosystems of any language, an interactive workflow that data scientists rely on, and decades of tutorials, answers and production experience. Salam has a broad standard library and a small community.

So Salam is not a drop-in replacement for Python. It is an option for the parts of a project where Python's speed, memory use or deployment story gets in the way, written in a syntax a Python programmer can read on the first day.

## Salam vs Python at a glance {#at-a-glance}

|                        | Python                                                                                              | Salam                                                                               |
| ---------------------- | --------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| Execution              | Interpreted bytecode (CPython); other implementations such as PyPy exist                            | Compiled ahead of time to native code; an interpreter (`salam exec`) for quick runs |
| Typing                 | Dynamic; optional type hints checked by external tools                                              | Static with inference, checked by the compiler                                      |
| Blocks                 | Indentation                                                                                         | A colon opens a block, `end` closes it                                              |
| Memory                 | Reference counting plus a cycle collector                                                           | Manual: `.Free()` and `defer`, with compile-time checks                             |
| Parallelism            | GIL in the default build; an optional free-threaded build (experimental in 3.13, supported in 3.14) | OS threads, a task pool, channels, atomics                                          |
| Speed on the benchmark | About 56 times Salam's time                                                                         | In the same range as C and Rust                                                     |
| Deployment             | Interpreter plus dependencies, or a bundler                                                         | One native executable                                                               |
| Ecosystem              | Enormous: PyPI, NumPy, pandas, PyTorch, Django, Jupyter                                             | Large standard library, few third-party packages                                    |
| Source languages       | English keywords                                                                                    | English or Persian keywords, same compiler                                          |

## The same program in both languages {#side-by-side}

Count the words in a sentence and print the three most common, most frequent first and alphabetically among ties. Python makes this a few lines with `collections.Counter`:

```python
from collections import Counter

text = "the cat sat on the mat and the dog sat on the log"
counts = Counter(text.Split())
top = sorted(counts.items(), key=lambda kv: (-kv[1], kv[0]))[:3]
for word, n in top:
    print(word, n)
```

Salam has no `Counter` type for this, so the program builds the counts in a `HashMap`, moves them into a `Vector` of structs and sorts it with a comparison function:

```salam
import sort
import str

struct Entry:
    pub word: str
    pub count: int
end

func main:
    text := "the cat sat on the mat and the dog sat on the log"
    words := str.Fields(text)
    defer words.Free()
    mut counts := HashMap {} as HashMap<str, int>
    defer counts.Free()
    each w in words:
        counts.Put(w, counts.GetOr(w, 0) + 1)
    end
    mut entries := Vector {} as Vector<Entry>
    defer entries.Free()
    each w, n in counts:
        entries.Push(Entry { word = w, count = n })
    end
    sort.SortBy(entries, (a: Entry, b: Entry):
        if a.count != b.count: ret a.count > b.count end
        ret str.Compare(a.word, b.word) < 0
    end)
    repeat 3 in i:
        e := entries.Get(i)
        println e.word, e.count
    end
end
```

```output
the 4
on 2
sat 2
```

Both print the same three lines. The Salam version is longer, and the reasons are worth naming: types are declared on the struct, the containers are freed with `defer`, and there is no ready-made counting type. On the other hand, much of it will look familiar to a Python programmer: `each w in words` is `for w in words`, `each w, n in counts` is `for w, n in counts.items()`, `repeat 3 in i` is `for i in range(3)`, and there are no braces or semicolons. Salam even prints like Python's `print`, joining its arguments with spaces.

## Static types against dynamic types {#typing}

In Python, a type mistake is found when the line runs:

```python
def discount(price, percent):
    return price * (100 - percent) / 100

print(discount(80.0, 15))    # 68.0
print(discount(80.0, "15"))  # TypeError: unsupported operand type(s) for -: 'int' and 'str'
```

In Salam, the same mistake never reaches a running program. The working version:

```salam
func discount(price: f64, percent: f64): f64:
    ret price * (100.0 - percent) / 100.0
end

func main:
    println discount(80.0, 15.0)
end
```

```output
68
```

Change the call to `discount(80.0, "15")` and `salam check` stops with `error[E012]: argument 2: cannot pass 'str' to 'f64'`, pointing at the line. Python can get close with type hints and a checker such as mypy or Pyright, and many teams do exactly that; in Salam the check is part of the compiler and cannot be skipped. Local types are inferred (`count := 0` is an integer, `name := "Sara"` a string), so the annotations you write are mostly on function parameters and struct fields. The cost is flexibility: no duck typing at runtime, no monkey patching, and a little more ceremony for heterogeneous data, which Salam handles with interfaces, generics and `int or str` union types. The [static typing article](/blog/static-typing-in-salam/) covers the type system.

## How much faster is Salam than Python? {#performance}

On the benchmark published on the [speed page](/speed/), the geometric mean over 39 same-algorithm programs was 10.6 ms for Salam and 598 ms for Python, so Python took about 56 times as long. Here is one of those programs, `04_collatz`, which finds the number below 30,001 with the longest Collatz sequence. The Python source from the benchmark:

```python
best_steps = 0
best_n = 1
for n in range(1, 30001):
    m = n
    c = 0
    while m != 1:
        if m % 2 == 0:
            m = m // 2
        else:
            m = 3 * m + 1
        c += 1
    if c > best_steps:
        best_steps = c
        best_n = n
print(best_steps)
print(best_n)
```

And the Salam source, from the same directory:

```salam
func main:
    mut best steps := 0 as i64
    mut best n := 1 as i64
    mut n := 1 as i64
    until n <= 30000:
        mut m := n
        mut c := 0 as i64
        until m != 1:
            if m % 2 == 0:
                m /= 2
            else:
                m = 3 * m + 1
            end
            c++
        end
        if c > best steps:
            best steps = c
            best n = n
        end
        n++
    end
    println best steps
    println best n
end
```

```output
307
26623
```

The two are nearly line for line the same (Salam writes `until cond:` for a loop that runs while the condition holds, and allows spaces in names like `best steps`). Python took 245.7 ms, Salam 3.7 ms. The reason is structural, not clever: CPython executes bytecode and works with boxed integer objects, while Salam compiles the loop to machine instructions on 64-bit integers.

Two caveats keep the comparison fair. First, the programs are short, so interpreter start-up is part of Python's times, and the ratio varies by program: on recursive Fibonacci it was about 19 times, on the game of life almost a thousand. Second, real Python code that does heavy numeric work calls into libraries written in C. Programs 40 to 42 of the suite use NumPy in Python and Salam's `matrix` and `tensor` packages: Python finished the 400 by 400 matrix multiplication in 121.8 ms, against 14.3 ms for Salam and 42.5 ms for C's hand-written loops. With NumPy, Python stays within a few times of compiled code; without it, it is one to two orders of magnitude behind. The [benchmark article](/blog/is-salam-faster-than-c-and-rust/) explains the method and its limits.

## The ecosystem gap {#ecosystem}

This is where Python wins, and by a wide margin. PyPI holds hundreds of thousands of packages. Data science runs on NumPy, pandas, SciPy and Jupyter; machine learning on PyTorch, scikit-learn and the Hugging Face libraries; web backends on Django, Flask and FastAPI; and almost every cloud service, database and API ships a Python client first. Answers to almost any Python question are a search away, and most programmers already know some Python.

Salam's [standard library](/std/) is unusually broad for a young language: HTTP servers and clients, a router, WebSocket, gRPC, drivers for SQLite, PostgreSQL, MySQL, Redis and ClickHouse, JSON and CSV, cryptography, compression, images, audio, and machine learning building blocks such as tensors, automatic differentiation, neural network layers and ONNX and GGUF model loading. It installs with the compiler, so many programs need no third-party code. But third-party packages are few, there is nothing like the scientific Python stack's depth, and the community that writes tutorials and answers questions is small. For machine learning in particular, the [machine learning article](/blog/machine-learning-with-salam/) describes what exists, and the [Salam vs Python and Julia](/blog/salam-vs-python-and-julia/) piece compares the numerical side.

## Memory and parallelism {#memory-concurrency}

Python manages memory for you with reference counting and a cycle collector, which is a large part of why it is pleasant to write. Salam has no garbage collector: containers are freed with `.Free()`, usually through `defer`, and a function that returns a new string or container hands its ownership to the caller. The compiler reports common mistakes such as use after free, but a forgotten `free` is a leak. In exchange, a Salam program's memory use follows what it allocates, with no collector and no boxed objects for plain numbers.

For parallel CPU work, CPython's default build has a global interpreter lock, so threads do not run Python code in parallel; multiprocessing, native extensions or the optional free-threaded build (supported since Python 3.14) are the usual answers. Salam threads run in parallel on all cores, and `pool.ParallelFor`, `pool.Map` and the `go` keyword spread work across them, with mutexes, atomics and channels to coordinate. The [concurrency article](/blog/concurrency-in-salam/) has examples.

## Workflow: scripts, REPLs and notebooks {#workflow}

Python's edit-and-run loop is immediate, and notebooks make exploratory work interactive. Salam keeps part of that feel. `salam run file.salam` compiles and runs in one step without leaving files behind, `salam exec` runs a program with the built-in interpreter when no C toolchain is available, and `salam cli` starts a REPL. The [online editor](https://editor.salamlang.ir) runs programs in a browser tab. There is no notebook environment comparable to Jupyter, and compiling adds a short delay before each run. The article [salam run vs salam build](/blog/salam-run-vs-salam-build/) explains the options.

At the other end, deployment favors Salam: `salam build` produces one native executable that needs no interpreter, no virtual environment and no dependency installation on the target machine.

## When to choose Python, when to choose Salam {#when}

Choose **Python** when:

- the work is data analysis, scientific computing or machine learning that relies on NumPy, pandas, PyTorch and friends;
- you are writing glue code, automation or prototypes where development speed matters more than runtime;
- the libraries, hiring pool or team experience you need are in Python;
- you work interactively in notebooks.

Choose **Salam** when:

- a CPU-bound part of the program is too slow in pure Python and you would rather not drop to C;
- you want a single native executable to deploy, low memory use and fast start-up;
- you want type errors caught before the program runs;
- you want a readable compiled language for teaching, including in Persian, since every keyword and error message exists in both languages ([one language, two scripts](/blog/one-language-two-scripts/)).

Many projects will use both: Python for exploration and orchestration, a compiled language for the hot path or the deployed service.

## Frequently asked questions {#faq}

### Is Salam faster than Python?

Yes, by a large margin on CPU-bound code. On the project's benchmark of 39 programs Python took about 56 times as long as Salam (598 ms against 10.6 ms, geometric mean). With NumPy doing the heavy work, Python's gap to compiled code shrinks to a few times.

### Is Salam easy to learn for Python programmers?

Mostly yes. Loops, conditions and printing look similar, there are no braces or semicolons, and types are inferred for local variables. The new parts are declared types on functions and structs, `end` instead of indentation, and freeing heap memory with `defer x.Free()`.

### Can Salam replace Python for data science?

Not today. Salam has tensors, matrices, statistics and neural network building blocks, but nothing close to the depth of NumPy, pandas, SciPy, PyTorch and Jupyter. It is better suited to speeding up specific parts or to deploying finished tools.

### Does Salam have a REPL like Python?

Yes. `salam cli` starts an interactive session, `salam exec` runs a file with the interpreter, and the online editor runs code in the browser. There is no notebook environment like Jupyter.

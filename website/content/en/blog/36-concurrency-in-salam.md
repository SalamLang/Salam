---
title: Concurrent programming in Salam: threads, task pools, channels and synchronization
seo_title: Concurrency in Salam: Threads, Channels and Thread Pools
description: Concurrency in Salam with runnable code: spawn and join, the go keyword, bounded thread pools, parallel loops, futures, channels, select, mutexes and atomics.
keywords: concurrency, threads, channels, thread pool, parallel programming, go keyword, mutex, atomic operations, select statement, futures, error group, Salam concurrency
category: build
summary: Salam's concurrency is built on operating system threads with a bounded task pool on top. This guide covers each tool, from spawn and join to channels, select, groups and futures, with programs that run, and compares the model with Go, Rust and Java.
date: 2026-11-11
---

Concurrency is the ability of a program to make progress on several tasks at once, and on a multi-core processor to run them in parallel. **Salam's concurrency model is built on operating system threads: `spawn` and `join` start and wait for threads, the `go` keyword runs a call on a shared bounded task pool, channels pass values between threads, and the `sync` and `atomic` packages protect shared data.** There is no garbage collector to pause the threads and no `async`/`await`.

This article walks through each tool with a program you can run, shows which one fits which problem, compares the model with Go, Rust and Java, and lists the mistakes that cause hangs and crashes. The course lesson on [concurrency](/learn/threads/) is the gentler introduction; this is the overview for people who already write concurrent code in another language.

## The concurrency model: OS threads, pools and no async/await {#model}

Every concurrent unit in Salam is, in the end, an operating system thread. On top of that, the standard library builds the usual higher-level tools:

| Need                             | Tool                                                                                |
| -------------------------------- | ----------------------------------------------------------------------------------- |
| Start a thread, wait for it      | `spawn`, `join` (built-in), [thread](/std/thread/) for groups and limits            |
| Many small jobs, bounded         | `go f(x)`, `pool.New`, `pool.Submit` from [pool](/std/pool/)                        |
| A loop over indices on all cores | `pool.ParallelFor`, `pool.Map`                                                      |
| Results and errors from tasks    | `pool.Async` and `Await`, `pool.NewGroup` and `GroupWait`                           |
| Pass values between threads      | [chan](/std/chan/): buffered or rendezvous channels, `Select`, timers               |
| Protect shared data              | [sync](/std/sync/): `Mutex`, `RWMutex`, `CondVar`, `WaitGroup`, `Semaphore`, `Once` |
| One number or flag, lock-free    | [atomic](/std/atomic/): `I64`, `Flag`                                               |
| Cancel work, set deadlines       | [ctx](/std/ctx/): `WithCancel`, `WithTimeout`, `WithDeadline`                       |

The choice of real threads has consequences you should know up front. Tasks on a pool are threads, not green threads: a task that blocks keeps its worker busy. That is simpler to reason about than an async runtime, and C libraries called through the [FFI](/blog/salam-c-interop-ffi/) behave exactly as they would in C. It also means a server with ten thousand idle connections does not want ten thousand threads, which is why Salam's HTTP server uses an epoll worker pool on Linux instead of a thread per connection.

## Threads with spawn and join {#threads}

`spawn(f)` starts `f` on a new thread and returns a handle; `join(handle)` waits for it. A thread function takes no arguments or exactly one pointer, `spawn(f, data)`. The pointer is how a thread gets its own piece of work and where it leaves its result, and the data has to stay alive while the thread runs, so it lives on the heap.

```salam
import mem

struct Range:
    pub first: i64 = 0
    pub last: i64 = 0
    pub sum: i64 = 0
end

func add squares(r: Range*):
    mut total := 0 as i64
    repeat r[0].first to r[0].last in i:
        total += i * i
    end
    r[0].sum = total
end

func new range(first: i64, last: i64): Range*:
    r := mem.Allocate(sizeof(Range)) as Range*
    r[0] = Range { first = first, last = last }
    ret r
end

func main:
    low := new range(1, 500000)
    high := new range(500001, 1000000)
    t1 := spawn(add squares, low)
    t2 := spawn(add squares, high)
    join(t1)
    join(t2)
    println "first half:", low[0].sum
    println "second half:", high[0].sum
    println "total:", low[0].sum + high[0].sum
    mem.Free(low as void*)
    mem.Free(high as void*)
end
```

```output
first half: 41666791666750000
second half: 291667041666750000
total: 333333833333500000
```

The two threads never touch the same memory, so they need no coordination: the easiest kind of concurrent program to get right. Join every thread exactly once and read its results only after the join; a second `join` on the same handle uses freed memory. `thread.SetLimit(n)` caps how many threads may be alive at once, after which `spawn` returns zero instead of exhausting the system, and `thread.SpawnStack` starts a thread with a larger stack for deep recursion.

## The go keyword and shared state with a mutex {#go}

For many small jobs, starting one thread each is wasteful and, without a limit, dangerous. Writing `go` before a call runs it as a task on a shared pool with one worker per core and a bounded queue. When the queue is full, `go` waits, so a fast loop cannot pile up unbounded work. `pool.GoWait()` waits for every task. The arguments are evaluated when the `go` line runs, so `go record(code)` in a loop sends each iteration's own value.

When tasks update the same data, they need a lock. Here 3,000 tasks count HTTP status codes into one shared map, guarded by a mutex:

```salam
import pool
import sync

mut lock := sync.NewMutex()
mut by_status := HashMap {} as HashMap<int, int>

func record(code: int):
    sync.Lock(lock)
    by_status.Put(code, by_status.GetOr(code, 0) + 1)
    sync.Unlock(lock)
end

func main:
    repeat 3000 in i:
        mut code := 200
        if i % 10 == 0:
            code = 404
        end
        if i % 100 == 0:
            code = 500
        end
        go record(code)
    end
    pool.GoWait()
    println "200:", by_status.GetOr(200, 0)
    println "404:", by_status.GetOr(404, 0)
    println "500:", by_status.GetOr(500, 0)
    sync.Destroy(lock)
    by_status.Free()
end
```

```output
200: 2700
404: 270
500: 30
```

Without the `Lock` and `Unlock`, two tasks can read the same old count and both write back one more, and two inserts at once can corrupt the map itself. For a single counter, `atomic.Add` on an `atomic.I64` does the read, add and write as one indivisible step and is lighter than a mutex. The `atomic` package is lock-free everywhere except when the program is built with tcc, where it falls back to a mutex per cell with the same semantics. `go` is a contextual keyword, so a variable named `go` still works, and in Persian the keyword is `برو`.

## Parallel loops: ParallelFor and Map {#parallel}

Often all you want is the same loop body over many indices, spread across the cores. `pool.ParallelFor(n, workers, body)` calls `body(i)` for every `i` from 0 to `n - 1` and returns when all are done; `pool.Map` does the same and collects each result into a `Vector<i64>`. This program counts the primes below one million in ten blocks:

```salam
import pool

func is prime(n: i64): bool:
    if n < 2: ret false end
    mut d := 2 as i64
    until d * d <= n:
        if n % d == 0: ret false end
        d++
    end
    ret true
end

func count primes(block: i64): i64:
    mut found := 0 as i64
    start := block * 100000
    repeat start to start + 99999 in n:
        if is prime(n): found++ end
    end
    ret found
end

func main:
    counts := pool.Map(10, pool.CPUCount(), (b: i64): ret count primes(b) end)
    mut total := 0 as i64
    repeat counts.len() in i:
        total += counts.Get(i)
    end
    println "primes below 1000000:", total
    println "in the first block:", counts.Get(0), "in the last:", counts.Get(9)
    counts.Free()
end
```

```output
primes below 1000000: 78498
in the first block: 9592 in the last: 7224
```

Results land in their own slots, so the output is the same whatever order the blocks finished in. The [pool reference](/std/pool/) gives the rules: never rely on the order of turns, keep each turn large enough to be worth a thread (chunks, not single items), and do not print from the loop body. Note `until d * d <= n:` in `is prime`: Salam's `until` loops _while_ its condition is true, which surprises people coming from Ruby.

## Error groups and futures {#tasks}

Two patterns from Go's `errgroup` and Java's `Future` are built into the pool package. A `Group` runs tasks with a limit on how many run at once and keeps the first error, reported as a non-empty string. `Async` runs a task on a pool and hands back a `Future` that you `Await`:

```salam
import pool

func check(row: int): str:
    if row == 13:
        ret "row 13: price is negative"
    end
    ret ""
end

func slow sum(n: i64): i64:
    mut s := 0 as i64
    repeat n in i:
        s += i
    end
    ret s
end

func main:
    g := pool.NewGroup(4)
    repeat 50 in row:
        pool.GroupGo(g, (): ret check(row) end)
    end
    println "first error:", pool.GroupWait(g)
    pool.GroupStop(g)

    p := pool.New(2, 8)
    a := pool.Async(p, (): ret slow sum(1000000) end)
    b := pool.Async(p, (): ret slow sum(2000) end)
    println "a =", pool.Await(a), "b =", pool.Await(b)
    pool.Release(a)
    pool.Release(b)
    pool.Stop(p)
end
```

```output
first error: row 13: price is negative
a = 499999500000 b = 1999000
```

`pool.New(2, 8)` makes a pool with two workers and room for eight waiting tasks; the thread count never grows. `pool.NewElastic(min, max, queue)` adds workers up to `max` when every worker is busy, which is the right choice when tasks may wait on each other. Because tasks are threads, a task must never wait for a task queued behind it on a full fixed pool; `pool.SubmitOrRun` runs the task on the calling thread when the queue is full, which is safe for tasks that submit tasks.

## Channels and select {#channels}

A channel from [chan](/std/chan/) carries values from one thread to another, so the threads share nothing but the channel. `init(n)` gives it room for `n` values; `init(0)` makes a rendezvous channel where `send` waits until a receiver takes the value. After `close()` nothing more can be sent, and once the remaining values are taken, `recv` sets its `ok` argument to false, which is a worker's signal to stop.

```salam
import chan

mut jobs := chan.Chan {} as chan.Chan<i64>
mut results := chan.Chan {} as chan.Chan<i64>

func worker():
    mut ok := true
    until true:
        n := jobs.recv(ok)
        if not ok:
            break
        end
        results.send(n * n)
    end
end

func main:
    jobs.init(8)
    results.init(100)
    mut workers := Vector {} as Vector<i64>
    defer workers.Free()
    repeat 3:
        workers.Push(spawn(worker))
    end
    repeat 1 to 100 in i:
        jobs.send(i as i64)
    end
    jobs.Close()
    each w in workers:
        join(w)
    end
    results.Close()
    all := chan.Drain(results)
    mut sum := 0 as i64
    each v in all:
        sum += v
    end
    println "results:", all.len(), "sum:", sum
    all.Free()
    jobs.Free()
    results.Free()
end
```

```output
results: 100 sum: 338350
```

`chan.Select` waits on several channels at once and returns the index of the one that delivered, `-1` on timeout and `-2` when every channel is closed. Combined with `chan.After(ms)` and `chan.Tick(ms)`, which deliver on a timer, it covers the timeout and heartbeat patterns Go programmers write with `select`:

```salam
import chan
import pool

func main:
    mut fast := chan.New(1) as chan.Chan<i64>
    mut slow := chan.New(1) as chan.Chan<i64>
    mut both := Vector {} as Vector<chan.Chan<i64>>
    defer both.Free()
    both.Push(fast)
    both.Push(slow)

    p := pool.New(2, 4)
    pool.Submit(p, (): fast.send(1 as i64) end)

    mut v := 0 as i64
    which := chan.Select(both, v, 1000)
    println "channel", which, "answered with", v

    which2 := chan.Select(both, v, 50)
    println "second select:", which2

    fast.Close()
    slow.Close()
    println "after closing:", chan.Select(both, v, 50)
    pool.Stop(p)
    fast.Free()
    slow.Free()
end
```

```output
channel 0 answered with 1
second select: -1
after closing: -2
```

Unlike Go's `select`, this is a library function over channels of one element type, not a statement over arbitrary channel operations. `SelectSend`, `TrySelect` and `TrySelectSend` cover the send and non-blocking variants. A channel copies a `Vector` or a pointer as a handle, so both threads share what it points to.

## Cancellation, deadlines and the other primitives {#tools}

The rest of the toolbox follows familiar designs:

- **[ctx](/std/ctx/)** builds a tree of contexts with `WithCancel`, `WithTimeout` and `WithDeadline`. Cancelling a node finishes everything below it, and a child's deadline can only shorten what it inherits. Workers poll `ctx.IsDone` or block in `ctx.Wait`.
- **`sync.RWMutex`** for many readers and an occasional writer (writer-preferring), **`sync.CondVar`** to sleep until a condition holds, **`sync.Semaphore`** to allow at most N threads into a section, and **`sync.Once`** for setup that must run exactly once.
- **`atomic.Flag`** with `TestAndSet` for a one-time stop signal that exactly one caller wins.
- **[events](/std/events/)** for an in-process event emitter with an optional worker thread, and **[cron](/std/cron/)** for scheduled jobs, each on its own thread.

The `sync` primitives block on a condition variable rather than polling, so a waiting thread costs no CPU.

## How Salam's concurrency compares with Go, Rust and Java {#compare}

|                      | Salam                           | Go                           | Rust                                                    | Java                                          |
| -------------------- | ------------------------------- | ---------------------------- | ------------------------------------------------------- | --------------------------------------------- |
| Unit of concurrency  | OS threads, bounded pools       | goroutines on a scheduler    | OS threads, or async tasks with a runtime such as Tokio | OS threads, and virtual threads since Java 21 |
| Spawn syntax         | `go f(x)`, `spawn(f)`           | `go f(x)`                    | `thread::spawn`, `tokio::spawn`                         | executors, `Thread.ofVirtual()`               |
| Channels             | library, with `Select` function | built-in, `select` statement | library (`std::sync::mpsc`, crossbeam)                  | `BlockingQueue` and friends                   |
| async/await          | no                              | no                           | yes                                                     | no (virtual threads instead)                  |
| Data race prevention | your discipline                 | race detector at runtime     | compile-time (`Send`, `Sync`)                           | your discipline                               |
| Garbage collector    | none                            | yes                          | none                                                    | yes                                           |

The honest reading: Salam's `go` keyword borrows Go's syntax, but not its runtime. A goroutine costs a few kilobytes and Go can run hundreds of thousands of them, parking blocked ones cheaply; a Salam task occupies an OS thread while it runs and while it blocks. For CPU-bound parallel work that difference matters little, and having no garbage collector removes pause times entirely. For hundreds of thousands of mostly idle connections, Go's model, or an event loop like the one inside Salam's own HTTP server, is the better fit.

The bigger gap is safety. Rust proves at compile time that data shared between threads is protected; Salam does not, and neither does Go or Java. A missing lock in Salam is a silent bug. The article [Salam vs Go](/blog/salam-vs-go/) compares the two languages more broadly.

## Pitfalls that cause hangs and crashes {#pitfalls}

- **Zero values are not ready.** A `sync.Mutex {}`, `atomic.I64 {}` or a channel that was never initialized crashes on first use. Create them with `sync.NewMutex()`, `atomic.NewI64(0)`, `chan.New(n)` or `init(n)`, for globals at the top of `main` or in the declaration.
- **A mutex is not reentrant.** Locking it twice on the same thread, or calling `sync.Do` inside its own `Once`, hangs.
- **Join exactly once,** and call `Destroy...` only after the last user has been joined.
- **Lambdas capture by value.** A loop body passed to `ParallelFor` cannot add to a local total; write to a slot, an atomic or a locked structure.
- **Print after the joins.** Lines printed from several threads interleave unpredictably.
- **The interpreter is not the reference.** Threads are fully supported by the compiled backends; test concurrent code with `salam run` or `salam build`, and with the [memory checker](/learn/memory/) when something looks wrong.

## Frequently asked questions {#faq}

### Does Salam have goroutines?

It has the `go` keyword, which runs a call on a shared bounded pool of OS threads, one per core. They are not green threads: a blocked task holds its thread. For most CPU-bound work this behaves like goroutines; for very large numbers of blocking tasks it does not.

### Does Salam have async/await?

No. Concurrency uses threads, task pools, channels and futures (`pool.Async` and `pool.Await`). Salam's HTTP server uses an epoll event loop internally on Linux, so handler code stays synchronous.

### Is Salam code free of data races?

Not automatically. The compiler does not check how threads share data, so protecting shared state with a mutex, an atomic or a channel is your responsibility, as in C, Go and Java.

### How many threads should a Salam program use?

For CPU-bound work, about one per core: `pool.CPUCount()` tells you how many, and `go`, `ParallelForAuto` and `pool.NewAuto` size themselves that way. For blocking I/O, use a bounded pool sized for your workload, and `thread.SetLimit` as a safety net.

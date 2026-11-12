---
title: Salam vs Java and C#: native code versus managed runtimes
seo_title: Salam vs Java and C#: Native Code vs Managed Runtimes
description: Salam vs Java and C#: JIT and garbage collection against ahead-of-time native code, with startup, memory, deployment, language features and when to pick each.
keywords: Salam vs Java, Salam vs C#, native vs managed runtime, JIT vs AOT, garbage collection vs manual memory, GraalVM native image, .NET Native AOT, JVM alternative, enterprise programming language
category: compare
summary: Java and C# run on managed runtimes with a JIT compiler and a garbage collector, backed by two of the largest ecosystems in software. Salam compiles ahead of time to native code with no collector. The differences reach from startup time to how a team writes code.
date: 2026-11-11
---

Comparing Salam vs Java and C# is really comparing two ways to run a program. **Java and C# are managed languages: the compiler produces bytecode (JVM bytecode or .NET IL), and a runtime loads it, compiles hot code to machine code with a just-in-time (JIT) compiler and frees memory with a garbage collector. Salam compiles ahead of time (AOT) to a native executable, with no virtual machine, no JIT and no garbage collector.** Both Java and .NET can now also be compiled ahead of time, which narrows the gap in some places and not in others.

This article explains what that difference means for startup, memory, deployment and day-to-day code, shows the same task in all three languages, and is direct about the large advantages Java and C# have in ecosystem and tooling. The Salam programs are compiled and run when this site is built; the Java example runs on Java 21 and the C# examples on .NET 8 or later.

## Managed runtimes and native code {#runtime-models}

The JVM and the .NET runtime are among the most heavily engineered pieces of software in existence. A tiered JIT starts by interpreting or quickly compiling methods, watches which code is hot, and recompiles it with aggressive optimizations based on what actually happens at runtime. Generational garbage collectors (G1 and ZGC on the JVM, the .NET GC) reclaim memory without the programmer thinking about it. Reflection, dynamic class loading and runtime code generation are all available because the runtime is there.

|                      | Java                                                | C#                                                        | Salam                                                           |
| -------------------- | --------------------------------------------------- | --------------------------------------------------------- | --------------------------------------------------------------- |
| Compiled to          | JVM bytecode, then JIT                              | .NET IL, then JIT                                         | Native machine code, ahead of time                              |
| Ahead-of-time option | GraalVM Native Image; AOT cache work in recent JDKs | Native AOT, ReadyToRun                                    | The only mode                                                   |
| Memory               | Garbage collected                                   | Garbage collected                                         | Stack values; heap collections freed with `.Free()` and `defer` |
| Deployment           | A JVM plus your JARs, or a native image             | The .NET runtime plus your assemblies, or a native binary | One executable                                                  |
| Generics             | Erased at runtime                                   | Reified, specialized for value types                      | Specialized per type at compile time                            |
| Errors               | Exceptions, checked and unchecked                   | Exceptions                                                | `throws`, `try`, `catch`, no unwinding                          |
| Ecosystem            | Maven Central, Spring, Jakarta EE                   | NuGet, ASP.NET Core, Entity Framework                     | A large standard library, few third-party packages              |

## The same task in three languages {#same-task}

Group orders by customer and print each customer's total, sorted by name. In Java 21, a record holds the data and a stream does the grouping:

```java
import java.util.List;
import java.util.Map;
import java.util.TreeMap;
import java.util.stream.Collectors;

public class Totals {
    record Order(String customer, double amount) {}

    public static void main(String[] args) {
        List<Order> orders = List.of(
            new Order("sara", 12.5),
            new Order("ali", 7.0),
            new Order("sara", 30.0));

        Map<String, Double> totals = orders.stream()
            .collect(Collectors.groupingBy(Order::customer, TreeMap::new,
                Collectors.summingDouble(Order::amount)));

        totals.forEach((name, sum) -> System.out.printf("%s %.2f%n", name, sum));
    }
}
```

C# with top-level statements and LINQ is shorter still, and its `decimal` type is the natural choice for money:

```csharp
var orders = new List<Order>
{
    new("sara", 12.5m),
    new("ali", 7.0m),
    new("sara", 30.0m),
};

foreach (var g in orders.GroupBy(o => o.Customer).OrderBy(g => g.Key))
{
    Console.WriteLine($"{g.Key} {g.Sum(o => o.Amount):F2}");
}

record Order(string Customer, decimal Amount);
```

Salam has no stream or LINQ library, so the grouping is an explicit loop over a `HashMap`. The map and the vector of names live on the heap and are freed by `defer` when `main` returns:

```salam
import conv
import sort

struct Order:
    pub customer: str
    pub amount: f64
end

func main:
    orders := [
        Order { customer = "sara", amount = 12.5 },
        Order { customer = "ali", amount = 7.0 },
        Order { customer = "sara", amount = 30.0 },
    ]
    mut totals := HashMap {} as HashMap<str, f64>
    defer totals.Free()
    each o in orders:
        totals.put(o.customer, totals.GetOr(o.customer, 0.0) + o.amount)
    end
    mut names := Vector {} as Vector<str>
    defer names.Free()
    each name, _ in totals:
        names.push(name)
    end
    sort.Sort(names)
    each name in names:
        println name, conv.FormatFloatPrec(totals.get(name), 2)
    end
end
```

```output
ali 7.00
sara 42.50
```

The Salam version is the longest of the three, and that is a fair summary of the trade: Java and C# give you rich collection libraries and a collector that cleans up, while Salam gives you plain loops and explicit ownership. For real money, Salam's [decimal package](/std/decimal/) is the equivalent of C#'s `decimal`.

## Startup time, memory and deployment {#startup-memory}

A JIT needs time to become fast. A JVM or .NET process starts by loading the runtime and class libraries, and code runs slower until the JIT has compiled the hot paths. For a server that runs for weeks this does not matter, and the JIT's profile-guided optimizations can produce excellent steady-state performance. For command-line tools, serverless functions that start for each request, and containers that scale up and down, startup and warm-up are a real cost, and so is the memory a managed heap needs to run well.

Both platforms have answers. GraalVM Native Image compiles a Java application ahead of time into a native executable that starts quickly, at the price of a closed-world assumption: reflection, dynamic proxies and resources have to be known at build time or declared in configuration. Recent JDKs (24 and 25) also shipped the first pieces of Project Leyden's ahead-of-time cache, which keeps the normal JVM but saves class loading and profiling work between runs. On .NET, Native AOT produces a self-contained native executable, and ASP.NET Core supports it for minimal APIs since .NET 8; it requires trim-compatible code and gives up runtime code generation. Both AOT options still include a garbage collector.

Salam starts from the other end. `salam build` produces one native executable; there is no runtime to install and nothing to warm up, and memory is what the program allocates, with no collector heap on top. The flip side is that there is no JIT either: the code is optimized once, at build time, and cannot adapt to the workload the way HotSpot or RyuJIT can.

Neither Java nor C# is part of Salam's published [benchmark](/speed/), so there are no measured numbers for this comparison. The benchmark does compare Salam with C, C++, Rust and Go on 39 short programs (Salam averaged 10.6 ms, C 11.7 ms and Rust 11.3 ms, with the caveat that start-up is part of each time and gaps under about 5% are noise). Short programs like those flatter AOT languages against JIT ones, so they would not be a fair test of a JVM's steady state anyway.

## Garbage collection vs explicit freeing {#gc}

A garbage collector removes a whole class of bugs: you cannot forget to free memory, and you cannot free it twice. Modern collectors such as ZGC and the .NET server GC keep pauses short even on large heaps. The costs are memory overhead (a collector needs headroom to work efficiently), some unpredictability, and CPU time spent collecting.

Salam frees memory explicitly. Numbers, structs and fixed arrays live on the stack; `Vector`, `HashMap` and other heap collections are released with `.Free()`, normally written as `defer x.Free()` right after creation. That gives predictable memory use and no pauses, and it asks the programmer for discipline. The compiler helps by reporting some use-after-free and double-free patterns, and `salam memcheck` runs a program under Valgrind or AddressSanitizer, but Salam does not prove memory safety, and a forgotten `free` is a leak. For a team used to a collector, that is the biggest adjustment. [Memory management in Salam](/blog/memory-management-in-salam/) and [Native compilation without a VM or garbage collector](/blog/native-compilation-without-vm-or-gc/) go deeper.

## A web endpoint {#web}

Web services are where Java and C# dominate in business software. An ASP.NET Core minimal API is hard to beat for brevity:

```csharp
var builder = WebApplication.CreateBuilder(args);
var app = builder.Build();

app.MapGet("/hello/{name}", (string name) => new { message = $"Hello, {name}" });

app.Run("http://localhost:8080");
```

Spring Boot on the Java side takes a little more ceremony but brings dependency injection, data access, security and observability that large organizations rely on. The same endpoint in Salam uses the [HTTP server](/std/net/http/) and router from the standard library, and the response struct is encoded by a JSON codec the compiler derives:

```salam
import net.http
import net.router
import encoding.json

struct Greeting:
    pub message: str
end

func hello(ctx: i64):
    name := http.Param(ctx, "name")
    http.Json(ctx, json.Marshal(Greeting { message = "Hello, " + name }))
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/hello/:name", hello)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

The server runs until it is stopped, so it is type-checked here rather than run. The standard library also has middleware, sessions, validation, OpenAPI documentation and drivers for PostgreSQL, MySQL, SQLite, Redis and ClickHouse, but nothing close to the breadth of Spring or ASP.NET Core, and no ORM like Hibernate or Entity Framework. Salam's approach to databases is typed SQL instead; see [Database applications in Salam](/blog/typed-sql-database-apps/).

## Language features compared {#features}

Java and C# have both moved toward the features Salam was designed with. Java 21 has records, sealed interfaces and pattern matching in `switch`, which together give it real sum types. C# has records, switch expressions and pattern matching, and its designers have been working on union types. Salam has enums that carry data and an exhaustive `match` from the start, which you can read about in [the enums lesson](/learn/enums/).

Generics differ underneath. Java erases type arguments at runtime, so a `List<String>` and a `List<Integer>` are the same class. C# keeps them and specializes generic code for value types. Salam specializes every generic function and struct per type at compile time, like C++ templates but with interface bounds instead of duck typing.

Errors are another split. Java and C# use exceptions that unwind the stack. Salam's `throws`, `try` and `catch` look similar, but an error is returned like a value: every call that can fail must be marked `try` or sit inside a `try` block, `defer` statements run, and nothing is unwound.

Reflection and annotations drive much of the Java and .NET world: dependency injection, serializers and ORMs inspect types while the program runs. Salam has no runtime reflection. Its JSON and JSON Schema support is generated by the compiler from the struct declaration, and dependency injection, where you want it, is done with explicit providers.

Concurrency: Java 21's virtual threads and C#'s `async`/`await` let one process handle very many concurrent operations with simple code. Salam has threads, a bounded task pool with a `go` keyword, channels, futures and atomics, but no `async`/`await` and no virtual threads. See [Concurrency in Salam](/blog/concurrency-in-salam/).

## Ecosystem, tooling and teams {#ecosystem}

This is where the comparison is least close. Java and C# have decades of libraries, frameworks for every enterprise pattern, IDEs such as IntelliJ IDEA, Visual Studio and Rider with deep refactoring support, profilers, monitoring agents, long-term support releases from several vendors, and millions of developers. Banks, governments and large companies run on them, and their security and compliance processes are built around them.

Salam has a single-executable toolchain with a language server, a formatter, a test runner, a debugger launcher and a package manager, plus plugins for Visual Studio Code and ten other editors ([the toolchain](/blog/salam-developer-tools/)). It is pre-1.0, its community is small, its third-party package registry is new and nearly empty, and there are few production deployments. A company choosing a language for a ten-year system has to weigh that. The article [What makes a programming language production ready?](/blog/what-makes-a-language-production-ready/) puts Salam against that checklist.

## When to choose which {#choose}

**Choose Java or C# when:**

- you build business systems that depend on Spring, Jakarta EE, ASP.NET Core, an ORM or other large frameworks;
- you need long-term support, vendor backing and a large hiring pool;
- the service runs for a long time, so JIT warm-up does not matter and peak performance does;
- your team values a garbage collector over control of memory.

**Choose Salam when:**

- you want one native executable with fast startup, predictable memory and no runtime to install;
- you write command-line tools, small services, edge functions or system components where a managed runtime is overhead;
- you are comfortable freeing heap memory explicitly and want sum types and strict compile-time checks;
- you want to program in Persian as well as English, for teaching or for a local team.

## Frequently asked questions {#faq}

### Is Salam faster than Java or C#?

There is no published benchmark that compares them. Salam avoids JIT warm-up and garbage collection, which helps startup and memory, while a JVM or .NET JIT can reach very high steady-state performance on long-running servers. Measure your own workload.

### Does Salam have a garbage collector?

No. Values on the stack need nothing, and heap collections are freed with `.Free()`, usually through `defer`. The compiler and `salam memcheck` help find mistakes.

### Is GraalVM Native Image or .NET Native AOT the same as Salam's compilation?

All three produce native executables ahead of time. The Java and .NET options still bundle a garbage collector and restrict reflection and runtime code generation, because those languages were designed around a runtime. Salam was designed for ahead-of-time compilation from the start.

### Can Salam be used for enterprise software?

It can build services, APIs and database applications, but it is pre-1.0 and lacks the frameworks, vendor support and community of Java and .NET. For most enterprise systems today, Java or C# is the lower-risk choice.

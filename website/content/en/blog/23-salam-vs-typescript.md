---
title: Salam vs TypeScript: one language for the browser and the server?
seo_title: Salam vs TypeScript: Types, Web Pages and Servers Compared
description: Salam vs TypeScript for full stack work: erased types vs checked ones, JSON at the boundary, Node vs a native server, and how each builds web pages.
keywords: Salam vs TypeScript, TypeScript alternative, full stack language, one language frontend and backend, compile to JavaScript, typed web development, Node.js alternative, Salam web development, HTML generation language
category: compare
summary: TypeScript runs everywhere JavaScript does and owns the largest ecosystem in software. Salam takes another route to one language for the whole stack: a native server, a page language and a JavaScript backend. Here is how they differ in practice.
date: 2026-11-11
---

Salam vs TypeScript is a comparison between two answers to the same wish: write the server and the browser code in one language. **TypeScript is a typed layer over JavaScript; its types are checked by the compiler and then erased, and the program runs on a JavaScript engine such as Node.js, Deno, Bun or a browser. Salam is a statically typed language that compiles to native code for the server and generates HTML, CSS and JavaScript for the browser.** Both are "full stack languages", but they get there from opposite ends.

This article compares them on the things that matter for web work: what the type system promises, what happens when JSON arrives from outside, how the server-side runs, how pages are built, and what each ecosystem gives you. The code on both sides solves the same small tasks, and every Salam example is compiled and run when this site is built.

## The short answer {#short-answer}

If you are building a typical web product today, with a team that knows JavaScript and a long list of npm packages it depends on, TypeScript is the safe choice and Salam is not a replacement for it. If you want a server that compiles to a single native executable, a type system whose guarantees still hold while the program runs, and pages that are checked at compile time without a JavaScript build chain, Salam is worth a serious look.

|                   | TypeScript                                              | Salam                                                            |
| ----------------- | ------------------------------------------------------- | ---------------------------------------------------------------- |
| Runs on           | Any JavaScript engine (browser, Node.js, Deno, Bun)     | Native code on the server; HTML, CSS and JS in the browser       |
| Types at runtime  | Erased; JavaScript values only                          | Kept in the compiled program; structs have a fixed layout        |
| Type system       | Structural, gradual (`any`, casts), very expressive     | Static, no `any`; interfaces are structural, structs are nominal |
| Sum types         | Discriminated unions by convention                      | Enums with data, checked `match`                                 |
| Server            | Node.js and frameworks such as Express, Fastify, NestJS | `net/http` and `net/router` in the standard library              |
| Pages             | JSX/TSX with React, Vue, Svelte and others              | The built-in page language, `salam page build`                   |
| Package ecosystem | npm, the largest package registry there is              | Large standard library, very small third-party ecosystem         |
| Maturity          | Released in 2012, used across the industry              | Pre-1.0, few production deployments                              |

## Two ideas of what a type is {#type-systems}

TypeScript's types describe JavaScript values. The compiler checks your code against them and then removes them, so the program that runs is plain JavaScript. That design is the reason TypeScript could be adopted file by file in existing JavaScript projects, and the reason its type system is so expressive: conditional types, mapped types and template literal types can describe almost any shape a JavaScript library returns. The cost is that the types are a promise made at compile time, and some escape hatches (`any`, a type assertion with `as`, an unchecked index) let a wrong value through.

Salam's types are the program's types. A `struct` has a fixed set of fields and a fixed layout in memory, an `int` is a 32-bit integer, and there is no `any`. Here is the same small task in both languages: describe a stream of order events, where each kind of event carries different data.

```typescript
type OrderEvent =
  | { kind: 'placed'; id: number; total: number }
  | { kind: 'shipped'; id: number; carrier: string }
  | { kind: 'cancelled'; id: number; reason: string };

function describe(e: OrderEvent): string {
  switch (e.kind) {
    case 'placed':
      return `order ${e.id} placed, total ${e.total}`;
    case 'shipped':
      return `order ${e.id} shipped with ${e.carrier}`;
    case 'cancelled':
      return `order ${e.id} cancelled: ${e.reason}`;
    default: {
      const unreachable: never = e;
      return unreachable;
    }
  }
}

const events: OrderEvent[] = [
  { kind: 'placed', id: 17, total: 42.5 },
  { kind: 'shipped', id: 17, carrier: 'DHL' },
  { kind: 'cancelled', id: 18, reason: 'out of stock' },
];

for (const e of events) console.log(describe(e));
```

The TypeScript version is a discriminated union: an object type per case with a shared `kind` tag, a `switch` on the tag, and the `never` trick in the `default` branch so that adding a fourth case without handling it becomes a compile error. It works well, and it is a pattern you have to know.

In Salam the same idea is a language feature, an [enum whose members carry data](/learn/enums/):

```salam
enum OrderEvent:
    Placed(id: int, total: f64)
    Shipped(id: int, carrier: str)
    Cancelled(id: int, reason: str)
end

func describe(e: OrderEvent): str:
    ret match e:
        Placed(id, total): "order " + id + " placed, total " + total end
        Shipped(id, carrier): "order " + id + " shipped with " + carrier end
        Cancelled(id, reason): "order " + id + " cancelled: " + reason end
    end
end

func main:
    events := [
        OrderEvent.Placed(17, 42.5),
        OrderEvent.Shipped(17, "DHL"),
        OrderEvent.Cancelled(18, "out of stock"),
    ]
    each e in events:
        println describe(e)
    end
end
```

```output
order 17 placed, total 42.5
order 17 shipped with DHL
order 18 cancelled: out of stock
```

`match` must cover every member or have an `else`, so a missing case is an error without any trick. The members are values the compiler knows about, which also lets the JSON encoder write them and the printer show them.

Where TypeScript is clearly ahead is expressiveness. Salam has generics with interface bounds, `T?` optionals and `A or B` unions of types, but nothing like mapped or conditional types, and no type-level programming. For describing the shape of an existing, dynamic JavaScript API, TypeScript's system is the better tool. The article [Static typing in Salam](/blog/static-typing-in-salam/) covers what Salam's checker does instead.

## Data at the boundary: JSON {#json}

The difference between erased and kept types shows most clearly where data enters the program. In TypeScript, `JSON.parse` returns `any`, and a cast tells the compiler what you hope the data contains:

```typescript
interface Signup {
  email: string;
  age: number;
}

const s = JSON.parse('{"email": "ali@example.com"}') as Signup;
console.log(s.age + 1); // NaN, and no error anywhere
```

The program compiles and prints `NaN`, because nothing checked that `age` was there. TypeScript projects solve this with a runtime validation library such as Zod, Valibot or io-ts, which describes the shape a second time as a value and checks it while the program runs. That works and is common practice, but the check is something you add, not something the language does.

In Salam, `json.Unmarshal` decodes into a struct whose fields the compiler knows, and it fails when the text does not fit:

```salam
import encoding.json

struct Signup:
    pub email: str
    pub age: int
end

func load(text: str):
    mut s := Signup { email = "", age = 0 }
    try:
        json.Unmarshal(text, s)
        println "accepted:", s.email, s.age
    catch e:
        println "rejected:", e.to_str()
    end
end

func main:
    load(`{"email": "sara@example.com", "age": 31}`)
    load(`{"email": "ali@example.com"}`)
    load(`{"email": "reza@example.com", "age": "31"}`)
end
```

```output
accepted: sara@example.com 31
rejected: json: missing key 'age' (line 1, column 1)
rejected: json: value does not match the expected type (line 1, column 38)
```

The decoder is derived from the struct declaration, so there is one description of the shape, not two. `UnmarshalLenient` exists for input where missing keys should keep their defaults, and `json.Schema` derives a JSON Schema from the same struct. The [encoding/json reference](/std/encoding/json/) lists the options.

## The server-side: Node.js or a native binary {#server}

A TypeScript server runs on a JavaScript runtime. Node.js is the default, Deno and Bun run TypeScript files directly, and recent Node.js versions can strip type annotations and run `.ts` files without a separate build step. The engine compiles hot code with a JIT and manages memory with a garbage collector, and one event loop handles many connections through asynchronous I/O. Here is a small JSON endpoint with Express 5:

```typescript
import express from 'express';

interface Product {
  id: number;
  name: string;
  price: number;
}

const app = express();

app.get('/products/:id', (req, res) => {
  const p: Product = {
    id: Number(req.params.id),
    name: 'Notebook',
    price: 4.5,
  };
  res.json(p);
});

app.listen(8080);
```

The same endpoint in Salam uses the standard library's [HTTP server](/std/net/http/) and router. A handler receives a request context, and `json.Marshal` writes the struct:

```salam
import net.http
import net.router
import encoding.json

struct Product:
    pub id: int
    pub name: str
    pub price: f64
end

func product(ctx: i64):
    id := http.Param(ctx, "id")
    p := Product { id = id.to_int(), name = "Notebook", price = 4.5 }
    http.Json(ctx, json.Marshal(p))
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/products/:id", product)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

The program runs until it is stopped, so it is type-checked here rather than run. Built with `salam build`, it becomes one native executable. There is no runtime to install on the server, no `node_modules` folder to ship and no JIT warm-up.

How much faster is it? The project's [HTTP router benchmark](/speed/) ran nine servers on the same eleven routes, interleaved, on one machine. Salam's `net.http` reached a geometric mean of 94,850 requests per second, and Node 24 with Express 5 reached 22,536, which the page rounds to 4.2 times. Read that with its caveats: the machine was a shared 6-vCPU sandbox with unrelated load, Express is a convenient framework rather than Node's fastest option, and the page itself asks you to rerun the harness on your own hardware before planning capacity with it. The article [Building fast web servers with Salam](/blog/fast-web-servers-with-salam/) goes through the server-side in detail.

What Node has that Salam does not is the async model and its libraries. Salam has no `async`/`await`; a Salam server handles concurrency with a pool of worker threads, and your own code uses threads, a task pool and channels (see [Concurrency in Salam](/blog/concurrency-in-salam/)). If your service is mostly waiting on other services, Node's model is a good fit and its client libraries for every cloud API are hard to beat.

## The browser side: pages and JavaScript {#browser}

On the frontend TypeScript is the default language of most frameworks. You write components in TSX, a bundler such as Vite turns them into JavaScript, and React, Vue, Svelte or Angular takes care of rendering and state. The ecosystem around them, from component libraries to testing tools, is enormous.

Salam's approach is different. It has a [page language](/learn/layout/) built into the compiler: you describe the page as a tree of elements, and `salam page build` turns it into HTML, CSS and, when there are event handlers, JavaScript. Every element, attribute and CSS value is checked at compile time. A layout with parameters becomes an ordinary function that returns HTML, with every value escaped, so the same page can be rendered by a Salam server for each request:

```salam
component Tag(text: str):
    span: content = text background = "#fde68a" end
end

page Cart(user: str, products: Vector<str>):
    title = "Cart of " + user
    heading: size = 1 content = user + "'s cart" end
    if products.is_empty():
        paragraph: content = "Your cart is empty" end
    else:
        paragraph: content = "Items: " + products.len() end
        list:
            each name in products:
                item: content = name end
            end
        end
    end
end

func main:
    mut products := Vector {} as Vector<str>
    defer products.Free()
    products.push("Pen")
    products.push("<script>alert(1)</script>")
    println Cart("Sara", products)
    println Tag("sale")
end
```

```output
<!doctype html><html lang="en" dir="ltr"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>Cart of Sara</title></head><body><h1>Sara&#39;s cart</h1><p>Items: 2</p><ul><li>Pen</li><li>&lt;script&gt;alert(1)&lt;/script&gt;</li></ul></body></html>
<style>.tag-a{background:#fde68a}</style><span class="tag-a">sale</span>
```

The script tag in the data comes out as text. The layout function is `pure`, so it cannot read files or the network while rendering; the handler fetches the data and passes it in. The article [Web development with Salam](/blog/web-development-with-salam/) walks through the page language from static pages to interactive components.

For code that has to run in the browser, `salam js` compiles Salam to JavaScript, and `salam web` builds a page together with the Salam scripts it uses. Event handlers written in the page language go through the same JavaScript backend. One limit is worth knowing: JavaScript numbers are doubles, so on this backend an `i64` keeps only 53 bits of precision. `9007199254740993` printed from a native build prints `9007199254740992` from `salam js`, which is also what a TypeScript `number` would do. For whole programs without a JavaScript layer at all, Salam can target WebAssembly; the article [From native to WebAssembly](/blog/salam-webassembly-and-javascript/) covers those targets.

What Salam does not have is a component framework ecosystem. There is no Salam equivalent of the React library world, and the page language's interactive model (state in variables, handlers that change it, re-rendering only what changed) is young. For a large, highly interactive single-page application, TypeScript with a mature framework is the realistic choice today.

## Tooling, build steps and ecosystem {#ecosystem}

A TypeScript project usually involves several tools: the `tsc` type checker, a runtime, a package manager (npm, pnpm, Yarn or Bun), a bundler, a linter, a formatter and a test runner. Each is excellent and each is a separate piece of configuration. Microsoft announced in 2025 that it is porting the TypeScript compiler to Go to make type checking much faster on large code bases, which says something about how much time big projects spend in that step.

Salam puts the toolchain in one executable: `salam build`, `salam check`, `salam test`, `salam format`, `salam page build`, a language server for editors and a package manager that adds packages from GitHub. The standard library covers what a web backend usually pulls from npm: HTTP, routing, middleware, sessions, JSON, JWT, WebSocket, database drivers and more.

The other side of that coin is the ecosystem. npm has packages for almost every task and every third-party API. Salam's own package registry is new and nearly empty, so anything outside the standard library you either write yourself or reach through Salam's C interop. Hiring is also different: almost every web developer can read TypeScript, while Salam developers are few.

## Where each one is the better choice {#choose}

**Choose TypeScript when:**

- the project depends on npm packages, a frontend framework or a cloud SDK;
- the team already knows JavaScript, and hiring matters;
- you are adding types to an existing JavaScript codebase, which is exactly what TypeScript was designed for;
- the frontend is a large, highly interactive application.

**Choose Salam when:**

- you want the server to be a single native executable with predictable memory and no runtime to deploy;
- you want types that also describe the data at runtime, with decoders derived from them;
- your pages are content, forms and dashboards that benefit from compile-time checking of every element and style, without a JavaScript build chain;
- you want to write both the server and the pages in Persian or English with one toolchain.

It is also reasonable to mix them: a Salam API server behind a TypeScript frontend, or a TypeScript service next to a Salam worker that does CPU-heavy work. They talk JSON over HTTP like any two services.

## Frequently asked questions {#faq}

### Can Salam replace TypeScript for frontend development?

Not for large framework-based applications. Salam's page language builds checked HTML, CSS and JavaScript and supports components with state and events, but it has nothing like the React or Vue ecosystems. For content pages, forms and dashboards it can replace a hand-written frontend.

### Is a Salam server faster than a Node.js server?

In the project's router benchmark Salam answered about 4.2 times as many requests per second as Node with Express, measured on a shared machine. That is one benchmark with stated limits, so measure your own workload before deciding.

### Does Salam compile to JavaScript like TypeScript does?

Yes, `salam js` compiles a Salam program to JavaScript for the browser, and page event handlers use the same backend. Unlike TypeScript, which is JavaScript plus types, Salam is a separate language, so some details differ, such as `i64` values being limited to 53 bits of precision there.

### Can I use npm packages from Salam?

No. Salam programs use the standard library, packages added with `salam add` from GitHub, and C libraries through `extern` declarations. A Salam service can still work beside Node.js services through HTTP.

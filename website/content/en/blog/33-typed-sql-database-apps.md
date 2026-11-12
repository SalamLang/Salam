---
title: Database applications in Salam: typed SQL, query checking and generated code
seo_title: Typed SQL and SQL Code Generation in Salam | Salam Magazine
description: Salam checks SQL against your schema before the program runs and generates typed query functions, sqlc style, on top of SQLite, MySQL and PostgreSQL drivers.
keywords: typed SQL, SQL code generation, database driver, sqlc alternative, SQL linter, check SQL at compile time, Salam database, PostgreSQL driver, MySQL driver, SQLite, database migrations
category: build
summary: How Salam talks to databases: one connection interface over SQLite, MySQL and PostgreSQL, migrations at start-up, a SQL checker that reads your schema, and a generator that turns annotated queries into typed functions.
date: 2026-11-11
---

Typed SQL means that the types of a query's parameters and results come from the database schema, and that a misspelled column or table is caught before the query ever reaches a database. Salam provides this in its standard library: **`salam sql check` reads the SQL inside your `.salam` and `.sql` files and checks it against your `CREATE TABLE` statements, and `salam sql generate` turns annotated queries into typed Salam functions and row structs, in the spirit of Go's sqlc.** Underneath sit database drivers for SQLite, MySQL, MariaDB and PostgreSQL behind one shared interface.

This article follows a database application from the connection to generated code, with programs you can run as they stand. None of them needs a database server: they use SQLite in memory, or no database at all.

## Why query strings are a source of bugs {#problem}

In most languages, SQL is a string. The compiler does not look inside it, so a column renamed in a migration, a table name in the plural, or a placeholder of the wrong dialect is found by the database, at runtime, on whichever code path nobody tested. ORMs avoid the problem by generating the SQL, at the cost of a second query language and less control. Tools such as sqlc for Go took a different route: keep writing SQL, but parse it, check it against the schema, and generate typed code.

Salam builds that route into the toolchain. The schema is the DDL you already have in your migrations, so there is no separate model file to keep in step.

## One interface over SQLite, MySQL and PostgreSQL {#drivers}

Each engine has its own package, and each of them can be turned into the shared `dyn db.Connection` from the [db package](/std/db/):

| Package                          | Engine                       | How it connects                                                              |
| -------------------------------- | ---------------------------- | ---------------------------------------------------------------------------- |
| [db.sqlite](/std/db/sqlite/)     | SQLite, a file or `:memory:` | wraps the SQLite C library, with `Prepare` and `Bind`                        |
| [db.mysql](/std/db/mysql/)       | MySQL and MariaDB            | speaks the client protocol in Salam, with TLS and utf8mb4; no libmysqlclient |
| [db.postgres](/std/db/postgres/) | PostgreSQL                   | implements the wire protocol in Salam, with SCRAM-SHA-256 and TLS; no libpq  |

Code written against `dyn db.Connection` runs on all three, which is how tests can use an in-memory SQLite database while production uses PostgreSQL. The interface gives `Exec`, `Query` (rows read with `Next`, `Int`, `Text`, `Float` and `IsNull`), `Begin` for transactions, `LastInsertId`, `Error` and `Kind`. Helpers such as `db.Run`, `db.QueryInt` and `db.QueryText` cover the one-line cases, and `db.InTx` runs a list of statements all or nothing.

Beyond SQL, the same family has clients for [Redis](/std/db/redis/), [ClickHouse](/std/db/clickhouse/), and the search engines [Elasticsearch](/std/db/elasticsearch/), [OpenSearch](/std/db/opensearch/), [Meilisearch](/std/db/meilisearch/) and [Typesense](/std/db/typesense/).

## Migrations and safe quoting {#migrations}

Schema changes are numbered migrations with an up and a down statement. Call `db.MigrateUp` every time the program starts: it records the applied version in a table of its own, so the first run creates the schema, later runs do nothing, and a release that adds migration 3 applies only that one.

```salam
import db
import db.sqlite

const SCHEMA_V1 := "CREATE TABLE customers (id INTEGER PRIMARY KEY AUTOINCREMENT, email TEXT NOT NULL UNIQUE, name TEXT)"
const SCHEMA_V2 := "CREATE TABLE orders (id INTEGER PRIMARY KEY AUTOINCREMENT, customer_id INTEGER NOT NULL, total_cents INTEGER NOT NULL, note TEXT)"

func migrate(c: dyn db.Connection):
    mut m := db.NewMigrator()
    db.AddMigration(m, 1, "customers", SCHEMA_V1, "DROP TABLE customers")
    db.AddMigration(m, 2, "orders", SCHEMA_V2, "DROP TABLE orders")
    before := db.SchemaVersion(m, c)
    applied := db.MigrateUp(m, c)
    println "schema", before, "->", db.SchemaVersion(m, c), "(" + applied + " applied)"
    db.FreeMigrator(m)
end

func add customer(c: dyn db.Connection, email: str, name: str): i64:
    sql := "INSERT INTO customers (email, name) VALUES (" + db.QuoteLiteral(email) + ", " + db.QuoteLiteral(name) + ")"
    if not db.Run(c, sql):
        println "insert failed:", c.Error()
        ret 0
    end
    ret c.LastInsertId()
end

func main:
    c := sqlite.Conn(sqlite.Open(":memory:"))
    migrate(c)
    migrate(c)
    id := add customer(c, "sara@example.com", "Sara")
    add customer(c, "omid@example.com", "Omid O'Neil")
    add customer(c, "sara@example.com", "Sara again")
    println "customers:", db.QueryInt(c, "SELECT COUNT(*) FROM customers")
    rows := c.Query("SELECT id, name FROM customers ORDER BY id")
    until rows.Next():
        println "#" + rows.Int(0), rows.Text(1)
    end
    _ := rows.Close()
    println "first id:", id
    _ := c.Close()
end
```

```output
schema 0 -> 2 (2 applied)
schema 2 -> 2 (0 applied)
insert failed: UNIQUE constraint failed: customers.email
customers: 2
#1 Sara
#2 Omid O'Neil
first id: 1
```

The second `migrate` call applies nothing, the duplicate email is refused by the `UNIQUE` constraint with the engine's own message, and the apostrophe in `O'Neil` survives because the value went through `db.QuoteLiteral`. Never glue raw user input into SQL. On MySQL, quote with the connection's own `c.Quote`, which also escapes backslashes; on SQLite, prepared statements with `sqlite.Prepare` and `BindText` are better still.

The [db reference](/std/db/) lists the traps worth knowing: `MigrateUp` compares version numbers, so a migration numbered below the highest applied one is skipped without a word; MySQL commits DDL statements one by one, so a failed migration there can leave part of it applied; and migration SQL is split at semicolons, so a PostgreSQL function body needs `c.Exec` instead.

## Checking SQL without a database {#check}

The [db.sql package](/std/db/sql/) lexes and parses SQL for four dialects (SQLite, MySQL, MariaDB and PostgreSQL). Hand it your DDL and it builds a catalog of tables and columns; then every query can be checked against it. The command-line form reads the SQL string literals out of your Salam source as well as your `.sql` files:

```bash
$ salam sql check src --schema=db
src/report.salam:6:33: error[SQL311]: no column named "totl_cents" in this statement
  help: did you mean "total_cents"?
src/report.salam:8:24: warning[SQL320]: this DELETE has no WHERE clause and removes every row
  help: add a WHERE clause, or use TRUNCATE when that is what you mean
src/report.salam:9:50: error[SQL317]: no table named "customer"
  help: did you mean "customers"?
checked 3 statement(s) in 1 file(s): 2 error(s), 1 warning(s)
```

That is real output for a three-line program with three typical mistakes. Positions point into the Salam file, and the command exits with status 1, so it fits in CI next to `salam check`. Without `--dialect`, each file is read with the engine it imports, so a project that uses both `db.sqlite` and `db.mysql` is checked correctly with no flags. A query assembled from pieces with `+` is counted as a fragment and skipped rather than reported, and the summary says how many were skipped, so nothing is ignored silently.

The same checks are available as a library, which is useful in tests. This program loads a PostgreSQL schema and runs four queries through it, then asks what one of them takes and returns:

```salam
import db.sql

func report(q: str, schema: sql.Catalog):
    mut r := sql.Check(q, schema)
    if r.ok and r.warnings == 0:
        println "ok   ", q
    else:
        println "bad  ", q
        each d in r.items:
            println "     ", d.code, d.message
        end
    end
    sql.FreeReport(r)
end

func main:
    mut schema := sql.NewCatalog(sql.DialectOf(sql.POSTGRES))
    defer sql.FreeCatalog(schema)
    mut ddl := sql.Apply(schema, """
        CREATE TABLE customers (id BIGSERIAL PRIMARY KEY, email TEXT NOT NULL, name TEXT);
        CREATE TABLE orders (id BIGSERIAL PRIMARY KEY, customer_id BIGINT NOT NULL, total_cents BIGINT NOT NULL);
    """)
    sql.FreeReport(ddl)

    report("SELECT id, phone FROM customers WHERE id = $1", schema)
    report("SELECT id FROM customers WHERE email = ?", schema)
    report("UPDATE orders SET total_cents = 0", schema)
    report("SELECT c.name, o.total_cents FROM orders o JOIN customers c ON c.id = o.customer_id WHERE o.id = $1", schema)

    mut a := sql.Analyze("SELECT c.name, o.total_cents FROM orders o LEFT JOIN customers c ON c.id = o.customer_id WHERE o.id = $1", schema)
    p := a.params.get(0)
    println "takes", p.column, "as", sql.SalamType(p.kind, p.not_null)
    each col in a.results:
        println "gives", col.name, "as", sql.SalamType(col.kind, not col.nullable)
    end
    sql.FreeAnalysis(a)
end
```

```output
bad   SELECT id, phone FROM customers WHERE id = $1
      SQL311 no column named "phone" in this statement
bad   SELECT id FROM customers WHERE email = ?
      SQL201 '?' is not a placeholder in postgres
bad   UPDATE orders SET total_cents = 0
      SQL320 this UPDATE has no WHERE clause and changes every row
ok    SELECT c.name, o.total_cents FROM orders o JOIN customers c ON c.id = o.customer_id WHERE o.id = $1
takes id as i64
gives name as str?
gives total_cents as i64
```

The second query is fine on SQLite and MySQL but wrong on PostgreSQL, which numbers its placeholders `$1`, `$2`; the dialect is not a hint. The analysis at the end shows why this is more than a linter: the `LEFT JOIN` makes every column of `customers` nullable, so `name` comes back as `str?`, while `total_cents` is `NOT NULL` and comes back as a plain `i64`. That is the signature your code should have, derived from the schema instead of remembered.

Diagnostic codes are stable: `SQL0xx` lexical, `SQL1xx` syntax, `SQL2xx` dialect rules, `SQL3xx` names and counts against the schema. Match on the code, not the message.

## Generating typed query functions {#generate}

The last step is mechanical, so the toolchain does it. Annotate a `.sql` file with a name and a kind per query, the way sqlc does:

```sql
-- name: CustomerByEmail :one
SELECT id, email, name FROM customers WHERE email = ?;

-- name: OrdersFor :many
SELECT id, total_cents, note FROM orders WHERE customer_id = ? ORDER BY id;

-- name: AddOrder :exec
INSERT INTO orders (customer_id, total_cents, note) VALUES (?, ?, ?);
```

Then generate:

```bash
$ salam sql generate queries --schema=db --output=gen/store.salam --package=store
wrote gen/store.salam (3 queries)
```

The kinds are `:one` (the first row, with an `ok` out parameter), `:many` (a `Vector` of rows), `:exec` (a `bool`) and `:execrows` (the number of rows changed). Each query is analyzed against the schema first, so a typo stops generation with the query's name in the message and never reaches the generated file. Parameters are named after the column each placeholder is compared with and typed from it.

Below is the generated code for two of the queries, pasted unchanged into a program that uses it, so you can see both what the generator writes and that it runs. In a project you would `include` or import the generated package instead.

```salam
import db
import db.sql
import db.sqlite
import mem

struct OrdersForRow:
    pub id: i64 = 0 as i64
    pub total_cents: i64 = 0 as i64
    pub note: str? = null
end

func fill_query(c: dyn db.Connection, raw: str, a: sql.Args): str:
    mut rep := sql.Report {}
    out := sql.Fill(raw, a, sql.DialectNamed(c.Kind()), rep)
    sql.FreeReport(rep)
    ret out
end

pub func OrdersFor(c: dyn db.Connection, customer_id: i64): Vector<OrdersForRow>:
    mut a := sql.NewArgs()
    sql.AddInt(a, customer_id)
    q := fill_query(c, "SELECT id, total_cents, note FROM orders WHERE customer_id = ? ORDER BY id", a)
    sql.FreeArgs(a)
    mut rows := Vector {} as Vector<OrdersForRow>
    r := c.Query(q)
    if r.Ok():
        until r.Next():
            mut out := OrdersForRow {}
            out.id = r.Int64(0)
            out.total_cents = r.Int64(1)
            if r.IsNull(2):
                out.note = null
            else:
                out.note = r.Text(2)
            end
            rows.push(out)
        end
        r.Close()
    end
    mem.Free(q as void*)
    ret rows
end

pub func AddOrder(c: dyn db.Connection, customer_id: i64, total_cents: i64, note: str): bool:
    mut a := sql.NewArgs()
    sql.AddInt(a, customer_id)
    sql.AddInt(a, total_cents)
    sql.AddText(a, note)
    q := fill_query(c, "INSERT INTO orders (customer_id, total_cents, note) VALUES (?, ?, ?)", a)
    sql.FreeArgs(a)
    done := c.Exec(q) == 0
    mem.Free(q as void*)
    ret done
end

func main:
    c := sqlite.Conn(sqlite.Open(":memory:"))
    _ := db.Run(c, "CREATE TABLE orders (id INTEGER PRIMARY KEY AUTOINCREMENT, customer_id INTEGER NOT NULL, total_cents INTEGER NOT NULL, note TEXT)")
    _ := AddOrder(c, 1 as i64, 1250 as i64, "first order")
    _ := AddOrder(c, 1 as i64, 990 as i64, "it's a gift")
    _ := db.Run(c, "INSERT INTO orders (customer_id, total_cents) VALUES (1, 400)")
    orders := OrdersFor(c, 1 as i64)
    each o in orders:
        println o.id, o.total_cents, o.note.unwrap_or("(no note)")
    end
    orders.free()
    _ := c.Close()
end
```

```output
1 1250 first order
2 990 it's a gift
3 400 (no note)
```

Three things stand out in the generated code. The nullable `note` column becomes a `str?`, and the generated reader checks `IsNull` before reading it, so the caller is forced to decide what a missing note means. The functions take `dyn db.Connection`, so one generated file serves SQLite, MySQL and PostgreSQL, picking each dialect's quoting at runtime from `c.Kind()`. And the values go into the SQL through `sql.Fill`, which lexes the query first, so a `?` inside a string literal is left alone, the number of values must match, and quotes are doubled correctly for the dialect.

That last point is also a limit: `Fill` is substitution, not a server-side prepared statement. It is safe quoting done once and audited, not plan reuse. Only `db.sqlite` offers `Prepare` and `Bind` today.

## Transactions, pools and threads {#pools}

For several statements that must succeed or fail together, `c.Begin()` returns a `dyn db.Tx` with its own `Exec` and `Query`, ended by `Commit` or `Rollback`; queries inside the transaction already see its uncommitted changes. `db.InTx` runs a fixed list of statements as one transaction.

A server keeps a few connections open and lends one to each request. `db.NewPool`, `Add`, `Acquire`, `Conn` and `Release` do the bookkeeping. The pool itself takes no lock, so lease and release from one thread or guard it with a `sync.Mutex`, and never use one connection from two threads at once. The simplest safe pattern for worker threads is one connection per thread. The article on [concurrency in Salam](/blog/concurrency-in-salam/) covers the locks and pools involved.

## How it compares, and what is missing {#compare}

Compared with an ORM such as Django's or Entity Framework, Salam keeps you in SQL and gives no object mapping beyond the generated row structs. Compared with sqlc, the idea is the same and the integration is tighter: the checker ships with the compiler, reads SQL out of ordinary source files, and needs no separate configuration file. Compared with raw drivers in C or Python, you get type information and schema checks before runtime.

The limits are worth stating:

- **Static analysis has edges.** A table whose columns are unknown, such as one created with `CREATE TABLE ... AS SELECT`, is left open and its columns are never reported as missing. Fix syntax errors first, because name checks run only on statements that parse.
- **No prepared statements on MySQL and PostgreSQL** through the shared interface yet; values are quoted into the SQL.
- **Rows are fetched whole on PostgreSQL**, so `RowCount` is known up front, but a huge result is held in memory at once.
- **Young drivers.** They are written in Salam and tested against real MySQL and PostgreSQL servers in the project's release builds, but they have far less production mileage than libpq or the official MySQL connectors.

For the bigger picture of what can be built around this, see [full-stack development with Salam](/blog/full-stack-development-with-salam/) and the [standard library tour](/blog/salam-standard-library-tour/).

## Frequently asked questions {#faq}

### Is there an ORM for Salam?

No. Salam's approach is SQL first: you write the queries, `salam sql check` verifies them against the schema, and `salam sql generate` writes the typed functions and row structs. The result is closer to sqlc than to an ORM.

### Does Salam need libpq or libmysqlclient?

No. The PostgreSQL and MySQL drivers implement the wire protocols in Salam itself, including TLS and modern password authentication. Only SQLite links its C library.

### Can salam SQL check run in CI?

Yes. `salam sql check src --schema=db` exits with status 1 when it finds an error, prints file, line and column for each problem, and needs no database server, only the DDL files.

### Which SQL dialects does Salam understand?

SQLite, MySQL, MariaDB and PostgreSQL. Placeholders, quoting, identity columns and dialect-only syntax such as `ILIKE` or `ON DUPLICATE KEY` are checked per dialect.

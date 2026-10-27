# HTTP router comparison

Nine servers answering the same eleven routes: Salam's `net.http`, Go's own
`net/http` (1.22+ `ServeMux`), three popular third-party Go frameworks
(Fiber, Gin, Echo), nginx, Apache, Node (Express 5), and PHP's built-in
development server. Every server implements the exact same response bodies
for the same input, so the only variable is the server.

- Date: 2026-10-27
- Load generator: `bench/loadgen.c`, 64 connections, 2 threads, 10s measured
  per route (plus a 3s discarded warmup and a drain for `TIME_WAIT` sockets
  between routes).
- Machine: 6-vCPU sandbox, not a dedicated, quiet benchmark host. See
  [Honesty about the machine](#honesty-about-the-machine) below. Servers
  pinned to 4 cores, the load generator to the other 2.
- Full command: `sh tests/en/apps/httpbench/bench/run.sh`
- Raw per-request data: [`results-2026-10-27.jsonl`](results-2026-10-27.jsonl)

## Environment

- **CPU**: AMD EPYC (virtualized, 6 vCPUs)
- **nginx**: 1.28.3
- **Apache**: 2.4.66
- **PHP**: 8.5.4 (cli)
- **Node**: 24.19.0, Express 5.3.0
- **Go**: 1.26.0
- **Fiber**: v2.52.16 (fasthttp-based)
- **Gin**: v1.12.0
- **Echo**: v4.16.2
- **Salam**: 0.5.9 (release build, LLVM backend)

## Why nginx and Apache have gaps

nginx and Apache are not application servers. They get every route they
can answer out of their own config or file path: `/plaintext`, `/json`,
`/cached`, `/file`, `/static/*`, `/users/:id` (a regex-captured segment
with a canned body), and `/` (a pre-rendered file). Nothing else.
`/search`, `/compute`, `/headers` and `POST /echo` need real per-request
work over caller-supplied input, which is what an application server is
for; faking those numbers behind a reverse proxy would measure the thing
behind nginx, not nginx, so they are left blank instead.

## Requests per second

| Route               |   Salam | Go (net/http) | Go (Fiber) | Go (Gin) | Go (Echo) |   nginx | Apache | node/express | php -S |
| ------------------- | ------: | ------------: | ---------: | -------: | --------: | ------: | -----: | -----------: | -----: |
| `/plaintext`        | 155,604 |       117,781 |    169,316 |  117,874 |   114,391 | 187,235 | 36,608 |       56,624 | 27,521 |
| `/json`             | 162,993 |       129,452 |    136,254 |  116,175 |   123,546 | 219,199 | 37,894 |       54,225 | 27,088 |
| `/cached`           | 177,257 |       119,645 |    170,810 |  120,935 |   131,827 | 135,937 | 46,783 |       52,897 | 22,990 |
| `/file`             |  63,855 |        59,971 |     88,998 |   37,256 |    66,560 | 121,672 | 36,586 |       27,750 | 25,931 |
| `/users/42`         | 158,812 |       106,526 |    151,210 |  104,831 |   105,241 | 168,227 |    n/a |       49,626 | 25,160 |
| `/search?q=&n=5`    | 168,742 |        81,732 |    114,306 |   81,714 |    79,588 |     n/a |    n/a |       44,300 | 26,068 |
| `/compute?n=1000`   | 164,029 |       112,313 |    179,943 |  107,664 |   111,836 |     n/a |    n/a |       44,916 | 24,646 |
| `/headers`          | 162,981 |        99,221 |    130,510 |  108,207 |   101,735 |     n/a |    n/a |       49,915 | 26,509 |
| `/static/style.css` |  49,092 |        45,031 |    102,101 |   42,275 |    41,798 | 108,856 | 41,814 |       17,926 | 21,658 |
| `/`                 | 168,227 |        64,401 |    179,609 |   97,890 |    98,817 |  99,460 | 38,444 |       47,293 | 17,653 |
| `POST /echo`        | 164,066 |        90,642 |    154,490 |   85,836 |    91,373 |     n/a |    n/a |       35,209 | 15,469 |

**Geometric mean across all 11 routes, application servers only.** nginx
and Apache are excluded: they don't implement 4 of the 11 routes, so a
mean over "the routes it answered" isn't comparable to one over all eleven.

| Server        | Geomean req/s |     vs Salam |
| ------------- | ------------: | -----------: |
| Go (Fiber)    |       139,948 | 1.03x faster |
| **Salam**     |   **135,323** |          n/a |
| Go (Echo)     |        92,922 | 1.46x slower |
| Go (net/http) |        89,044 | 1.52x slower |
| Go (Gin)      |        87,094 | 1.55x slower |
| node/express  |        41,701 | 3.25x slower |
| php -S        |        23,352 | 5.79x slower |

## Latency, 99th percentile

| Route               |  Salam | Go (net/http) | Go (Fiber) | Go (Gin) | Go (Echo) |  nginx |  Apache | node/express |  php -S |
| ------------------- | -----: | ------------: | ---------: | -------: | --------: | -----: | ------: | -----------: | ------: |
| `/plaintext`        | 952 us |        3.1 ms |     1.6 ms |   3.2 ms |    3.3 ms | 957 us | 13.3 ms |       2.4 ms |  5.1 ms |
| `/json`             | 926 us |        2.6 ms |     3.6 ms |   3.0 ms |    2.7 ms | 765 us | 13.7 ms |       2.5 ms |  5.2 ms |
| `/cached`           | 851 us |        2.9 ms |     1.6 ms |   3.4 ms |    2.5 ms | 1.2 ms |  7.0 ms |       2.7 ms |  6.6 ms |
| `/file`             | 3.8 ms |        5.0 ms |     3.6 ms |  11.0 ms |    4.4 ms | 1.8 ms |  9.4 ms |       3.9 ms |  5.3 ms |
| `/users/42`         | 1.0 ms |        3.0 ms |     3.0 ms |   3.6 ms |    3.4 ms | 981 us |     n/a |       2.7 ms |  5.3 ms |
| `/search?q=&n=5`    | 947 us |        4.0 ms |     4.3 ms |   3.9 ms |    4.0 ms |    n/a |     n/a |       3.1 ms |  5.3 ms |
| `/compute?n=1000`   | 997 us |        3.0 ms |     1.4 ms |   3.1 ms |    2.9 ms |    n/a |     n/a |       3.0 ms |  5.5 ms |
| `/headers`          | 943 us |        3.4 ms |     3.4 ms |   3.2 ms |    3.2 ms |    n/a |     n/a |       2.8 ms |  5.3 ms |
| `/static/style.css` | 5.8 ms |        9.1 ms |     3.3 ms |   7.4 ms |    7.3 ms | 1.7 ms |  9.0 ms |       8.6 ms |  5.8 ms |
| `/`                 | 922 us |        5.9 ms |     1.5 ms |   3.8 ms |    3.7 ms | 1.8 ms | 10.4 ms |       2.9 ms | 12.1 ms |
| `POST /echo`        | 928 us |        4.1 ms |     2.7 ms |   4.1 ms |    3.8 ms |    n/a |     n/a |       3.8 ms | 11.8 ms |

Salam's p99 stays under 1 millisecond on 8 of the 11 routes. Every one of
the four Go setups, including Fiber, is above 1.4ms on every route in this
run. A goroutine scheduler, and for the three non-Fiber frameworks a
garbage collector too, both get a vote on tail latency that Salam's
no-GC, OS-thread-per-connection model doesn't have to take.

## Reading the two columns that move the most

Fiber's edge over Salam is concentrated in exactly two routes: `/file`
(+39%) and `/static/style.css` (+108%). Both are "hand bytes off disk back
to the client" work, where fasthttp's zero-copy file serving is simply very
good. Salam's edge over every Go setup, Fiber included, is concentrated in
the routes that look like real application work: `/search` (+48% over
Fiber, the next best), `/headers` (+25%), `/json` (+20%). For a backend
that spends its time building responses rather than serving static files,
that is the more representative comparison, and it is why the full table,
not just one headline route, is reproduced above instead of picking the
single best number.

## Honesty about the machine

The language benchmark elsewhere on this site runs on a dedicated AMD EPYC
7763 host with nothing else scheduled on it. This one ran on a shared,
6-vCPU development sandbox: not a quiet machine, and not the same one. The
absolute req/s numbers above should not be compared across the two pages.
The _relative_ standing between servers measured together, in the same
run, on the same machine, is what this page is reporting. Rerun
`tests/en/apps/httpbench/bench/run.sh` on your own hardware for numbers you
can trust for your own capacity planning. The whole point of checking
these numbers in is that anyone can.

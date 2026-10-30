# HTTP router comparison

Nine servers answering the same eleven routes: Salam's `net.http`, Go's own
`net/http` (1.22+ `ServeMux`), three popular third-party Go frameworks
(Fiber, Gin, Echo), nginx, Apache, Node (Express 5), and PHP's built-in
development server. Every server implements the exact same response bodies
for the same input, so the only variable is the server.

- Date: 2026-10-28
- Load generator: `bench/loadgen.c`, 64 connections, 2 threads, 10s measured
  per route (plus a 3s discarded warmup and a drain for `TIME_WAIT` sockets
  between routes).
- Order: interleaved. Every server is started first, then each route is
  measured on all of them back to back, so load that comes and goes during
  the run lands on every server alike.
- Machine: 6-vCPU sandbox with steady background load from unrelated work
  (a load average around 11). See
  [Honesty about the machine](#honesty-about-the-machine) below. Servers
  pinned to 4 cores, the load generator to the other 2.
- Full command: `sh tests/en/apps/httpbench/bench/run.sh`
- Raw per-request data: [`results-2026-10-28.jsonl`](results-2026-10-28.jsonl)

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
- **Salam**: `main` plus the epoll worker pool (#2473), built with the 0.5.9 release compiler (LLVM backend)

## Why nginx and Apache have gaps

nginx and Apache are not application servers. They get every route they
can answer out of their own config or file path: `/plaintext`, `/json`,
`/cached`, `/file`, `/static/*`, `/users/:id` (a regex-captured segment
with a canned body), and `/` (a pre-rendered file). Nothing else.
`/search`, `/compute`, `/headers` and `POST /echo` need real per-request
work over caller-supplied input, which is what an application server is
for; faking those numbers behind a reverse proxy would measure the thing
behind nginx, not nginx, so they are left blank instead.

Load: 64 connections, 2 generator threads, 10s measured per route.

### Requests per second

| Route               |   Salam | Go (net/http) | Go (Fiber) | Go (Gin) | Go (Echo) |  nginx | Apache | node/express | php -S |
| ------------------- | ------: | ------------: | ---------: | -------: | --------: | -----: | -----: | -----------: | -----: |
| `/plaintext`        |  96,600 |        45,294 |     74,329 |   43,348 |    40,563 | 94,891 | 19,267 |       27,507 | 11,814 |
| `/json`             |  94,438 |        37,786 |     48,330 |   40,968 |    46,486 | 81,444 | 19,352 |       20,033 | 10,988 |
| `/cached`           |  99,104 |        46,640 |     83,858 |   54,826 |    35,002 | 59,796 | 29,254 |       28,230 | 10,126 |
| `/file`             |  82,282 |        39,644 |     55,164 |   38,229 |    35,779 | 56,219 | 27,561 |       19,321 | 14,458 |
| `/users/42`         | 121,298 |        49,082 |     73,996 |   49,103 |    44,546 | 91,391 |        |       27,605 | 13,068 |
| `/search?q=&n=5`    |  99,143 |        33,193 |     48,247 |   33,824 |    32,524 |        |        |       25,847 | 12,773 |
| `/compute?n=1000`   |  89,183 |        28,390 |     62,966 |   29,829 |    45,446 |        |        |       24,620 |  9,839 |
| `/headers`          | 100,530 |        28,179 |     70,920 |   31,720 |    28,604 |        |        |       24,076 | 11,806 |
| `/static/style.css` |  79,775 |        15,422 |     73,797 |   17,854 |    20,091 | 35,831 | 22,668 |        8,047 | 10,389 |
| `/`                 |  91,136 |        34,019 |     87,651 |   43,717 |    50,685 | 34,831 | 19,056 |       32,695 | 14,122 |
| `POST /echo`        |  95,865 |        44,435 |     33,131 |   21,144 |    22,966 |        |        |       22,766 | 13,647 |

### Requests per minute

| Route               |     Salam | Go (net/http) | Go (Fiber) |  Go (Gin) | Go (Echo) |     nginx |    Apache | node/express |  php -S |
| ------------------- | --------: | ------------: | ---------: | --------: | --------: | --------: | --------: | -----------: | ------: |
| `/plaintext`        | 5,796,005 |     2,717,621 |  4,459,744 | 2,600,866 | 2,433,756 | 5,693,443 | 1,156,035 |    1,650,409 | 708,853 |
| `/json`             | 5,666,284 |     2,267,158 |  2,899,781 | 2,458,099 | 2,789,134 | 4,886,611 | 1,161,117 |    1,201,977 | 659,310 |
| `/cached`           | 5,946,269 |     2,798,379 |  5,031,463 | 3,289,541 | 2,100,105 | 3,587,753 | 1,755,227 |    1,693,809 | 607,544 |
| `/file`             | 4,936,891 |     2,378,657 |  3,309,810 | 2,293,740 | 2,146,719 | 3,373,155 | 1,653,644 |    1,159,269 | 867,447 |
| `/users/42`         | 7,277,899 |     2,944,940 |  4,439,741 | 2,946,205 | 2,672,754 | 5,483,448 |           |    1,656,324 | 784,086 |
| `/search?q=&n=5`    | 5,948,558 |     1,991,554 |  2,894,794 | 2,029,414 | 1,951,462 |           |           |    1,550,821 | 766,362 |
| `/compute?n=1000`   | 5,350,955 |     1,703,425 |  3,777,934 | 1,789,720 | 2,726,781 |           |           |    1,477,210 | 590,353 |
| `/headers`          | 6,031,830 |     1,690,764 |  4,255,193 | 1,903,220 | 1,716,225 |           |           |    1,444,590 | 708,355 |
| `/static/style.css` | 4,786,518 |       925,343 |  4,427,803 | 1,071,266 | 1,205,445 | 2,149,868 | 1,360,051 |      482,811 | 623,354 |
| `/`                 | 5,468,162 |     2,041,161 |  5,259,068 | 2,623,014 | 3,041,121 | 2,089,855 | 1,143,345 |    1,961,674 | 847,313 |
| `POST /echo`        | 5,751,904 |     2,666,118 |  1,987,850 | 1,268,616 | 1,377,967 |           |           |    1,365,939 | 818,814 |

### Latency, 99th percentile

| Route               |  Salam | Go (net/http) | Go (Fiber) | Go (Gin) | Go (Echo) |   nginx |  Apache | node/express |  php -S |
| ------------------- | -----: | ------------: | ---------: | -------: | --------: | ------: | ------: | -----------: | ------: |
| `/plaintext`        | 6.4 ms |       13.9 ms |     9.1 ms |  14.3 ms |   14.9 ms |  9.7 ms | 23.8 ms |      16.8 ms | 23.1 ms |
| `/json`             | 6.7 ms |       14.8 ms |    13.9 ms |  14.8 ms |   13.3 ms | 10.2 ms | 25.9 ms |      23.7 ms | 26.3 ms |
| `/cached`           | 6.4 ms |       12.3 ms |     6.1 ms |  11.5 ms |   17.9 ms | 13.0 ms | 15.5 ms |      18.0 ms | 27.7 ms |
| `/file`             | 7.9 ms |       12.5 ms |    12.3 ms |  13.0 ms |   13.9 ms | 12.8 ms | 16.5 ms |      16.1 ms | 21.8 ms |
| `/users/42`         | 3.8 ms |       11.8 ms |     9.8 ms |  12.4 ms |   13.0 ms |  6.3 ms |         |      14.9 ms | 23.5 ms |
| `/search?q=&n=5`    | 4.6 ms |       15.2 ms |    15.0 ms |  14.6 ms |   15.9 ms |         |         |      15.7 ms | 21.5 ms |
| `/compute?n=1000`   | 6.2 ms |       20.7 ms |    10.7 ms |  20.0 ms |   13.1 ms |         |         |      17.0 ms | 25.1 ms |
| `/headers`          | 5.3 ms |       19.7 ms |    12.1 ms |  18.3 ms |   21.2 ms |         |         |      17.1 ms | 22.1 ms |
| `/static/style.css` | 9.6 ms |       30.0 ms |     9.8 ms |  25.3 ms |   21.4 ms | 17.5 ms | 20.4 ms |      41.4 ms | 24.4 ms |
| `/`                 | 6.6 ms |       15.7 ms |     6.5 ms |  13.5 ms |   12.3 ms | 19.7 ms | 18.9 ms |      15.1 ms | 22.1 ms |
| `POST /echo`        | 5.6 ms |       13.2 ms |    17.2 ms |  22.2 ms |   21.3 ms |         |         |      16.9 ms | 23.3 ms |

### Latency, median

| Route               |  Salam | Go (net/http) | Go (Fiber) | Go (Gin) | Go (Echo) |  nginx | Apache | node/express | php -S |
| ------------------- | -----: | ------------: | ---------: | -------: | --------: | -----: | -----: | -----------: | -----: |
| `/plaintext`        | 315 us |        588 us |     329 us |   574 us |    624 us | 221 us | 1.7 ms |       1.1 ms | 3.7 ms |
| `/json`             | 305 us |        728 us |     499 us |   614 us |    545 us | 260 us | 1.6 ms |       1.5 ms | 3.8 ms |
| `/cached`           | 307 us |        553 us |     329 us |   479 us |    790 us | 441 us | 1.3 ms |       1.1 ms | 4.0 ms |
| `/file`             | 326 us |        818 us |     552 us |   853 us |    899 us | 459 us | 1.3 ms |       2.5 ms | 2.7 ms |
| `/users/42`         | 313 us |        557 us |     337 us |   544 us |    585 us | 267 us |        |       1.4 ms | 3.3 ms |
| `/search?q=&n=5`    | 312 us |        777 us |     344 us |   761 us |    767 us |        |        |       1.6 ms | 3.7 ms |
| `/compute?n=1000`   | 331 us |        982 us |     426 us |   958 us |    590 us |        |        |       1.7 ms | 4.5 ms |
| `/headers`          | 300 us |        982 us |     320 us |   856 us |    910 us |        |        |       1.4 ms | 3.8 ms |
| `/static/style.css` | 336 us |        1.9 ms |     361 us |   1.5 ms |    1.4 ms | 566 us | 1.3 ms |       5.6 ms | 4.5 ms |
| `/`                 | 361 us |        693 us |     328 us |   597 us |    508 us | 840 us | 2.3 ms |       1.4 ms | 3.1 ms |
| `POST /echo`        | 351 us |        608 us |     852 us |   1.3 ms |    1.1 ms |        |        |       2.0 ms | 3.0 ms |

### Geometric mean, every route

Servers that answer all 11 routes.

| Server        | Geomean req/s |     vs Salam |
| ------------- | ------------: | -----------: |
| **Salam**     |        94,850 |          n/a |
| Go (Fiber)    |        62,493 | 1.52x slower |
| Go (Echo)     |        35,238 | 2.69x slower |
| Go (Gin)      |        34,999 | 2.71x slower |
| Go (net/http) |        34,987 | 2.71x slower |
| node/express  |        22,536 | 4.21x slower |
| php -S        |        11,992 | 7.91x slower |

### Geometric mean on the routes nginx answers

The 7 routes nginx serves from its own config, for every server that answers all of them.

| Server        | Geomean req/s |     vs Salam |
| ------------- | ------------: | -----------: |
| **Salam**     |        94,156 |          n/a |
| Go (Fiber)    |        69,653 | 1.35x slower |
| nginx         |        60,490 | 1.56x slower |
| Go (Gin)      |        39,217 | 2.40x slower |
| Go (Echo)     |        37,644 | 2.50x slower |
| Go (net/http) |        36,202 | 2.60x slower |
| node/express  |        21,585 | 4.36x slower |
| php -S        |        12,029 | 7.83x slower |

### Errors and forced reconnects

| Server | Route               | non-2xx | connect errors | I/O errors | reconnects | died |
| ------ | ------------------- | ------: | -------------: | ---------: | ---------: | :--: |
| Apache | `/`                 |       0 |              0 |          2 |          2 |      |
| php -S | `/plaintext`        |       0 |              0 |          0 |    153,676 |      |
| php -S | `/json`             |       0 |              0 |          0 |    143,883 |      |
| php -S | `/cached`           |       0 |              0 |          0 |    136,805 |      |
| php -S | `/file`             |       0 |              0 |          0 |    192,880 |      |
| php -S | `/users/42`         |       0 |              0 |          0 |    171,199 |      |
| php -S | `/search?q=&n=5`    |       0 |              0 |          0 |    166,942 |      |
| php -S | `/compute?n=1000`   |       0 |              0 |          0 |    127,279 |      |
| php -S | `/headers`          |       0 |              0 |          0 |    158,971 |      |
| php -S | `/static/style.css` |       0 |              0 |          0 |    130,865 |      |
| php -S | `/`                 |       0 |              0 |          0 |    187,847 |      |
| php -S | `POST /echo`        |       0 |              0 |          0 |    180,293 |      |

## Reading the results

Salam is ahead on every one of the eleven routes. The widest gaps are on the
routes that do real per-request work: `POST /echo` (95,865 req/s against
Fiber's 33,131), `/search` (99,143 against 48,247) and `/headers` (100,530
against 70,920). The thinnest are `/` (+4% over Fiber) and
`/static/style.css` (+8% over Fiber, 2.2x nginx). Against nginx on the seven
routes it serves itself, Salam leads on all seven, from `/plaintext`
(96,600 against 94,891) to `/static` (79,775 against 35,831).

## Honesty about the machine

The language benchmark elsewhere on this site runs in CI, on a GitHub Actions
`ubuntu-24.04` runner (a shared cloud VM). This one ran on a shared,
6-vCPU development sandbox that was busy with unrelated work the whole time.
Interleaving the routes means every server got the same share of that
load, but contention costs servers that run many threads more than servers
that keep a few busy workers, and Salam's server keeps a few. On a quiet
moment of the same machine, a shorter interleaved run of Salam, Fiber and
nginx put Salam about 16% ahead of Fiber on the geometric mean, not 52%.
The absolute req/s numbers above should not be compared across the two
pages. Rerun `tests/en/apps/httpbench/bench/run.sh` on your own hardware for
numbers you can trust for your own capacity planning. The whole point of
checking these numbers in is that anyone can.

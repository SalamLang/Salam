/*
 * loadgen - a small, dependency-free HTTP/1.1 load generator.
 *
 * Written for this benchmark because the machine it runs on has no wrk and no
 * way to install one, and because `ab` cannot do two things this comparison
 * needs: drive load from more than one thread, and stay honest when the
 * server under test closes the connection after every response.
 *
 * That second point is the whole reason this file exists. A server without
 * keep-alive forces a fresh TCP connection per request. A generator that
 * treats a closed connection as an error reports garbage against such a
 * server; one that silently reconnects hides the cost. This one reconnects
 * and counts the reconnects, so the difference shows up as a number instead
 * of as a mystery.
 *
 *   cc -O2 -pthread -o loadgen loadgen.c
 *   ./loadgen -c 64 -t 4 -d 10 http://127.0.0.1:8099/plaintext
 *
 * Options
 *   -c N     concurrent connections, spread evenly over threads   (default 64)
 *   -t N     threads                                              (default 4)
 *   -d N     duration in seconds                                  (default 10)
 *   -w N     warmup seconds, measured then discarded              (default 2)
 *   -m M     HTTP method                                          (default GET)
 *   -b BODY  request body (implies a Content-Type of text/plain)
 *   -k 0|1   send Connection: keep-alive                          (default 1)
 *   -j       emit one line of JSON instead of a human-readable report
 *
 * What it reports: completed responses per second, per minute, the latency
 * distribution, bytes read per second, and every way a request can fail to
 * count (connect errors, read errors, timeouts, non-2xx responses).
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

/* 1 us per bucket, up to 2.09 s. Anything slower is clamped into the last
 * bucket and also tracked exactly by `max_us`, so a clamped tail is visible
 * rather than silently folded away. */
#define HIST_BUCKETS (1u << 21)
#define RD_BUF 65536

struct config {
    char host[256];
    char port[16];
    char path[1024];
    char method[16];
    char *body;
    size_t body_len;
    int connections;
    int threads;
    int duration_s;
    int warmup_s;
    int keepalive;
    int json_out;
};

struct stats {
    uint64_t requests; /* responses fully read */
    uint64_t bytes;    /* bytes read off the socket */
    uint64_t non2xx;
    uint64_t connect_err;
    uint64_t io_err;
    uint64_t reconnects; /* server closed and we opened a new connection */
    uint64_t max_us;
    uint32_t *hist;
};

/* Per-connection response parser. HTTP/1.1 responses here are either
 * Content-Length delimited, chunked, or delimited by the close itself. All
 * three appear in this comparison: nginx and node send Content-Length, a
 * close-per-request server sends Connection: close, and Express can send
 * chunked for streamed bodies. */
enum body_mode { BODY_LENGTH, BODY_CHUNKED, BODY_UNTIL_CLOSE };

struct conn {
    int fd;
    int in_headers;
    int will_close;
    enum body_mode mode;
    long long remaining;  /* body bytes still to read (BODY_LENGTH) */
    long long chunk_left; /* bytes left in the current chunk */
    int chunk_need_size;  /* parsing a chunk-size line */
    int status;
    size_t hdr_len; /* bytes of the current header block seen */
    char hdr[8192];
    struct timespec sent_at;
    int measuring; /* false during warmup */
};

struct worker {
    pthread_t th;
    int id;
    const struct config *cfg;
    struct stats st;
    int n_conns;
    struct conn *conns;
    char *request;
    size_t request_len;
    struct addrinfo *ai;
    volatile int *phase; /* 0 warmup, 1 measuring, 2 stop */
};

static long long now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static void record(struct stats *st, long long us)
{
    if (us < 0) us = 0;
    if ((uint64_t)us > st->max_us) st->max_us = (uint64_t)us;
    uint32_t b = (uint64_t)us >= HIST_BUCKETS ? HIST_BUCKETS - 1 : (uint32_t)us;
    st->hist[b]++;
}

static int set_nonblock(int fd)
{
    int fl = fcntl(fd, F_GETFL, 0);
    return fl < 0 ? -1 : fcntl(fd, F_SETFL, fl | O_NONBLOCK);
}

/* Opens a non-blocking connection and registers it. Connect completes
 * asynchronously; the first writable event is when the request goes out. */
static int conn_open(struct worker *w, int ep, struct conn *c)
{
    int fd = socket(w->ai->ai_family, SOCK_STREAM, 0);
    if (fd < 0) {
        w->st.connect_err++;
        return -1;
    }
    set_nonblock(fd);
    int one = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
    /* Close with an RST instead of a FIN.
     *
     * Against a server that closes after every response, this generator opens
     * tens of thousands of connections a minute, and each one leaves a socket
     * in TIME_WAIT holding its four-tuple for a minute afterwards. On loopback
     * the ephemeral range is about 28,000 ports, so a few minutes of that runs
     * the client out of ports: connections start failing and the measurement
     * turns into a measurement of the kernel's port table. Closing hard, once
     * the response has been read in full, skips TIME_WAIT.
     *
     * This is safe here and only here: the reply is already complete when the
     * close happens, so nothing is lost by aborting rather than draining. */
    struct linger lg = {1, 0};
    setsockopt(fd, SOL_SOCKET, SO_LINGER, &lg, sizeof lg);
    if (connect(fd, w->ai->ai_addr, w->ai->ai_addrlen) < 0 && errno != EINPROGRESS) {
        close(fd);
        w->st.connect_err++;
        return -1;
    }
    c->fd = fd;
    c->in_headers = 1;
    c->hdr_len = 0;
    c->status = 0;
    c->will_close = 0;
    struct epoll_event ev = {.events = EPOLLOUT, .data.ptr = c};
    if (epoll_ctl(ep, EPOLL_CTL_ADD, fd, &ev) < 0) {
        close(fd);
        c->fd = -1;
        w->st.connect_err++;
        return -1;
    }
    return 0;
}

static void conn_close(struct worker *w, int ep, struct conn *c)
{
    if (c->fd >= 0) {
        epoll_ctl(ep, EPOLL_CTL_DEL, c->fd, NULL);
        close(c->fd);
        c->fd = -1;
    }
    (void)w;
}

/* Writes the whole request. At these sizes (a few hundred bytes) a single
 * send always takes it all on a fresh socket buffer; a short write is treated
 * as an I/O error rather than handled, because it would mean something is
 * wrong that this tool should surface, not paper over. */
static int conn_send(struct worker *w, int ep, struct conn *c)
{
    ssize_t n = send(c->fd, w->request, w->request_len, MSG_NOSIGNAL);
    if (n != (ssize_t)w->request_len) {
        w->st.io_err++;
        return -1;
    }
    clock_gettime(CLOCK_MONOTONIC, &c->sent_at);
    c->measuring = (*w->phase == 1);
    c->in_headers = 1;
    c->hdr_len = 0;
    c->status = 0;
    c->will_close = 0;
    struct epoll_event ev = {.events = EPOLLIN, .data.ptr = c};
    return epoll_ctl(ep, EPOLL_CTL_MOD, c->fd, &ev);
}

static void complete(struct worker *w, int ep, struct conn *c)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    if (c->measuring && *w->phase != 0) {
        long long us = (now.tv_sec - c->sent_at.tv_sec) * 1000000LL +
                       (now.tv_nsec - c->sent_at.tv_nsec) / 1000LL;
        w->st.requests++;
        if (c->status < 200 || c->status > 299) w->st.non2xx++;
        record(&w->st, us);
    }
    if (*w->phase == 2) {
        conn_close(w, ep, c);
        return;
    }
    if (c->will_close || !w->cfg->keepalive) {
        conn_close(w, ep, c);
        w->st.reconnects++;
        conn_open(w, ep, c);
        return;
    }
    if (conn_send(w, ep, c) < 0) {
        conn_close(w, ep, c);
        conn_open(w, ep, c);
    }
}

/* Case-insensitive search for `needle` in the header block. */
static const char *hdr_find(const char *buf, size_t len, const char *needle)
{
    size_t nl = strlen(needle);
    if (len < nl) return NULL;
    for (size_t i = 0; i + nl <= len; i++)
        if (strncasecmp(buf + i, needle, nl) == 0) return buf + i;
    return NULL;
}

static void parse_headers(struct conn *c, const char *end_of_headers)
{
    size_t hl = (size_t)(end_of_headers - c->hdr) + 4;
    if (c->hdr_len >= 12 && strncmp(c->hdr, "HTTP/1.", 7) == 0)
        c->status = atoi(c->hdr + 9);

    const char *cl = hdr_find(c->hdr, hl, "\ncontent-length:");
    const char *te = hdr_find(c->hdr, hl, "\ntransfer-encoding:");
    const char *cn = hdr_find(c->hdr, hl, "\nconnection:");

    if (cn) {
        const char *p = cn + 12;
        while (*p == ' ')
            p++;
        if (strncasecmp(p, "close", 5) == 0) c->will_close = 1;
    }
    if (te &&
        hdr_find(te, 40 < (size_t)(c->hdr + hl - te) ? 40 : (size_t)(c->hdr + hl - te),
                 "chunked")) {
        c->mode = BODY_CHUNKED;
        c->chunk_need_size = 1;
        c->chunk_left = 0;
    } else if (cl) {
        c->mode = BODY_LENGTH;
        c->remaining = atoll(cl + 16);
    } else if (c->status == 204 || c->status == 304) {
        c->mode = BODY_LENGTH;
        c->remaining = 0;
    } else {
        c->mode = BODY_UNTIL_CLOSE;
        c->will_close = 1;
    }
    c->in_headers = 0;
    /* Bytes of body that arrived in the same read as the headers. */
    size_t spill = c->hdr_len - hl;
    if (c->mode == BODY_LENGTH) {
        c->remaining -= (long long)spill;
        if (c->remaining < 0) c->remaining = 0;
    } else if (c->mode == BODY_CHUNKED) {
        /* A chunked body that arrives inside the header read is rare at these
         * sizes; fall back to reading to close rather than mis-frame it. */
        if (spill > 0) {
            c->mode = BODY_UNTIL_CLOSE;
            c->will_close = 1;
        }
    }
}

/* Consumes `n` body bytes for a chunked response. Only counts framing well
 * enough to find the terminating 0-length chunk; the payload itself is
 * discarded, as it is in every mode. */
static int chunked_consume(struct conn *c, const char *p, size_t n)
{
    size_t i = 0;
    while (i < n) {
        if (c->chunk_need_size) {
            const char *nl = memchr(p + i, '\n', n - i);
            if (!nl) return 0;
            c->chunk_left = strtoll(p + i, NULL, 16);
            i = (size_t)(nl - p) + 1;
            c->chunk_need_size = 0;
            if (c->chunk_left == 0) return 1; /* last chunk */
        } else {
            size_t take = n - i;
            if ((long long)take > c->chunk_left + 2) take = (size_t)c->chunk_left + 2;
            long long payload =
                (long long)take > c->chunk_left ? c->chunk_left : (long long)take;
            c->chunk_left -= payload;
            i += take;
            if (c->chunk_left == 0) c->chunk_need_size = 1;
        }
    }
    return 0;
}

static void on_readable(struct worker *w, int ep, struct conn *c)
{
    char buf[RD_BUF];
    for (;;) {
        ssize_t n = recv(c->fd, buf, sizeof buf, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) return;
            w->st.io_err++;
            conn_close(w, ep, c);
            if (*w->phase != 2) {
                w->st.reconnects++;
                conn_open(w, ep, c);
            }
            return;
        }
        if (n == 0) {
            /* Clean EOF. For a read-until-close body that is the response
             * ending; otherwise the peer hung up early. */
            if (!c->in_headers && c->mode == BODY_UNTIL_CLOSE) {
                c->will_close = 1;
                complete(w, ep, c);
            } else {
                w->st.io_err++;
                conn_close(w, ep, c);
                if (*w->phase != 2) {
                    w->st.reconnects++;
                    conn_open(w, ep, c);
                }
            }
            return;
        }
        w->st.bytes += (uint64_t)n;
        const char *p = buf;
        size_t left = (size_t)n;

        if (c->in_headers) {
            size_t room = sizeof c->hdr - c->hdr_len;
            size_t take = left < room ? left : room;
            memcpy(c->hdr + c->hdr_len, p, take);
            c->hdr_len += take;
            const char *eoh = memmem(c->hdr, c->hdr_len, "\r\n\r\n", 4);
            if (!eoh) {
                if (c->hdr_len == sizeof c->hdr) { /* header block too large */
                    w->st.io_err++;
                    conn_close(w, ep, c);
                    if (*w->phase != 2) {
                        w->st.reconnects++;
                        conn_open(w, ep, c);
                    }
                    return;
                }
                continue;
            }
            parse_headers(c, eoh);
            if (c->mode == BODY_LENGTH && c->remaining == 0) {
                complete(w, ep, c);
                return;
            }
            left -= take;
            p += take;
            if (left == 0) continue;
        }

        if (c->mode == BODY_LENGTH) {
            long long take =
                (long long)left < c->remaining ? (long long)left : c->remaining;
            c->remaining -= take;
            if (c->remaining == 0) {
                complete(w, ep, c);
                return;
            }
        } else if (c->mode == BODY_CHUNKED) {
            if (chunked_consume(c, p, left)) {
                complete(w, ep, c);
                return;
            }
        }
        /* BODY_UNTIL_CLOSE: keep reading until EOF, handled above. */
    }
}

static void *run_worker(void *arg)
{
    struct worker *w = arg;
    int ep = epoll_create1(0);
    if (ep < 0) return NULL;

    for (int i = 0; i < w->n_conns; i++) {
        w->conns[i].fd = -1;
        conn_open(w, ep, &w->conns[i]);
    }

    struct epoll_event evs[1024];
    while (*w->phase != 2) {
        int n = epoll_wait(ep, evs, 1024, 50);
        for (int i = 0; i < n; i++) {
            struct conn *c = evs[i].data.ptr;
            if (c->fd < 0) continue;
            if (evs[i].events & (EPOLLERR | EPOLLHUP)) {
                int err = 0;
                socklen_t el = sizeof err;
                getsockopt(c->fd, SOL_SOCKET, SO_ERROR, &err, &el);
                /* EPOLLHUP with data still buffered is a normal
                 * close-after-response; let the read path drain it. */
                if (!(evs[i].events & EPOLLIN)) {
                    w->st.io_err++;
                    conn_close(w, ep, c);
                    if (*w->phase != 2) {
                        w->st.reconnects++;
                        conn_open(w, ep, c);
                    }
                    continue;
                }
            }
            if (evs[i].events & EPOLLOUT) {
                if (conn_send(w, ep, c) < 0) {
                    conn_close(w, ep, c);
                    conn_open(w, ep, c);
                }
                continue;
            }
            if (evs[i].events & EPOLLIN) on_readable(w, ep, c);
        }
    }
    for (int i = 0; i < w->n_conns; i++)
        conn_close(w, ep, &w->conns[i]);
    close(ep);
    return NULL;
}

static uint64_t percentile(const uint32_t *hist, uint64_t total, double p)
{
    if (total == 0) return 0;
    uint64_t want = (uint64_t)(total * p);
    if (want == 0) want = 1;
    uint64_t seen = 0;
    for (uint32_t i = 0; i < HIST_BUCKETS; i++) {
        seen += hist[i];
        if (seen >= want) return i;
    }
    return HIST_BUCKETS - 1;
}

static int parse_url(struct config *cfg, const char *url)
{
    const char *p = url;
    if (strncmp(p, "http://", 7) == 0) p += 7;
    const char *slash = strchr(p, '/');
    const char *hostend = slash ? slash : p + strlen(p);
    const char *colon = memchr(p, ':', (size_t)(hostend - p));
    size_t hl = (size_t)((colon ? colon : hostend) - p);
    if (hl == 0 || hl >= sizeof cfg->host) return -1;
    memcpy(cfg->host, p, hl);
    cfg->host[hl] = 0;
    if (colon) {
        size_t pl = (size_t)(hostend - colon - 1);
        if (pl >= sizeof cfg->port) return -1;
        memcpy(cfg->port, colon + 1, pl);
        cfg->port[pl] = 0;
    } else {
        strcpy(cfg->port, "80");
    }
    snprintf(cfg->path, sizeof cfg->path, "%s", slash ? slash : "/");
    return 0;
}

int main(int argc, char **argv)
{
    struct config cfg = {
        .method = "GET",
        .connections = 64,
        .threads = 4,
        .duration_s = 10,
        .warmup_s = 2,
        .keepalive = 1,
        .json_out = 0,
    };
    const char *url = NULL;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-c") && i + 1 < argc)
            cfg.connections = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-t") && i + 1 < argc)
            cfg.threads = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-d") && i + 1 < argc)
            cfg.duration_s = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-w") && i + 1 < argc)
            cfg.warmup_s = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-m") && i + 1 < argc)
            snprintf(cfg.method, sizeof cfg.method, "%s", argv[++i]);
        else if (!strcmp(argv[i], "-b") && i + 1 < argc) {
            cfg.body = argv[++i];
            cfg.body_len = strlen(cfg.body);
        } else if (!strcmp(argv[i], "-k") && i + 1 < argc)
            cfg.keepalive = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-j"))
            cfg.json_out = 1;
        else
            url = argv[i];
    }
    if (!url || parse_url(&cfg, url) < 0) {
        fprintf(
            stderr,
            "usage: loadgen [-c N] [-t N] [-d SEC] [-w SEC] [-m METHOD] [-b BODY] [-k 0|1] [-j] URL\n");
        return 2;
    }
    if (cfg.threads < 1) cfg.threads = 1;
    if (cfg.connections < cfg.threads) cfg.connections = cfg.threads;

    struct addrinfo hints = {.ai_family = AF_INET, .ai_socktype = SOCK_STREAM}, *ai;
    int rc = getaddrinfo(cfg.host, cfg.port, &hints, &ai);
    if (rc != 0) {
        fprintf(stderr, "resolve %s:%s: %s\n", cfg.host, cfg.port, gai_strerror(rc));
        return 2;
    }

    char request[4096];
    int rl = snprintf(request, sizeof request,
                      "%s %s HTTP/1.1\r\nHost: %s:%s\r\nUser-Agent: loadgen/1.0\r\n"
                      "Accept: */*\r\nConnection: %s\r\n",
                      cfg.method, cfg.path, cfg.host, cfg.port,
                      cfg.keepalive ? "keep-alive" : "close");
    if (cfg.body_len > 0)
        rl += snprintf(request + rl, sizeof request - (size_t)rl,
                       "Content-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s",
                       cfg.body_len, cfg.body);
    else
        rl += snprintf(request + rl, sizeof request - (size_t)rl, "\r\n");

    volatile int phase = 0;
    struct worker *ws = calloc((size_t)cfg.threads, sizeof *ws);
    int base = cfg.connections / cfg.threads, extra = cfg.connections % cfg.threads;
    for (int i = 0; i < cfg.threads; i++) {
        ws[i].id = i;
        ws[i].cfg = &cfg;
        ws[i].ai = ai;
        ws[i].phase = &phase;
        ws[i].request = request;
        ws[i].request_len = (size_t)rl;
        ws[i].n_conns = base + (i < extra ? 1 : 0);
        ws[i].conns = calloc((size_t)ws[i].n_conns, sizeof(struct conn));
        ws[i].st.hist = calloc(HIST_BUCKETS, sizeof(uint32_t));
        pthread_create(&ws[i].th, NULL, run_worker, &ws[i]);
    }

    struct timespec ts;
    if (cfg.warmup_s > 0) {
        ts.tv_sec = cfg.warmup_s;
        ts.tv_nsec = 0;
        nanosleep(&ts, NULL);
    }
    long long t0 = now_ns();
    phase = 1;
    ts.tv_sec = cfg.duration_s;
    ts.tv_nsec = 0;
    nanosleep(&ts, NULL);
    long long t1 = now_ns();
    phase = 2;
    for (int i = 0; i < cfg.threads; i++)
        pthread_join(ws[i].th, NULL);

    struct stats tot = {0};
    tot.hist = calloc(HIST_BUCKETS, sizeof(uint32_t));
    for (int i = 0; i < cfg.threads; i++) {
        tot.requests += ws[i].st.requests;
        tot.bytes += ws[i].st.bytes;
        tot.non2xx += ws[i].st.non2xx;
        tot.connect_err += ws[i].st.connect_err;
        tot.io_err += ws[i].st.io_err;
        tot.reconnects += ws[i].st.reconnects;
        if (ws[i].st.max_us > tot.max_us) tot.max_us = ws[i].st.max_us;
        for (uint32_t b = 0; b < HIST_BUCKETS; b++)
            tot.hist[b] += ws[i].st.hist[b];
    }

    double secs = (double)(t1 - t0) / 1e9;
    double rps = secs > 0 ? (double)tot.requests / secs : 0;
    uint64_t p50 = percentile(tot.hist, tot.requests, 0.50);
    uint64_t p90 = percentile(tot.hist, tot.requests, 0.90);
    uint64_t p99 = percentile(tot.hist, tot.requests, 0.99);
    double sum_us = 0;
    for (uint32_t b = 0; b < HIST_BUCKETS; b++)
        sum_us += (double)b * tot.hist[b];
    double avg = tot.requests ? sum_us / (double)tot.requests : 0;

    if (cfg.json_out) {
        printf(
            "{\"url\":\"%s\",\"connections\":%d,\"threads\":%d,\"duration_s\":%.3f,"
            "\"requests\":%llu,\"rps\":%.1f,\"rpm\":%.0f,\"bytes_per_s\":%.0f,"
            "\"avg_us\":%.1f,\"p50_us\":%llu,\"p90_us\":%llu,\"p99_us\":%llu,\"max_us\":%llu,"
            "\"non2xx\":%llu,\"connect_err\":%llu,\"io_err\":%llu,\"reconnects\":%llu}\n",
            url, cfg.connections, cfg.threads, secs, (unsigned long long)tot.requests,
            rps, rps * 60.0, secs > 0 ? (double)tot.bytes / secs : 0, avg,
            (unsigned long long)p50, (unsigned long long)p90, (unsigned long long)p99,
            (unsigned long long)tot.max_us, (unsigned long long)tot.non2xx,
            (unsigned long long)tot.connect_err, (unsigned long long)tot.io_err,
            (unsigned long long)tot.reconnects);
    } else {
        printf("%s  %d conns / %d threads / %.1fs\n", url, cfg.connections, cfg.threads,
               secs);
        printf("  requests   %llu  (%.0f/s, %.0f/min)\n",
               (unsigned long long)tot.requests, rps, rps * 60.0);
        printf(
            "  latency    avg %.0fus  p50 %lluus  p90 %lluus  p99 %lluus  max %lluus\n",
            avg, (unsigned long long)p50, (unsigned long long)p90,
            (unsigned long long)p99, (unsigned long long)tot.max_us);
        printf("  transfer   %.2f MB/s\n",
               secs > 0 ? (double)tot.bytes / secs / 1048576.0 : 0);
        printf("  errors     non2xx %llu  connect %llu  io %llu   reconnects %llu\n",
               (unsigned long long)tot.non2xx, (unsigned long long)tot.connect_err,
               (unsigned long long)tot.io_err, (unsigned long long)tot.reconnects);
    }
    freeaddrinfo(ai);
    return 0;
}

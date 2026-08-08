/*
 * Salam Programming Language (2024-2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#include "core/prof_self.h"
#include "core/sal_format.h"
#include "core/timer.h"
#include "i18n/i18n.h"

#define PROF_MAX_DEPTH 64
#define PROF_MAX_FILES 512
#define PROF_TOP_FILES 8

typedef struct {
    uint64_t total_ns; /* wall, nested scopes included */
    uint64_t self_ns;  /* wall, minus directly nested scopes */
    uint64_t cpu_ns;
    uint32_t calls;
} phase_acc_t;

/* The label is copied, not borrowed. Callers pass source paths allocated in
 * the compiler's arena, and the arena is freed before driver_main() prints the
 * report, so holding the pointer would be a use-after-free. */
#define PROF_LABEL_MAX 200

typedef struct {
    char label[PROF_LABEL_MAX];
    uint64_t ns[TP__COUNT];
    uint64_t total_ns;
} file_acc_t;

typedef struct {
    const char *name;
    int ph;
    uint64_t ts;
    uint64_t dur;
} trace_ev_t;

typedef struct {
    int ph;
    const char *label;
    uint64_t t0;
    uint64_t cpu0;
    uint64_t child_ns;
} prof_frame_t;

static struct {
    bool on;
    bool trace;
    bool overflow;
    uint64_t start_ns;
    uint64_t start_cpu_ns;
    phase_acc_t acc[TP__COUNT];
    uint64_t counter[TC__COUNT];
    prof_frame_t stack[PROF_MAX_DEPTH];
    int depth;
    file_acc_t files[PROF_MAX_FILES];
    int nfiles;
    bool files_full;
    trace_ev_t *events;
    size_t nevents;
    size_t cap_events;
} g;

static const char *k_phase_name[TP__COUNT] = {"driver",   "source",  "lexer", "parser",
                                              "semantic", "codegen", "llvm",  "jsgen",
                                              "write",    "cc",      "link"};

static const char *k_counter_name[TC__COUNT] = {
    "tokens",        "ast_nodes",      "symbols",     "funcs_emitted",
    "modules_built", "modules_cached", "arena_bytes", "arena_blocks"};

const char *prof_self_phase_name(timing_phase_t ph)
{
    if ((int)ph < 0 || (int)ph >= TP__COUNT) return "?";
    return k_phase_name[ph];
}

void prof_self_enable(bool trace_events)
{
    memset(&g, 0, sizeof g);
    g.on = true;
    g.trace = trace_events;
    g.start_ns = salam_mono_ns();
    g.start_cpu_ns = salam_cpu_ns();
}

bool prof_self_on(void)
{
    return g.on;
}

/* printf's %llu is not reliable across every host C compiler this project is
 * built with (and `unsigned long` is 32-bit on Windows), so 64-bit values are
 * always rendered by hand and printed with %s. */
static const char *u64s(uint64_t v, char *buf, size_t cap)
{
    char tmp[24];
    size_t n = 0;
    size_t i;
    if (cap == 0) return "";
    if (v == 0) {
        tmp[n++] = '0';
    } else {
        while (v > 0 && n < sizeof tmp) {
            tmp[n++] = (char)('0' + (int)(v % 10));
            v /= 10;
        }
    }
    for (i = 0; i < n && i + 1 < cap; i++)
        buf[i] = tmp[n - 1 - i];
    buf[i] = '\0';
    return buf;
}

static file_acc_t *file_slot(const char *label)
{
    int i;
    size_t n;
    for (i = 0; i < g.nfiles; i++)
        if (strcmp(g.files[i].label, label) == 0) return &g.files[i];
    if (g.nfiles >= PROF_MAX_FILES) {
        g.files_full = true;
        return NULL;
    }
    memset(&g.files[g.nfiles], 0, sizeof g.files[0]);
    n = strlen(label);
    if (n >= PROF_LABEL_MAX) {
        /* Keep the tail: the interesting part of a long path is its end. */
        memcpy(g.files[g.nfiles].label, label + (n - (PROF_LABEL_MAX - 1)),
               PROF_LABEL_MAX - 1);
    } else {
        memcpy(g.files[g.nfiles].label, label, n);
    }
    return &g.files[g.nfiles++];
}

static void trace_push(const char *name, int ph, uint64_t ts, uint64_t dur)
{
    trace_ev_t *ev;
    if (g.nevents == g.cap_events) {
        size_t ncap = g.cap_events ? g.cap_events * 2 : 256;
        trace_ev_t *np = (trace_ev_t *)realloc(g.events, ncap * sizeof(trace_ev_t));
        if (!np) return; /* out of memory: drop the event, keep the numbers */
        g.events = np;
        g.cap_events = ncap;
    }
    ev = &g.events[g.nevents++];
    ev->name = name;
    ev->ph = ph;
    ev->ts = ts;
    ev->dur = dur;
}

uint64_t prof_self_begin(timing_phase_t ph, const char *label)
{
    prof_frame_t *f;
    if (!g.on) return 0;
    if ((int)ph < 0 || (int)ph >= TP__COUNT) return 0;
    if (g.depth >= PROF_MAX_DEPTH) {
        g.overflow = true;
        return 0;
    }
    f = &g.stack[g.depth];
    f->ph = (int)ph;
    f->label = label;
    f->child_ns = 0;
    f->cpu0 = salam_cpu_ns();
    f->t0 = salam_mono_ns();
    g.depth++;
    return (uint64_t)g.depth; /* 1-based, so 0 can mean "not recording" */
}

void prof_self_end(timing_phase_t ph, uint64_t token)
{
    prof_frame_t *f;
    phase_acc_t *acc;
    file_acc_t *fa;
    uint64_t now;
    uint64_t cpu;
    uint64_t dt;
    (void)ph;
    if (!g.on || token == 0) return;
    if (token > (uint64_t)g.depth) return; /* already unwound; nothing to do */
    now = salam_mono_ns();
    cpu = salam_cpu_ns();
    /* Tolerate a scope left open by an early return: unwind to this token
     * rather than mis-attributing every later reading. */
    g.depth = (int)token - 1;
    f = &g.stack[g.depth];
    dt = now > f->t0 ? now - f->t0 : 0;
    acc = &g.acc[f->ph];
    acc->total_ns += dt;
    acc->self_ns += dt > f->child_ns ? dt - f->child_ns : 0;
    acc->cpu_ns += cpu > f->cpu0 ? cpu - f->cpu0 : 0;
    acc->calls++;
    if (g.depth > 0) g.stack[g.depth - 1].child_ns += dt;
    fa = f->label ? file_slot(f->label) : NULL;
    if (fa) {
        fa->ns[f->ph] += dt;
        fa->total_ns += dt;
    }
    /* Both alternatives outlive the report: fa->label is the profiler's own
     * copy, k_phase_name is static. f->label is not, so it is never stored. */
    if (g.trace) trace_push(fa ? fa->label : k_phase_name[f->ph], f->ph, f->t0, dt);
}

void prof_self_count(timing_counter_t c, uint64_t n)
{
    if (!g.on) return;
    if ((int)c < 0 || (int)c >= TC__COUNT) return;
    g.counter[c] += n;
}

/* ------------------------------------------------------------------ */
/* formatting helpers                                                  */
/* ------------------------------------------------------------------ */

/* Picks a unit so the number keeps 3 or 4 significant digits. */
static const char *fmt_dur(uint64_t ns, char *buf, size_t cap)
{
    char nb[24];
    if (ns >= 1000000000ULL)
        sal_snprintf(buf, cap, "%.3f s", (double)ns / 1e9);
    else if (ns >= 1000000ULL)
        sal_snprintf(buf, cap, "%.2f ms", (double)ns / 1e6);
    else if (ns >= 1000ULL)
        sal_snprintf(buf, cap, "%.2f us", (double)ns / 1e3);
    else
        sal_snprintf(buf, cap, "%s ns", u64s(ns, nb, sizeof nb));
    return buf;
}

static const char *fmt_bytes(uint64_t b, char *buf, size_t cap)
{
    char nb[24];
    if (b >= (1ULL << 30))
        sal_snprintf(buf, cap, "%.1f GB", (double)b / (double)(1ULL << 30));
    else if (b >= (1ULL << 20))
        sal_snprintf(buf, cap, "%.1f MB", (double)b / (double)(1ULL << 20));
    else if (b >= 1024ULL)
        sal_snprintf(buf, cap, "%.1f KB", (double)b / 1024.0);
    else
        sal_snprintf(buf, cap, "%s B", u64s(b, nb, sizeof nb));
    return buf;
}

/* Groups digits so six-figure token counts stay readable. */
static const char *fmt_num(uint64_t n, char *buf, size_t cap)
{
    char tmp[24];
    size_t len;
    size_t i;
    size_t o = 0;
    u64s(n, tmp, sizeof tmp);
    len = strlen(tmp);
    for (i = 0; i < len; i++) {
        if (i > 0 && ((len - i) % 3) == 0 && o + 1 < cap) buf[o++] = ',';
        if (o + 1 < cap) buf[o++] = tmp[i];
    }
    if (cap) buf[o < cap ? o : cap - 1] = '\0';
    return buf;
}

static int cmp_phase_self(const void *pa, const void *pb)
{
    int a = *(const int *)pa;
    int b = *(const int *)pb;
    if (g.acc[a].self_ns > g.acc[b].self_ns) return -1;
    if (g.acc[a].self_ns < g.acc[b].self_ns) return 1;
    return a - b;
}

static int cmp_file_total(const void *pa, const void *pb)
{
    const file_acc_t *a = (const file_acc_t *)pa;
    const file_acc_t *b = (const file_acc_t *)pb;
    if (a->total_ns > b->total_ns) return -1;
    if (a->total_ns < b->total_ns) return 1;
    return 0;
}

static void json_escape(FILE *out, const char *s)
{
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    for (; *p; p++) {
        if (*p == '"' || *p == '\\')
            fprintf(out, "\\%c", (int)*p);
        else if (*p == '\n')
            fputs("\\n", out);
        else if (*p == '\r')
            fputs("\\r", out);
        else if (*p == '\t')
            fputs("\\t", out);
        else if (*p < 0x20)
            fprintf(out, "\\u%04x", (unsigned)*p);
        else
            fputc((int)*p, out);
    }
}

static void report_table(FILE *out, uint64_t wall_ns, uint64_t cpu_ns)
{
    int order[TP__COUNT];
    static file_acc_t sorted[PROF_MAX_FILES];
    char b1[48];
    char b2[48];
    char b3[48];
    int i;
    int shown;

    fprintf(out, "\n%s\n", i18n_tr("Salam compile time report"));
    fprintf(out, "  %s %s   %s %s   %s %s\n", i18n_tr("wall"),
            fmt_dur(wall_ns, b1, sizeof b1), i18n_tr("cpu"),
            fmt_dur(cpu_ns, b2, sizeof b2), i18n_tr("peak rss"),
            fmt_bytes(salam_peak_rss(), b3, sizeof b3));

    for (i = 0; i < TP__COUNT; i++)
        order[i] = i;
    qsort(order, TP__COUNT, sizeof order[0], cmp_phase_self);

    fprintf(out, "\n  %-10s %10s %11s %8s %8s\n", i18n_tr("phase"), i18n_tr("self"),
            i18n_tr("total"), i18n_tr("%self"), i18n_tr("calls"));
    fprintf(out, "  ----------------------------------------------------\n");
    for (i = 0; i < TP__COUNT; i++) {
        const phase_acc_t *a = &g.acc[order[i]];
        double pct;
        char cb[24];
        if (a->calls == 0) continue;
        pct = wall_ns ? (double)a->self_ns * 100.0 / (double)wall_ns : 0.0;
        fprintf(out, "  %-10s %10s %11s %7.1f%% %8s\n", k_phase_name[order[i]],
                fmt_dur(a->self_ns, b1, sizeof b1), fmt_dur(a->total_ns, b2, sizeof b2),
                pct, u64s((uint64_t)a->calls, cb, sizeof cb));
    }

    if (g.nfiles > 0) {
        memcpy(sorted, g.files, (size_t)g.nfiles * sizeof(file_acc_t));
        qsort(sorted, (size_t)g.nfiles, sizeof(file_acc_t), cmp_file_total);
        shown = g.nfiles < PROF_TOP_FILES ? g.nfiles : PROF_TOP_FILES;
        fprintf(out, "\n  %s\n", i18n_tr("slowest inputs"));
        for (i = 0; i < shown; i++) {
            int worst = -1;
            int p;
            for (p = 0; p < TP__COUNT; p++)
                if (worst < 0 || sorted[i].ns[p] > sorted[i].ns[worst]) worst = p;
            fprintf(out, "    %-44s %10s", sorted[i].label,
                    fmt_dur(sorted[i].total_ns, b1, sizeof b1));
            if (worst >= 0 && sorted[i].ns[worst] > 0)
                fprintf(out, "   (%s %s)", k_phase_name[worst],
                        fmt_dur(sorted[i].ns[worst], b2, sizeof b2));
            fputc('\n', out);
        }
        if (g.files_full)
            fprintf(out, "    %s\n",
                    i18n_tr("(input table full, some files not listed)"));
    }

    fprintf(out, "\n  %s\n", i18n_tr("counters"));
    fprintf(out, "    %-16s %12s   %-16s %12s\n", "tokens",
            fmt_num(g.counter[TC_TOKENS], b1, sizeof b1), "ast nodes",
            fmt_num(g.counter[TC_AST_NODES], b2, sizeof b2));
    fprintf(out, "    %-16s %12s   %-16s %12s\n", "symbols",
            fmt_num(g.counter[TC_SYMBOLS], b1, sizeof b1), "funcs emitted",
            fmt_num(g.counter[TC_FUNCS_EMITTED], b2, sizeof b2));
    fprintf(out, "    %-16s %12s   %-16s %12s\n", "modules built",
            fmt_num(g.counter[TC_MODULES_BUILT], b1, sizeof b1), "modules cached",
            fmt_num(g.counter[TC_MODULES_CACHED], b2, sizeof b2));
    fprintf(out, "    %-16s %12s   %-16s %12s\n", "arena",
            fmt_bytes(g.counter[TC_ARENA_BYTES], b1, sizeof b1), "arena blocks",
            fmt_num(g.counter[TC_ARENA_BLOCKS], b2, sizeof b2));

    if (g.overflow)
        fprintf(out, "\n  %s\n",
                i18n_tr("warning: timing scopes nested deeper than the profiler "
                        "stack; some readings were dropped"));
    fputc('\n', out);
}

static void report_json(FILE *out, uint64_t wall_ns, uint64_t cpu_ns)
{
    char b1[24];
    char b2[24];
    char b3[24];
    char b4[24];
    int i;
    bool first;
    fprintf(out, "{\"schema\":\"salam.timereport.v1\"");
    fprintf(out, ",\"wall_ns\":%s", u64s(wall_ns, b1, sizeof b1));
    fprintf(out, ",\"cpu_ns\":%s", u64s(cpu_ns, b2, sizeof b2));
    fprintf(out, ",\"peak_rss\":%s", u64s(salam_peak_rss(), b3, sizeof b3));
    fprintf(out, ",\"truncated\":%s", (g.overflow || g.files_full) ? "true" : "false");

    fprintf(out, ",\"phases\":{");
    first = true;
    for (i = 0; i < TP__COUNT; i++) {
        const phase_acc_t *a = &g.acc[i];
        if (a->calls == 0) continue;
        fprintf(out, "%s\"%s\":{\"self_ns\":%s", first ? "" : ",", k_phase_name[i],
                u64s(a->self_ns, b1, sizeof b1));
        fprintf(out, ",\"total_ns\":%s,\"cpu_ns\":%s,\"calls\":%s}",
                u64s(a->total_ns, b2, sizeof b2), u64s(a->cpu_ns, b3, sizeof b3),
                u64s((uint64_t)a->calls, b4, sizeof b4));
        first = false;
    }
    fprintf(out, "}");

    fprintf(out, ",\"files\":[");
    for (i = 0; i < g.nfiles; i++) {
        int p;
        bool pfirst = true;
        fprintf(out, "%s{\"name\":\"", i ? "," : "");
        json_escape(out, g.files[i].label);
        fprintf(out, "\",\"total_ns\":%s,\"phases\":{",
                u64s(g.files[i].total_ns, b1, sizeof b1));
        for (p = 0; p < TP__COUNT; p++) {
            if (g.files[i].ns[p] == 0) continue;
            fprintf(out, "%s\"%s\":%s", pfirst ? "" : ",", k_phase_name[p],
                    u64s(g.files[i].ns[p], b2, sizeof b2));
            pfirst = false;
        }
        fprintf(out, "}}");
    }
    fprintf(out, "]");

    fprintf(out, ",\"counters\":{");
    for (i = 0; i < TC__COUNT; i++)
        fprintf(out, "%s\"%s\":%s", i ? "," : "", k_counter_name[i],
                u64s(g.counter[i], b1, sizeof b1));
    fprintf(out, "}}\n");
}

void prof_self_report(FILE *out, int fmt)
{
    uint64_t wall_ns;
    uint64_t cpu_ns;
    uint64_t now;
    uint64_t cpu;
    if (!g.on || !out) return;
    /* Close anything still open so a report taken from an error path is not
     * missing the scope that was in flight. */
    while (g.depth > 0)
        prof_self_end((timing_phase_t)g.stack[g.depth - 1].ph, (uint64_t)g.depth);
    now = salam_mono_ns();
    cpu = salam_cpu_ns();
    wall_ns = now > g.start_ns ? now - g.start_ns : 0;
    cpu_ns = cpu > g.start_cpu_ns ? cpu - g.start_cpu_ns : 0;
    if (fmt == PROF_FMT_JSON)
        report_json(out, wall_ns, cpu_ns);
    else
        report_table(out, wall_ns, cpu_ns);
    fflush(out);
}

bool prof_self_write_trace(const char *path)
{
    FILE *f;
    size_t i;
    if (!g.on || !path) return false;
    f = fopen(path, "wb");
    if (!f) return false;
    /* Chrome Trace Event format: an array of complete ("X") events with
     * microsecond timestamps. chrome://tracing, Perfetto and speedscope all
     * read this directly, so we get a flame chart of the build for free. */
    fputs("[\n", f);
    for (i = 0; i < g.nevents; i++) {
        const trace_ev_t *ev = &g.events[i];
        fprintf(f, "%s{\"name\":\"", i ? ",\n" : "");
        json_escape(f, ev->name);
        fprintf(f, "\",\"cat\":\"%s\",\"ph\":\"X\",\"pid\":1,\"tid\":1",
                k_phase_name[ev->ph]);
        fprintf(f, ",\"ts\":%.3f,\"dur\":%.3f}", (double)(ev->ts - g.start_ns) / 1000.0,
                (double)ev->dur / 1000.0);
    }
    fputs("\n]\n", f);
    fclose(f);
    return true;
}

void prof_self_shutdown(void)
{
    free(g.events);
    g.events = NULL;
    g.nevents = 0;
    g.cap_events = 0;
    g.on = false;
}

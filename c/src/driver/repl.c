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

/* The interactive prompt. repl_line.c reads the keys, repl_session.c
 * remembers what has been typed, and this file is the turn loop between
 * them.
 *
 * Salam compiles ahead of time, so a turn is not an eval: the session is
 * re-rendered as one whole program and rebuilt on every statement. That is
 * what makes bindings survive - `mut xs := Vector {}` on one line and
 * `xs.push("a")` on the next are two statements of the same synthesized
 * `func main`, not two programs. Re-running the program also re-runs the
 * output of the turns before it, so repl_emit_output() prints only what
 * the newest statement added. */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "driver/driver.h"
#include "driver/build.h"
#include "driver/repl.h"
#include "driver/repl_internal.h"
#include "core/arena.h"
#include "core/sb.h"
#include "cli/options.h"
#include "langpack/langpack.h"
#include "lexer/lexer.h"
#include "logger/logger.h"
#include "source/source.h"
#include "condcomp/condcomp.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "codegen/codegen.h"
#include "layout/layout_gen.h"
#include "diag/diag.h"

#if defined(_WIN32)
#  include <process.h>
#  define REPL_GETPID() _getpid()
#else
#  include <unistd.h>
#  define REPL_GETPID() getpid()
#endif

#define REPL_NAME_MAX 128
#define REPL_PATH_MAX 600

typedef struct {
    options_t *opt;
    const langpack_t *pack;
    repl_session_t s;
    char *last_out; /* what the session printed on its last good run */
    size_t last_len;
} repl_ctx_t;

/* ------------------------------------------------------------------ */
/* reading a turn                                                      */
/* ------------------------------------------------------------------ */

/* Is the turn still unfinished? The parser decides: it is the only thing
 * that knows a `match` arm from a struct field, and a block-opening ':'
 * from the ':' in `f(n: int): int`. Counting colons and `end`s by hand -
 * what this used to do - miscounted the moment a function took a typed
 * parameter, and the prompt then swallowed everything typed after it. */
static bool repl_incomplete(const langpack_t *pack, const char *src)
{
    arena_t *a = arena_new(1 << 16);
    logger_t *lg = logger_new(stderr, LOG_OFF, false);
    source_file_t sf;
    token_stream_t *toks = NULL;
    bool more = false;
    sf.path = "<repl>";
    sf.text = CONST_CAST(src);
    sf.len = strlen(src);
    if (lexer_run(a, lg, pack, &sf, &toks)) more = parser_needs_more(a, lg, toks);
    logger_free(lg);
    arena_free(a);
    return more;
}

/* Reads continuation lines until the turn parses (or input ends). Returns
 * the whole turn newline-terminated, or NULL if the user abandoned it -
 * without that escape a block that can never be completed (a typo in the
 * word that closes it) would hold the prompt forever. */
static char *repl_read_block(const langpack_t *pack, const char *first)
{
    sb_t block;
    sb_init(&block);
    sb_puts(&block, first);
    sb_putc(&block, '\n');
    while (repl_incomplete(pack, sb_cstr(&block))) {
        char *cont = repl_readline("  ... ");
        if (!cont) break;
        if (strcmp(cont, ":abort") == 0) {
            free(cont);
            sb_free(&block);
            printf("abandoned.\n");
            fflush(stdout);
            return NULL;
        }
        sb_puts(&block, cont);
        sb_putc(&block, '\n');
        repl_hist_push(cont);
        free(cont);
    }
    return block.data;
}

/* ------------------------------------------------------------------ */
/* checking and running a session                                      */
/* ------------------------------------------------------------------ */

/* Front end only: lex, parse, prune, analyze. `quiet` decides whether the
 * user sees the diagnostics - the echo probe below asks a question it does
 * not want answered out loud. */
static bool repl_check(repl_ctx_t *c, const char *src, bool quiet)
{
    arena_t *a = arena_new(1 << 20);
    logger_t *lg =
        logger_new(stderr, quiet ? LOG_OFF : LOG_ERROR, resolve_color(c->opt->color));
    source_file_t sf;
    token_stream_t *toks = NULL;
    ast_node_t *prog = NULL;
    cc_table_t *cc;
    sema_result_t *sr;
    bool ok;
    sf.path = "<repl>";
    sf.text = CONST_CAST(src);
    sf.len = strlen(src);
    if (!quiet)
        logger_set_diag_source(lg, sf.text, sf.len, c->opt->diag_style,
                               c->opt->diag_format);
    ok = lexer_run(a, lg, c->pack, &sf, &toks);
    ok = ok && parser_run(a, lg, toks, &prog);
    cc = cc_table_build(a, NULL, c->opt->defines, c->opt->ndefines);
    ok = ok && cc_prune_program(a, lg, sf.path, cc, prog);
    sr = ok ? sema_run(a, lg, prog, sf.path, langpack_code(c->pack), cc) : NULL;
    ok = sr && sr->ok;
    logger_free(lg);
    arena_free(a);
    return ok;
}

static const char *repl_tmp_dir(void)
{
#if defined(_WIN32)
    const char *t = getenv("TEMP");
    if (!t) t = getenv("TMP");
#else
    const char *t = getenv("TMPDIR");
    if (!t) t = "/tmp";
#endif
    return (t && t[0]) ? t : ".";
}

static char *repl_read_all(const char *path, size_t *len_out)
{
    long n;
    char *buf;
    FILE *f = fopen(path, "rb");
    *len_out = 0;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n < 0) n = 0;
    buf = (char *)malloc((size_t)n + 1);
    if (buf) {
        size_t got = fread(buf, 1, (size_t)n, f);
        buf[got] = '\0';
        *len_out = got;
    }
    fclose(f);
    return buf;
}

/* Prints what this run added. Everything the session printed on earlier
 * turns comes out again byte for byte, so the common prefix is replay and
 * only the tail is news; a program whose output changed (it reads a clock,
 * a file, the network) fails the prefix test and is shown whole rather
 * than guessed at. */
static void repl_emit_output(repl_ctx_t *c, const char *out, size_t len)
{
    size_t skip = 0;
    if (c->last_out && len >= c->last_len && memcmp(out, c->last_out, c->last_len) == 0)
        skip = c->last_len;
    if (len > skip) {
        fwrite(out + skip, 1, len - skip, stdout);
        fflush(stdout);
    }
}

static void repl_keep_output(repl_ctx_t *c, char *out, size_t len)
{
    free(c->last_out);
    c->last_out = out;
    c->last_len = len;
}

static void repl_run_cmd(char *cmd, size_t cap, const char *exe, const char *out)
{
#if defined(_WIN32)
    /* cmd.exe strips the outermost quote pair off everything after /c, so
     * the whole redirected command needs one more pair around it for a
     * temp path with a space in it to survive. */
    sal_snprintf(cmd, cap, "\"\"%s\" > \"%s\" 2>&1\"", exe, out);
#else
    sal_snprintf(cmd, cap, "\"%s\" > \"%s\" 2>&1", exe, out);
#endif
}

/* Builds `src` the way `salam run` builds a file - driver_build() owns
 * backend selection (in-process LLVM first, then the C backend and its own
 * clang > gcc > tcc ladder), and the session's `import`s resolve exactly as
 * they would in a normal build. Returns 0 when the program built and exited
 * cleanly. `replay` is true for a turn (dedup the output) and false for an
 * explicit :run (show all of it). */
static int repl_exec(repl_ctx_t *c, const char *src, bool replay)
{
    static char tmp_src[REPL_PATH_MAX], tmp_exe[REPL_PATH_MAX], tmp_out[REPL_PATH_MAX];
    static unsigned long seq = 0;
    const char *dir = repl_tmp_dir();
    unsigned long pid = (unsigned long)REPL_GETPID();
    options_t *opt = c->opt;
    const char *exe;
    char cmd[REPL_PATH_MAX * 2 + 32];
    char *out;
    size_t len;
    int rc;
    FILE *f;

    if (opt->log_level == LOG_INFO) opt->log_level = LOG_WARN;
    seq++;
    sal_snprintf(tmp_src, sizeof tmp_src, "%s/salam-repl-%lu-%lu.salam", dir, pid, seq);
    sal_snprintf(tmp_out, sizeof tmp_out, "%s/salam-repl-%lu-%lu.out", dir, pid, seq);
#if defined(_WIN32)
    sal_snprintf(tmp_exe, sizeof tmp_exe, "%s/salam-repl-%lu-%lu.exe", dir, pid, seq);
#else
    sal_snprintf(tmp_exe, sizeof tmp_exe, "%s/salam-repl-%lu-%lu", dir, pid, seq);
#endif
    f = fopen(tmp_src, "wb");
    if (!f) {
        fprintf(stderr, "error: could not write a temp file for the session\n");
        return -1;
    }
    fputs(src, f);
    fclose(f);

    opt->inputs[0] = tmp_src;
    opt->input_count = 1;
    opt->input = tmp_src;
    opt->output = tmp_exe;
    opt->exe_path[0] = '\0';
    opt->keep_c = false;
    rc = driver_build(opt);
    exe = opt->exe_path[0] ? opt->exe_path : tmp_exe;
    if (rc == 0) {
        repl_run_cmd(cmd, sizeof cmd, exe, tmp_out);
        rc = system(cmd);
        out = repl_read_all(tmp_out, &len);
        if (out) {
            if (replay) {
                repl_emit_output(c, out, len);
            } else {
                fwrite(out, 1, len, stdout);
                fflush(stdout);
            }
            if (rc == 0)
                repl_keep_output(c, out, len); /* takes ownership */
            else
                free(out);
        }
        if (rc != 0) printf("[program exited with %d]\n", rc);
        remove(tmp_out);
        remove(exe);
    }
    remove(tmp_src);
    return rc;
}

/* ------------------------------------------------------------------ */
/* turns                                                               */
/* ------------------------------------------------------------------ */

/* `expr` alone on a line is a question, so answer it: the REPL tries the
 * turn as `println expr` first and keeps that form only if it compiles.
 * A void call like `xs.push("a")` fails that check and runs as the plain
 * statement it is. The expression is not parenthesized - print statements
 * reject that outright - which is safe because it is the whole line. */
static char *repl_echo_wrap(const langpack_t *pack, const char *block)
{
    const char *kw = langpack_spelling(pack, TK_KW_PRINTLN);
    size_t n = strlen(block);
    sb_t w;
    while (n > 0 && (block[n - 1] == '\n' || block[n - 1] == '\r'))
        n--;
    sb_init(&w);
    sb_printf(&w, "%s %.*s\n", kw ? kw : "println", (int)n, block);
    return w.data;
}

static void repl_turn_stmt(repl_ctx_t *c, const char *block)
{
    repl_entry_t cand;
    char *echo = NULL;
    char *src;
    cand.kind = RS_STMT;
    cand.name = NULL;
    cand.text = CONST_CAST(block);
    if (repl_is_expr_line(c->pack, block)) {
        echo = repl_echo_wrap(c->pack, block);
        cand.text = echo;
        src = repl_render(&c->s, c->pack, &cand);
        if (!repl_check(c, src, true)) cand.text = CONST_CAST(block);
        free(src);
    }
    src = repl_render(&c->s, c->pack, &cand);
    /* Analyze before building. A rejected turn is then reported against
     * "<repl>" - the same source the def path shows - instead of against a
     * temp file whose path is longer than the program and which is deleted
     * before the user could open it, and no backend or C compiler is
     * started for a turn that cannot survive the front end. */
    if (repl_check(c, src, false) && repl_exec(c, src, true) == 0)
        repl_session_add_stmt(&c->s, cand.text);
    free(src);
    free(echo);
}

static bool repl_defines(const repl_session_t *s, const char *name)
{
    size_t i = 0;
    for (; i < s->ndefs; i++)
        if (strcmp(s->defs[i].name, name) == 0) return true;
    return false;
}

static void repl_turn_def(repl_ctx_t *c, repl_kind_t kind, const char *name,
                          const char *block)
{
    repl_entry_t cand;
    bool again = repl_defines(&c->s, name);
    char *src;
    bool ok;
    cand.kind = kind;
    cand.name = CONST_CAST(name);
    cand.text = CONST_CAST(block);
    src = repl_render(&c->s, c->pack, &cand);
    ok = repl_check(c, src, false);
    free(src);
    if (!ok) return;
    repl_session_add(&c->s, c->pack, kind, name, block);
    if (c->s.has_main && c->s.nstmts > 0)
        printf("note: your own %s replaces the %lu statement(s) typed so far.\n",
               langpack_entry(c->pack), (unsigned long)c->s.nstmts);
    /* An import or an extern block names nothing worth reading back. */
    if (kind == RS_PKG || kind == RS_IMPORT || kind == RS_EXTERN)
        printf("ok.\n");
    else
        printf("%s %s.%s\n", again ? "redefined" : "defined", name,
               c->s.has_main ? " ':run' to execute." : "");
    fflush(stdout);
}

/* ------------------------------------------------------------------ */
/* commands                                                            */
/* ------------------------------------------------------------------ */

static void repl_help(const options_t *opt)
{
    if (opt->command == CMD_REPL_LAYOUT) {
        printf("Layout REPL commands:\n"
               "  :clear  erase accumulated session\n"
               "  :defs   show current session\n"
               "  :quit   exit  (also: :q  quit  exit  q  Ctrl-D)\n"
               "\nType a 'layout: ... end' block to render HTML inline.\n"
               "Blocks can span multiple lines - the REPL waits until every\n"
               "':' opener has a matching 'end' (always lowercase) before running.\n"
               "\nRoot-level attributes: title, lang, dir\n"
               "Common elements:  box  heading  p  link  image  input  button\n"
               "Element attrs:    content  color  background  font-size  ...\n"
               "\nExample:\n"
               "  layout: title = \"Hi\" box: heading: size = 1 content = \"Hello\" "
               "end end end\n");
        return;
    }
    printf("Commands:\n"
           "  :run        rebuild and run the whole session, showing all output\n"
           "  :defs       print the session as one program\n"
           "  :undo       drop the last statement\n"
           "  :save FILE  write the session to FILE\n"
           "  :clear      erase the session\n"
           "  :quit       exit  (also: :q  quit  exit  q  Ctrl-D)\n"
           "  :abort      at the '...' prompt, abandon the block being typed\n"
           "\nDefinitions (func/struct/enum/type/const/import/extern) are kept and\n"
           "sorted into a legal program; typing a name again redefines it.\n"
           "Statements accumulate in a synthesized main, so a variable bound on one\n"
           "line is still there on the next. A bare expression is printed.\n"
           "Any block opened with ':' waits for a matching 'end' (lowercase).\n"
           "\nEvery turn re-runs the session from the top, so a statement with an\n"
           "outside effect (writing a file, sending a request) happens again each\n"
           "time. Only the new output is shown.\n");
}

static void repl_save(repl_ctx_t *c, const char *arg)
{
    char *src;
    FILE *f;
    while (*arg == ' ' || *arg == '\t')
        arg++;
    if (!*arg) {
        printf("usage: :save FILE\n");
        return;
    }
    f = fopen(arg, "wb");
    if (!f) {
        printf("could not write %s\n", arg);
        return;
    }
    src = repl_render(&c->s, c->pack, NULL);
    fputs(src, f);
    fclose(f);
    free(src);
    printf("saved to %s\n", arg);
}

/* Returns true when the line was a command and the turn is over. */
static bool repl_command(repl_ctx_t *c, const char *line, bool *quit)
{
    if (strcmp(line, ":help") == 0) {
        repl_help(c->opt);
    } else if (strcmp(line, ":clear") == 0) {
        repl_session_free(&c->s);
        repl_session_init(&c->s);
        free(c->last_out);
        c->last_out = NULL;
        c->last_len = 0;
        printf("session cleared.\n");
    } else if (strcmp(line, ":defs") == 0) {
        char *src = repl_render(&c->s, c->pack, NULL);
        printf("%s", (src && src[0]) ? src : "(empty)\n");
        free(src);
    } else if (strcmp(line, ":undo") == 0) {
        fputs(repl_session_undo(&c->s) ? "dropped the last statement.\n"
                                       : "no statement to drop.\n",
              stdout);
    } else if (strncmp(line, ":save", 5) == 0 && (line[5] == '\0' || line[5] == ' ')) {
        repl_save(c, line + 5);
    } else if (strcmp(line, ":quit") == 0 || strcmp(line, ":q") == 0 ||
               strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0 ||
               strcmp(line, "q") == 0) {
        *quit = true;
    } else {
        return false;
    }
    fflush(stdout);
    return true;
}

/* `main()` typed at the prompt reads as "run it", the same shorthand the
 * REPL has always accepted, in the session's own language. */
static bool repl_wants_run(const repl_ctx_t *c, const char *line)
{
    const char *entry = langpack_entry(c->pack);
    size_t n = strlen(entry);
    while (*line == ' ' || *line == '\t')
        line++;
    return strncmp(line, entry, n) == 0 && line[n] == '(' && line[n + 1] == ')';
}

static void repl_cmd_run(repl_ctx_t *c)
{
    char *src;
    if (c->opt->command == CMD_REPL_LAYOUT) {
        printf("layout REPL: type 'layout: ... end' blocks to render HTML.\n");
        return;
    }
    if (!c->s.has_main && c->s.nstmts == 0) {
        printf("nothing to run yet - type a statement, or define %s.\n",
               langpack_entry(c->pack));
        return;
    }
    src = repl_render(&c->s, c->pack, NULL);
    repl_exec(c, src, false);
    free(src);
}

/* ------------------------------------------------------------------ */
/* the layout REPL                                                     */
/* ------------------------------------------------------------------ */

static ast_node_t *repl_find_layout(ast_node_t *prog)
{
    size_t i = 0;
    for (; i < prog->list.len; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        if (d->kind == AST_LAYOUT_BLOCK) return d;
    }
    return NULL;
}

static void repl_layout_exec(repl_ctx_t *c, const char *src_text)
{
    arena_t *arena = arena_new(1 << 20);
    logger_t *log = logger_new(stderr, LOG_ERROR, resolve_color(c->opt->color));
    source_file_t sf;
    token_stream_t *toks = NULL;
    ast_node_t *prog = NULL;
    bool ok;
    sf.path = "<repl>";
    sf.text = CONST_CAST(src_text);
    sf.len = strlen(src_text);
    logger_set_diag_source(log, sf.text, sf.len, c->opt->diag_style, c->opt->diag_format);
    ok = lexer_run(arena, log, c->pack, &sf, &toks);
    ok = ok && parser_run(arena, log, toks, &prog);
    if (ok) {
        cc_table_t *cc = cc_table_build(arena, NULL, c->opt->defines, c->opt->ndefines);
        ok = cc_prune_program(arena, log, sf.path, cc, prog);
        if (ok) sema_run(arena, log, prog, sf.path, langpack_code(c->pack), cc);
    }
    if (ok) {
        ast_node_t *lb = repl_find_layout(prog);
        if (!lb) {
            fprintf(stderr, "error: no layout block found\n");
        } else {
            diag_engine_t *diag = diag_new(arena, log, PH_CODEGEN);
            layout_result_t *r = layout_generate(arena, log, diag, sf.path, lb, false);
            if (r && diag->errors == 0)
                printf("%s", layout_document(arena, r, true, NULL, NULL, false));
        }
    }
    fflush(stdout);
    logger_free(log);
    arena_free(arena);
}

static void repl_layout_turn(repl_ctx_t *c, const char *first, const char *block)
{
    const char *p = first;
    while (*p == ' ' || *p == '\t')
        p++;
    if (strncmp(p, "layout", 6) == 0 &&
        (p[6] == ':' || p[6] == ' ' || p[6] == '\t' || p[6] == '\0')) {
        repl_layout_exec(c, block);
        return;
    }
    fprintf(stderr, "error: layout REPL only accepts 'layout: ... end' blocks\n");
    fflush(stderr);
}

/* ------------------------------------------------------------------ */

static void repl_turn(repl_ctx_t *c, const char *line)
{
    char name[REPL_NAME_MAX];
    char *block = repl_read_block(c->pack, line);
    repl_kind_t kind;
    if (!block) return;
    if (c->opt->command == CMD_REPL_LAYOUT) {
        repl_layout_turn(c, line, block);
        free(block);
        return;
    }
    kind = repl_classify(c->pack, line, name, sizeof name);
    if (kind != RS_STMT)
        repl_turn_def(c, kind, name, block);
    else if (c->s.has_main)
        printf("%s is defined - ':run' to execute, ':clear' to start over.\n",
               langpack_entry(c->pack));
    else
        repl_turn_stmt(c, block);
    fflush(stdout);
    free(block);
}

int driver_repl(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    langpack_t *pack = langpack_load(opt->lang);
    repl_ctx_t c;
    const char *which;
    bool quit = false;
    if (!pack) {
        LOG_E(log, PH_DRIVER, "unknown language pack '%s'", opt->lang);
        logger_free(log);
        return 2;
    }
    /* A turn is a fragment of a program still being written: a binding
     * whose first use is on the next line, a helper defined before its
     * caller. See sema_set_relax_unused. */
    sema_set_relax_unused(true);
    memset(&c, 0, sizeof c);
    c.opt = opt;
    c.pack = pack;
    repl_session_init(&c.s);
    which = (opt->command == CMD_REPL_LAYOUT) ? "layout" : "general";
    printf("Salam %s REPL (lang=%s). ':help' for commands.\n", which, opt->lang);
    fflush(stdout);
    while (!quit) {
        char *line = repl_readline("salam> ");
        if (!line) break;
        if (!line[0]) {
            free(line);
            continue;
        }
        if (strcmp(line, ":run") == 0 || repl_wants_run(&c, line))
            repl_cmd_run(&c);
        else if (!repl_command(&c, line, &quit))
            repl_turn(&c, line);
        repl_hist_push(line);
        free(line);
    }
    printf("bye.\n");
    repl_session_free(&c.s);
    free(c.last_out);
    repl_hist_clear();
    logger_free(log);
    return 0;
}

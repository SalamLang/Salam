/*
 * Salam Programming Language (2024–2026)
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

#include "core/prelude.h"
#include "driver/driver.h"
#include "driver/repl.h"
#include "core/arena.h"
#include "core/sb.h"
#include "cli/options.h"
#include "langpack/langpack.h"
#include "lexer/lexer.h"
#include "logger/logger.h"
#include "source/source.h"
#include "preproc/preproc.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "codegen/codegen.h"
#include "layout/layout_gen.h"
#include "diag/diag.h"

#if defined(_WIN32)
#  include <windows.h>
#  define REPL_USE_WINCON 1
#else
#  include <termios.h>
#  include <unistd.h>
#  define REPL_USE_WINCON 0
#endif

#define REPL_HIST_MAX  200
#define REPL_LINE_MAX 4096
static char *repl_hist[REPL_HIST_MAX];
static int   repl_nhist = 0;

static void repl_hist_push(const char *line)
{
    if (!line || !line[0]) return;
    if (repl_nhist > 0 && strcmp(repl_hist[repl_nhist - 1], line) == 0) return;
    if (repl_nhist == REPL_HIST_MAX) {
        free(repl_hist[0]);
        memmove(repl_hist, repl_hist + 1, (REPL_HIST_MAX - 1) * sizeof *repl_hist);
        repl_nhist--;
    }
    repl_hist[repl_nhist++] = sal_strdup(line);
}

static char *repl_readline(const char *prompt)
{
#if REPL_USE_WINCON
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE || GetFileType(hIn) != FILE_TYPE_CHAR) {
        printf("%s", prompt); fflush(stdout);
        char *buf = (char *)malloc(REPL_LINE_MAX);
        if (!buf || !fgets(buf, REPL_LINE_MAX, stdin)) { free(buf); return NULL; }
        size_t n = strlen(buf);
        while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
        return buf;
    }
    DWORD orig; GetConsoleMode(hIn, &orig);
    SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT);
    printf("%s", prompt); fflush(stdout);
    char *buf = (char *)malloc(REPL_LINE_MAX);
    if (!buf) { SetConsoleMode(hIn, orig); return NULL; }
    int len = 0, pos = 0, hi = repl_nhist;
    char saved[REPL_LINE_MAX]; saved[0] = '\0'; int saved_len = 0;
    for (;;) {
        DWORD nr; INPUT_RECORD ev;
        if (!ReadConsoleInputA(hIn, &ev, 1, &nr)) break;
        if (ev.EventType != KEY_EVENT || !ev.Event.KeyEvent.bKeyDown) continue;
        WORD vk = ev.Event.KeyEvent.wVirtualKeyCode;
        char ch = ev.Event.KeyEvent.uChar.AsciiChar;
        if (vk == VK_RETURN) { putchar('\n'); fflush(stdout); buf[len] = '\0'; break; }
        if (vk == VK_BACK && pos > 0) {
            memmove(buf+pos-1, buf+pos, (size_t)(len-pos)); pos--; len--;
            printf("\r%s%.*s \r%s%.*s", prompt, len, buf, prompt, pos, buf); fflush(stdout); continue;
        }
        if (vk == VK_DELETE && pos < len) {
            memmove(buf+pos, buf+pos+1, (size_t)(len-pos-1)); len--;
            printf("\r%s%.*s \r%s%.*s", prompt, len, buf, prompt, pos, buf); fflush(stdout); continue;
        }
        if (vk == VK_LEFT  && pos > 0)   { pos--; printf("\r%s%.*s", prompt, pos, buf); fflush(stdout); continue; }
        if (vk == VK_RIGHT && pos < len) { pos++; printf("\r%s%.*s", prompt, pos, buf); fflush(stdout); continue; }
        if (vk == VK_HOME) { pos = 0;   printf("\r%s",          prompt);       fflush(stdout); continue; }
        if (vk == VK_END)  { pos = len; printf("\r%s%.*s", prompt, pos, buf); fflush(stdout); continue; }
        if (vk == VK_UP) {
            if (hi == repl_nhist) { memcpy(saved, buf, (size_t)len); saved[len] = '\0'; saved_len = len; }
            if (hi > 0) {
                hi--; int hlen = (int)strlen(repl_hist[hi]);
                printf("\r%s%*s\r%s%.*s", prompt, len+1, "", prompt, hlen, repl_hist[hi]);
                memcpy(buf, repl_hist[hi], (size_t)(hlen+1)); len = pos = hlen; fflush(stdout);
            }
            continue;
        }
        if (vk == VK_DOWN) {
            if (hi < repl_nhist) {
                hi++; const char *h = (hi == repl_nhist) ? saved : repl_hist[hi];
                int hlen = (hi == repl_nhist) ? saved_len : (int)strlen(h);
                printf("\r%s%*s\r%s%.*s", prompt, len+1, "", prompt, hlen, h);
                memcpy(buf, h, (size_t)((hlen < REPL_LINE_MAX-1 ? hlen : REPL_LINE_MAX-2)+1));
                len = pos = hlen; fflush(stdout);
            }
            continue;
        }
        if (ch == 1)  { pos = 0;   printf("\r%s",          prompt);       fflush(stdout); continue; }
        if (ch == 5)  { pos = len; printf("\r%s%.*s", prompt, pos, buf); fflush(stdout); continue; }
        if (ch == 21) { printf("\r%s%*s\r%s", prompt, len+1, "", prompt); len = pos = 0; fflush(stdout); continue; }
        if (ch == 3 || (ch == 4 && len == 0)) {
            printf("\n"); fflush(stdout); SetConsoleMode(hIn, orig); free(buf); return NULL;
        }
        if ((unsigned char)ch >= 32 && len < REPL_LINE_MAX - 1) {
            memmove(buf+pos+1, buf+pos, (size_t)(len-pos)); buf[pos] = ch; len++;
            printf("\r%s%.*s \r%s%.*s", prompt, len, buf, prompt, pos+1, buf);
            pos++; fflush(stdout);
        }
    }
    SetConsoleMode(hIn, orig);
    return buf;
#else 
    if (!isatty(STDIN_FILENO)) {
        fputs(prompt, stdout); fflush(stdout);
        char *buf = (char *)malloc(REPL_LINE_MAX);
        if (!buf || !fgets(buf, REPL_LINE_MAX, stdin)) { free(buf); return NULL; }
        size_t n = strlen(buf);
        while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) buf[--n] = '\0';
        return buf;
    }
    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_iflag &= ~((tcflag_t)(BRKINT | ICRNL | INPCK | ISTRIP | IXON));
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~((tcflag_t)(ECHO | ICANON | IEXTEN | ISIG));
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    fputs(prompt, stdout); fflush(stdout);
    char *buf = (char *)malloc(REPL_LINE_MAX);
    if (!buf) { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig); return NULL; }
    int len = 0, pos = 0, hi = repl_nhist;
    char saved[REPL_LINE_MAX]; saved[0] = '\0'; int saved_len = 0;
#define WR(s,n)  do { ssize_t _wr = write(STDOUT_FILENO, (s), (size_t)(n)); (void)_wr; } while(0)
#define WRC(c)   do { char _wc = (char)(c); WR(&_wc, 1); } while(0)
    for (;;) {
        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) <= 0) { if (len == 0) { free(buf); buf = NULL; } break; }
        if (c == '\r' || c == '\n') { WR("\r\n", 2); break; }
        if (c == 127 || c == 8) {
            if (pos > 0) {
                memmove(buf+pos-1, buf+pos, (size_t)(len-pos)); pos--; len--;
                WRC('\b'); WR(buf+pos, len-pos); WRC(' ');
                { int i = pos; for (; i <= len; i++) WRC('\b'); }
                fflush(stdout);
            }
            continue;
        }
        if (c == 1)  { { int i = 0; for (; i < pos; i++) WRC('\b'); } pos = 0; fflush(stdout); continue; }
        if (c == 5)  { WR(buf+pos, len-pos); pos = len; fflush(stdout); continue; }
        if (c == 21) {
            { int i = 0; for (; i < pos; i++) WRC('\b'); }
            { int i = 0; for (; i < len; i++) WRC(' '); }
            { int i = 0; for (; i < len; i++) WRC('\b'); }
            len = pos = 0; fflush(stdout); continue;
        }
        if (c == 3 || (c == 4 && len == 0)) {
            WR("\r\n", 2); fflush(stdout); tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
            free(buf); return NULL;
        }
        if (c == 27) {
            unsigned char s1, s2;
            if (read(STDIN_FILENO, &s1, 1) <= 0 || s1 != '[') continue;
            if (read(STDIN_FILENO, &s2, 1) <= 0) continue;
            if (s2 == 'A') { 
                if (hi == repl_nhist) { memcpy(saved, buf, (size_t)len); saved[len] = '\0'; saved_len = len; }
                if (hi > 0) {
                    hi--; { int i = 0; for (; i < pos; i++) WRC('\b'); }
                    { int i = 0; for (; i < len; i++) WRC(' '); }
                    { int i = 0; for (; i < len; i++) WRC('\b'); }
                    int hlen = (int)strlen(repl_hist[hi]);
                    memcpy(buf, repl_hist[hi], (size_t)(hlen+1)); len = pos = hlen;
                    WR(buf, len); fflush(stdout);
                }
            } else if (s2 == 'B') { 
                if (hi < repl_nhist) {
                    hi++; const char *h = (hi == repl_nhist) ? saved : repl_hist[hi];
                    int hlen = (hi == repl_nhist) ? saved_len : (int)strlen(h);
                    { int i = 0; for (; i < pos; i++) WRC('\b'); }
                    { int i = 0; for (; i < len; i++) WRC(' '); }
                    { int i = 0; for (; i < len; i++) WRC('\b'); }
                    memcpy(buf, h, (size_t)(hlen+1)); len = pos = hlen;
                    WR(buf, len); fflush(stdout);
                }
            } else if (s2 == 'C' && pos < len) { WR(buf+pos, 1); pos++; fflush(stdout); }
            else if   (s2 == 'D' && pos > 0)  { WRC('\b'); pos--; fflush(stdout); }
            else if (s2 == '3') {
                unsigned char tilde;
                if (read(STDIN_FILENO, &tilde, 1) <= 0) continue;  
                if (tilde == '~' && pos < len) {
                    memmove(buf+pos, buf+pos+1, (size_t)(len-pos-1)); len--;
                    WR(buf+pos, len-pos); WRC(' ');
                    { int i = pos; for (; i <= len; i++) WRC('\b'); }
                    fflush(stdout);
                }
            }
            continue;
        }
        if (c >= 32 && len < REPL_LINE_MAX - 1) {
            memmove(buf+pos+1, buf+pos, (size_t)(len-pos)); buf[pos] = (char)c; len++;
            WR(buf+pos, len-pos); { int i = pos+1; for (; i < len; i++) WRC('\b'); }
            pos++; fflush(stdout);
        }
    }
#undef WR
#undef WRC
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    if (buf) buf[len] = '\0';
    return buf;
#endif 
}

static bool repl_is_def(const char *line)
{
    while (*line == ' ' || *line == '\t') line++;
    static const char *kws[] = {
        "func ", "struct ", "enum ", "type ",
        "import ", "package ", "link ", "//", NULL
    };
    { int i = 0; for (; kws[i]; i++)
        if (strncmp(line, kws[i], strlen(kws[i])) == 0) return true; }
    return false;
}

static int repl_block_depth(const langpack_t *pack, const char *src,
                            bool layout_mode)
{
    (void)layout_mode;
    arena_t  *a  = arena_new(1 << 16);
    logger_t *lg = logger_new(stderr, LOG_OFF, false);
    source_file_t sf;
    sf.path = "<repl>"; sf.text = CONST_CAST(src); sf.len = strlen(src);
    token_stream_t *toks = NULL;
    int depth = 0;
    if (lexer_run(a, lg, pack, &sf, &toks)) {
        size_t n = token_stream_count(toks);
        { size_t i = 0; for (; i < n; i++) {
            const token_t *t = token_stream_at(toks, i);
            if (t->kind == TK_KW_END) {
                depth--;
            } else if (t->kind == TK_COLON) {
                uint32_t idx = t->span.begin.index;
                int lws = (idx == 0) ||
                          src[idx-1]==' '||src[idx-1]=='\t'||
                          src[idx-1]=='\n'||src[idx-1]=='\r';
                char nx = src[idx+1];
                int nxt = nx=='\0'||nx==' '||nx=='\t'||nx=='\n'||nx=='\r';
                if (!lws && nxt) depth++;
            }
        } }
    }
    logger_free(lg); arena_free(a);
    return depth;
}

static bool repl_wants_run(const char *line)
{
    while (*line == ' ' || *line == '\t') line++;
    return strncmp(line, "main()", 6) == 0;
}

static bool repl_compile_pkg(arena_t *a, logger_t *lg, langpack_t *pack,
                             options_t *opt, const char *pkg)
{
    salam_set_stdlib_root(opt->stdlib_path);
    const char *path = salam_resolve_import(a, "", pkg);
    source_file_t *src = path ? source_load(a, path) : NULL;
    if (!src) return false;
    src = preproc_source(a, lg, src, NULL, 0);     
    token_stream_t *toks = NULL;
    if (!lexer_run(a, lg, pack, src, &toks)) return false;
    ast_node_t *prog = NULL;
    if (!parser_run(a, lg, toks, &prog)) return false;
    sema_result_t *sr = sema_run(a, lg, prog, src->path);
    if (!sr || !sr->ok) return false;
    codegen_output_t *out = codegen_run(a, lg, prog, sr, pkg, false, false,
                                        src->path, langpack_entry(pack));
    char cpath[64], hpath[64];
    snprintf(cpath, sizeof cpath, "salam_mod_%s.c", pkg);
    snprintf(hpath, sizeof hpath, "salam_mod_%s.h", pkg);
    FILE *f;
    if ((f = fopen(cpath, "wb"))) { fputs(out->c_src, f); fclose(f); }
    if ((f = fopen(hpath, "wb"))) { fputs(out->h_src, f); fclose(f); }
    return true;
}

static bool repl_exec(const char *full_src, const char *cc,
                      langpack_t *pack, options_t *opt)
{
    arena_t  *a  = arena_new(1 << 20);
    logger_t *lg = logger_new(stderr, LOG_ERROR, resolve_color(opt->color));
    source_file_t sf;
    sf.path = "<repl>"; sf.text = CONST_CAST(full_src); sf.len = strlen(full_src);
    logger_set_diag_source(lg, sf.text, sf.len, opt->diag_style, opt->diag_format);
    token_stream_t *toks = NULL;
    bool ok = lexer_run(a, lg, pack, &sf, &toks);
    ast_node_t *prog = NULL;
    ok = ok && parser_run(a, lg, toks, &prog);
    sema_result_t *sr = ok ? sema_run(a, lg, prog, sf.path) : NULL;
    ok = sr && sr->ok;
    if (!ok) { logger_free(lg); arena_free(a); return false; }
    codegen_output_t *out = codegen_run(a, lg, prog, sr, "_repl_", false, false, sf.path,
                                        langpack_entry(pack));
    FILE *f;
    if ((f = fopen("salam_mod__repl_.c", "wb"))) { fputs(out->c_src, f); fclose(f); }
    if ((f = fopen("salam_mod__repl_.h", "wb"))) { fputs(out->h_src, f); fclose(f); }
    
    static const char *const RT_PKGS[] = { "mem", "core", "map", "text", "console", "fs", "thread" };
    bool have_pkg[sizeof(RT_PKGS)/sizeof(RT_PKGS[0])];
    { size_t i = 0; for (; i < sizeof(RT_PKGS)/sizeof(RT_PKGS[0]); i++)
        have_pkg[i] = repl_compile_pkg(a, lg, pack, opt, RT_PKGS[i]); }
    sb_t cmd; sb_init(&cmd);
    sb_puts(&cmd, cc);
    sb_puts(&cmd, " -I. -o _salam_repl_");
#ifdef _WIN32
    sb_puts(&cmd, ".exe");
#endif
    sb_puts(&cmd, " salam_mod__repl_.c");
    { size_t i = 0; for (; i < sizeof(RT_PKGS)/sizeof(RT_PKGS[0]); i++)
        if (have_pkg[i]) { sb_puts(&cmd, " salam_mod_"); sb_puts(&cmd, RT_PKGS[i]); sb_puts(&cmd, ".c"); } }
#ifdef _WIN32
    sb_puts(&cmd, " -lmsvcrt -lws2_32 -lwinhttp");
#else
    sb_puts(&cmd, " -lm");
#endif
    int crc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    if (crc == 0) {
#ifdef _WIN32
        int _src = system(".\\_salam_repl_.exe");
#else
        int _src = system("./_salam_repl_");
#endif
        (void)_src;
        fflush(stdout);
    }
    remove("salam_mod__repl_.c"); remove("salam_mod__repl_.h");
    { size_t i = 0; for (; i < sizeof(RT_PKGS)/sizeof(RT_PKGS[0]); i++) {
        char cp[64], hp[64];
        snprintf(cp, sizeof cp, "salam_mod_%s.c", RT_PKGS[i]);
        snprintf(hp, sizeof hp, "salam_mod_%s.h", RT_PKGS[i]);
        remove(cp); remove(hp);
    } }
#ifdef _WIN32
    remove("_salam_repl_.exe");
#else
    remove("./_salam_repl_");
#endif
    logger_free(lg); arena_free(a);
    return crc == 0;
}

static ast_node_t *repl_find_layout(ast_node_t *prog)
{
    { size_t i = 0; for (; i < prog->list.len; i++) {
        ast_node_t *d = (ast_node_t *)prog->list.data[i];
        if (d->kind == AST_LAYOUT_BLOCK) return d;
    } }
    return NULL;
}

static void repl_layout_exec(const char *src_text, const langpack_t *pack,
                             options_t *opt)
{
    arena_t  *arena = arena_new(1 << 20);
    logger_t *log   = logger_new(stderr, LOG_ERROR, resolve_color(opt->color));
    source_file_t sf;
    sf.path = "<repl>"; sf.text = CONST_CAST(src_text); sf.len = strlen(src_text);
    logger_set_diag_source(log, sf.text, sf.len, opt->diag_style, opt->diag_format);
    token_stream_t *toks = NULL;
    bool ok = lexer_run(arena, log, pack, &sf, &toks);
    ast_node_t *prog = NULL;
    ok = ok && parser_run(arena, log, toks, &prog);
    if (ok) sema_run(arena, log, prog, sf.path);
    if (ok) {
        ast_node_t *lb = repl_find_layout(prog);
        if (lb) {
            diag_engine_t *diag = diag_new(arena, log, PH_CODEGEN);
            layout_result_t *r = layout_generate(arena, log, diag, sf.path, lb);
            if (r && diag->errors == 0) {
                
                printf("%s", layout_document(arena, r, true, NULL, NULL));
            }
        } else {
            fprintf(stderr, "error: no layout block found\n");
        }
    }
    fflush(stdout);
    logger_free(log); arena_free(arena);
}

int driver_repl(options_t *opt)
{
    logger_t   *log   = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    arena_t    *arena = arena_new(1 << 16);
    langpack_t *pack  = langpack_load(opt->lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, "unknown language pack '%s'", opt->lang);
        logger_free(log); arena_free(arena); return 2;
    }
    const char *which = (opt->command == CMD_REPL_LAYOUT) ? "layout" : "general";
    printf("Salam %s REPL (lang=%s). ':help' for commands.\n", which, opt->lang);
    fflush(stdout);
    sb_t   session; sb_init(&session);
    bool   has_main = false;
    const char *cc  = (opt->cc && opt->cc[0]) ? opt->cc : "tcc";
    for (;;) {
        char *line = repl_readline("salam> ");
        if (!line) break;
        if (!line[0]) { free(line); continue; }
        
        if (strcmp(line, ":quit") == 0 || strcmp(line, ":q")   == 0 ||
            strcmp(line, "quit")  == 0 || strcmp(line, "exit")  == 0 ||
            strcmp(line, "q")     == 0) { free(line); break; }
        
        if (strcmp(line, ":help") == 0) {
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
                       "  layout: title = \"Hi\" box: heading: size = 1 content = \"Hello\" end end end\n");
            } else {
                printf("Commands:\n"
                       "  :run    compile + run the session (requires func main:)\n"
                       "  :clear  erase all accumulated definitions\n"
                       "  :defs   print current session\n"
                       "  :quit   exit  (also: :q  quit  exit  q  Ctrl-D)\n"
                       "\nTop-level defs (func/struct/enum/type) are accumulated.\n"
                       "Statements are wrapped in func main: and executed immediately.\n"
                       "Any block opened with ':' waits for a matching 'end' (lowercase).\n");
            }
            fflush(stdout); free(line); continue;
        }
        if (strcmp(line, ":clear") == 0) {
            sb_free(&session); sb_init(&session); has_main = false;
            printf("session cleared.\n"); fflush(stdout); free(line); continue;
        }
        if (strcmp(line, ":defs") == 0) {
            const char *s = sb_cstr(&session);
            printf("%s\n", (s && s[0]) ? s : "(empty)"); fflush(stdout);
            free(line); continue;
        }
        if (strcmp(line, ":run") == 0 || repl_wants_run(line)) {
            if (opt->command == CMD_REPL_LAYOUT)
                printf("layout REPL: type 'layout: ... end' blocks to render HTML.\n");
            else if (!has_main)
                printf("no func main defined - define one, or type a statement.\n");
            else
                repl_exec(sb_cstr(&session), cc, pack, opt);
            fflush(stdout); repl_hist_push(line); free(line); continue;
        }

        sb_t block; sb_init(&block);
        sb_puts(&block, line); sb_putc(&block, '\n');
        bool is_layout = (opt->command == CMD_REPL_LAYOUT);
        while (repl_block_depth(pack, sb_cstr(&block), is_layout) > 0) {
            char *cont = repl_readline("  ... ");
            if (!cont) break;
            sb_puts(&block, cont); sb_putc(&block, '\n');
            repl_hist_push(cont); free(cont);
        }
        const char *first = line;
        while (*first == ' ' || *first == '\t') first++;
        
        if (opt->command == CMD_REPL_LAYOUT) {
            if (strncmp(first, "layout", 6) == 0 &&
                (first[6]==':'||first[6]==' '||first[6]=='\t'||first[6]=='\0')) {
                repl_layout_exec(sb_cstr(&block), pack, opt);
            } else {
                fprintf(stderr, "error: layout REPL only accepts 'layout: ... end' blocks\n");
                fflush(stderr);
            }
            sb_free(&block); repl_hist_push(line); free(line); continue;
        }

        if (repl_is_def(first)) {
            sb_t test; sb_init(&test);
            sb_puts(&test, sb_cstr(&session));
            sb_puts(&test, sb_cstr(&block));
            source_file_t sf;
            sf.path = "<repl>"; sf.text = CONST_CAST(sb_cstr(&test)); sf.len = strlen(sf.text);
            logger_t *vl = logger_new(stderr, LOG_ERROR, resolve_color(opt->color));
            logger_set_diag_source(vl, sf.text, sf.len, opt->diag_style, opt->diag_format);
            arena_t *va = arena_new(1 << 20);
            token_stream_t *toks = NULL;
            bool ok = lexer_run(va, vl, pack, &sf, &toks);
            ast_node_t *prog = NULL;
            ok = ok && parser_run(va, vl, toks, &prog);
            sema_result_t *sr = ok ? sema_run(va, vl, prog, sf.path) : NULL;
            ok = sr && sr->ok;
            arena_free(va); logger_free(vl); sb_free(&test);
            if (ok) {
                sb_puts(&session, sb_cstr(&block));
                if (strncmp(first, "func main", 9) == 0) {
                    has_main = true;
                    printf("defined. ':run' to execute.\n");
                } else {
                    printf("defined.\n");
                }
                fflush(stdout);
            }
            sb_free(&block); repl_hist_push(line); free(line); continue;
        }

        if (has_main) {
            printf("func main is defined - ':run' to execute, ':clear' to reset.\n");
            fflush(stdout);
        } else {
            sb_t full; sb_init(&full);
            sb_puts(&full, sb_cstr(&session));
            sb_puts(&full, "\nfunc main:\n");
            sb_puts(&full, sb_cstr(&block));
            sb_puts(&full, "end\n");
            repl_exec(sb_cstr(&full), cc, pack, opt);
            sb_free(&full);
            fflush(stdout);
        }
        sb_free(&block); repl_hist_push(line); free(line);
    }
    printf("bye.\n");
    sb_free(&session);
    { int i = 0; for (; i < repl_nhist; i++) free(repl_hist[i]); }
    repl_nhist = 0;
    logger_free(log); arena_free(arena);
    return 0;
}

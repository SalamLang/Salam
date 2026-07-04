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
#include "core/sal_format.h"
#include "core/build_info.h"
#include "driver/driver.h"
#include "driver/build.h"
#include "driver/llvm_build.h"
#include "driver/layout_build.h"
#include "driver/debug_cmd.h"
#include "driver/repl.h"
#include "core/arena.h"
#include "core/sb.h"
#include "cli/options.h"
#include "langpack/langpack.h"
#include "lexer/lexer.h"
#include "logger/logger.h"
#include "source/source.h"
#include "token/token.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "semantic/sema.h"
#include "interp/interp.h"
#include "preproc/preproc.h"
#include "xml/xml.h"
#include "i18n/i18n.h"
#include "layout/schema.h"
#include "fmt/fmt.h"
#include <errno.h>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <io.h>
#  include <direct.h>
#  include <process.h>
#  define salam_isatty(fd) _isatty(fd)
#  define salam_fileno(f)  _fileno(f)
#  define salam_mkdir(p)   _mkdir(p)
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <dirent.h>
#  include <sys/wait.h>
#  define salam_isatty(fd) isatty(fd)
#  define salam_fileno(f)  fileno(f)
#  define salam_mkdir(p)   mkdir((p), 0755)
#endif

bool resolve_color(int flag)
{
    if (flag == 1) return true;
    if (flag == 0) return false;
    return salam_isatty(salam_fileno(stderr)) != 0;   
}

static int write_xml(logger_t *log, const sb_t *sb, const char *xml_out, const char *what)
{
    if (xml_out) {
        FILE *f = fopen(xml_out, "wb");
        if (!f) {
            LOG_E(log, PH_DRIVER, i18n_tr("cannot open '%s' for writing"), xml_out);
            return 2;
        }
        fputs(sb_cstr(sb), f);
        fclose(f);
        LOG_I(log, PH_DRIVER, "%s XML written to %s", what, xml_out);
    } else {
        fputs(sb_cstr(sb), stdout);
    }
    return 0;
}

static int emit_tokens_xml(logger_t *log, const token_stream_t *toks, const char *xml_out)
{
    sb_t sb; sb_init(&sb);
    xml_writer_t *w = xml_new(&sb);
    xml_decl(w);
    tokens_to_xml(w, toks);
    xml_free(w);
    int rc = write_xml(log, &sb, xml_out, "tokens");
    sb_free(&sb);
    return rc;
}

static int emit_ast_xml(logger_t *log, const ast_node_t *program, const char *xml_out)
{
    sb_t sb; sb_init(&sb);
    xml_writer_t *w = xml_new(&sb);
    xml_decl(w);
    ast_to_xml(w, program);
    xml_free(w);
    int rc = write_xml(log, &sb, xml_out, "AST");
    sb_free(&sb);
    return rc;
}

static int emit_symbol_xml(logger_t *log, const sema_result_t *sr, const char *xml_out)
{
    sb_t sb; sb_init(&sb);
    xml_writer_t *w = xml_new(&sb);
    xml_decl(w);
    symbols_to_xml(w, sr);
    xml_free(w);
    int rc = write_xml(log, &sb, xml_out, "symbols");
    sb_free(&sb);
    return rc;
}

static int driver_new(options_t *opt)
{
    char namebuf[256];
    const char *name = opt->new_name;
    if (!name || !name[0]) {
        printf("Project name: ");
        fflush(stdout);
        if (!fgets(namebuf, sizeof namebuf, stdin)) {
            fprintf(stderr, "%s", i18n_tr("salam: no project name provided\n"));
            return 2;
        }
        size_t n = strlen(namebuf);
        while (n > 0 && (namebuf[n-1] == '\n' || namebuf[n-1] == '\r' ||
                         namebuf[n-1] == ' '  || namebuf[n-1] == '\t')) namebuf[--n] = '\0';
        char *p = namebuf;
        while (*p == ' ' || *p == '\t') p++;
        name = p;
    }
    if (!name[0]) {
        fprintf(stderr, "%s", i18n_tr("salam: project name cannot be empty\n"));
        return 2;
    }
    if (strchr(name, '/') || strchr(name, '\\')) {
        fprintf(stderr, i18n_tr("salam: project name must not contain path separators ('%s')\n"), name);
        return 2;
    }
    if (salam_mkdir(name) != 0) {
        if (errno == EEXIST)
            fprintf(stderr, i18n_tr("salam: '%s' already exists\n"), name);
        else
            fprintf(stderr, i18n_tr("salam: cannot create directory '%s': %s\n"), name, strerror(errno));
        return 2;
    }
    bool fa = opt->lang && strcmp(opt->lang, "fa") == 0;
    
    const char *en_content =
        "func main:\n"
        "    println(\"Hello, World!\")\n"
        "end\n";
    
    const char *fa_content =
        "\xD8\xAA\xD8\xA7\xD8\xA8\xD8\xB9 main:\n"
        "    \xDA\x86\xD8\xA7\xD9\xBE(\"\xD8\xB3\xD9\x84\xD8\xA7\xD9\x85\xD8\x8C "
        "\xD8\xAF\xD9\x86\xDB\x8C\xD8\xA7!\")\n"
        "\xD9\xBE\xD8\xA7\xDB\x8C\xD8\xA7\xD9\x86\n";
    const char *content = fa ? fa_content : en_content;
    char path[512];
    sal_snprintf(path, sizeof path, "%s/main.salam", name);
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, i18n_tr("salam: cannot write '%s': %s\n"), path, strerror(errno));
        return 2;
    }
    fputs(content, f);
    fclose(f);
    printf("Created project '%s':\n", name);
    printf("  %s\n", path);
    printf("\nNext steps:\n");
    printf("  salam build %s --output=%s%s\n", path, name,
#if defined(_WIN32)
           ".exe");
#else
           "");
#endif
    if (fa)
        printf("  (\xD8\xB2\xD8\xA8\xD8\xA7\xD9\x86 \xD9\x81\xD8\xA7\xD8\xB1\xD8\xB3\xDB\x8C)\n");
    return 0;
}

static void list_salam_files(arena_t *a, const char **out, int *n)
{
    *n = 0;
#if defined(_WIN32)
    struct _finddata_t fd;
    intptr_t h = _findfirst("*.salam", &fd);
    if (h == -1) return;
    do {
        if (!(fd.attrib & _A_SUBDIR) && *n < SALAM_MAX_INPUTS)
            out[(*n)++] = arena_strdup(a, fd.name);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(".");
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && *n < SALAM_MAX_INPUTS) {
        size_t L = strlen(e->d_name);
        if (L > 6 && strcmp(e->d_name + L - 6, ".salam") == 0)
            out[(*n)++] = arena_strdup(a, e->d_name);
    }
    closedir(d);
#endif
}

static bool file_has_entry(arena_t *a, langpack_t *pack, const char *entry, const char *path)
{
    logger_t *quiet = logger_new(stderr, LOG_OFF, false);
    source_file_t *src = source_load(a, path);
    bool found = false;
    if (src) {
        src = preproc_source(a, quiet, src, NULL, 0);
        token_stream_t *toks = NULL;
        lexer_run(a, quiet, pack, src, &toks);
        ast_node_t *program = NULL;
        parser_run(a, quiet, toks, &program);
        if (program) {
            { size_t i = 0; for (; i < program->list.len; i++) {
                ast_node_t *d = (ast_node_t *)program->list.data[i];
                if (d->kind == AST_FUNC_DEF && d->name && strcmp(d->name, entry) == 0) {
                    found = true;
                    break;
                }
            } }
        }
    }
    logger_free(quiet);
    return found;
}

static int driver_run(options_t *opt)
{
    if (opt->log_level == LOG_INFO) opt->log_level = LOG_WARN;
    logger_t *log   = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    arena_t  *arena = arena_new(1 << 20);
    int rc = 0;
    
    if (opt->input_count == 0) {
        langpack_t *pack = langpack_load(opt->lang);
        if (!pack) { LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
                     arena_free(arena); logger_free(log); return 2; }
        const char *entry = langpack_entry(pack);   
        const char *files[SALAM_MAX_INPUTS]; int nfiles = 0;
        list_salam_files(arena, files, &nfiles);
        if (nfiles == 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("no .salam files found in the current directory"));
            arena_free(arena); logger_free(log); return 2;
        }
        const char *entries[SALAM_MAX_INPUTS]; int nentries = 0;
        { int i = 0; for (; i < nfiles; i++)
            if (file_has_entry(arena, pack, entry, files[i])) entries[nentries++] = files[i]; }
        if (nentries == 0) {
            LOG_E(log, PH_DRIVER,
                  i18n_tr("no entry point: none of the %d .salam file(s) here defines a '%s' function"),
                  nfiles, entry);
            arena_free(arena); logger_free(log); return 2;
        }
        if (nentries > 1) {
            LOG_E(log, PH_DRIVER, i18n_tr("ambiguous entry point: %d files define '%s':"), nentries, entry);
            { int i = 0; for (; i < nentries; i++)
                fprintf(stderr, "    %s\n", entries[i]); }
            fprintf(stderr, "  run a specific one with: salam run <file.salam>\n");
            arena_free(arena); logger_free(log); return 2;
        }
        opt->inputs[0]  = entries[0];
        opt->input_count = 1;
        opt->input       = entries[0];
        LOG_I(log, PH_DRIVER, i18n_tr("entry point: %s"), entries[0]);
    }
    
    char tmp_exe[600] = {0};
    bool temp_exe = (opt->output == NULL);
    if (temp_exe) {
#if defined(_WIN32)
        const char *t = getenv("TEMP"); if (!t) t = getenv("TMP"); if (!t) t = ".";
        sal_snprintf(tmp_exe, sizeof tmp_exe, "%s\\salam-run-%lu.exe", t, (unsigned long)_getpid());
#else
        const char *t = getenv("TMPDIR"); if (!t) t = "/tmp";
        sal_snprintf(tmp_exe, sizeof tmp_exe, "%s/salam-run-%lu", t, (unsigned long)getpid());
#endif
        opt->output = tmp_exe;
    }
    opt->command = CMD_BUILD;   
    opt->keep_c  = false;       
    
    int brc = driver_build(opt);
    if (brc != 0) {
        if (temp_exe) remove(opt->output);
        arena_free(arena); logger_free(log);
        return brc;
    }
    
    const char *exe = opt->exe_path[0] ? opt->exe_path : opt->output;
    char cmd[700];
#if defined(_WIN32)
    sal_snprintf(cmd, sizeof cmd, "\"%s\"", exe);
#else
    if (strchr(exe, '/')) sal_snprintf(cmd, sizeof cmd, "\"%s\"", exe);
    else                  sal_snprintf(cmd, sizeof cmd, "\"./%s\"", exe);
#endif
    int run_rc = system(cmd);
    if (temp_exe) remove(exe);
#if defined(_WIN32)
    rc = run_rc;
#else
    rc = WIFEXITED(run_rc) ? WEXITSTATUS(run_rc) : 1;
#endif
    arena_free(arena); logger_free(log);
    return rc;
}

static int driver_interp(options_t *opt)
{
    if (opt->log_level == LOG_INFO) opt->log_level = LOG_WARN;
    logger_t *log   = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    arena_t  *arena = arena_new(1 << 20);
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) { LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
                 arena_free(arena); logger_free(log); return 2; }
    const char *entry = langpack_entry(pack);
    
    if (opt->input == NULL) {
        const char *files[SALAM_MAX_INPUTS]; int nfiles = 0;
        list_salam_files(arena, files, &nfiles);
        const char *entries[SALAM_MAX_INPUTS]; int nentries = 0;
        { int i = 0; for (; i < nfiles; i++)
            if (file_has_entry(arena, pack, entry, files[i])) entries[nentries++] = files[i]; }
        if (nentries != 1) {
            LOG_E(log, PH_DRIVER, i18n_tr("no entry point: define a '%s' function"), entry);
            arena_free(arena); logger_free(log); return 2;
        }
        opt->input = entries[0];
    }
    salam_set_stdlib_root(opt->stdlib_path);
    source_file_t *src = source_load(arena, opt->input);
    if (!src) { LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), opt->input);
                arena_free(arena); logger_free(log); return 2; }
    src = preproc_source(arena, log, src, opt->defines, opt->ndefines);
    logger_set_diag_source(log, src->text, src->len, opt->diag_style, opt->diag_format);
    token_stream_t *toks = NULL;
    bool lok = lexer_run(arena, log, pack, src, &toks);
    ast_node_t *program = NULL;
    bool pok = parser_run(arena, log, toks, &program);
    sema_result_t *sr = sema_run(arena, log, program, src->path);
    if (!lok || !pok || !sr->ok) {
        LOG_E(log, PH_DRIVER, i18n_tr("build aborted: errors in source"));
        arena_free(arena); logger_free(log); return 1;
    }
    interp_options_t io = { stdout, stderr, NULL, opt->lang, 0 };
    int rc = interp_run(arena, log, program, sr, entry, &io);
    arena_free(arena); logger_free(log);
    return rc;
}

typedef struct {
    const langpack_t *pack;
    logger_t         *log;
    bool              check;
    fmt_style_t       style;
    int               changed;
    int               ok;
    int               errors;
    int               total;
} fmt_ctx_t;

static bool has_salam_ext(const char *name)
{
    size_t L = strlen(name);
    return L > 6 && strcmp(name + L - 6, ".salam") == 0;
}

static bool path_is_dir(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
#else
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static void fmt_one_file(fmt_ctx_t *c, const char *path)
{
    arena_t *a = arena_new(1 << 18);
    source_file_t *src = source_load(a, path);
    if (!src) {
        LOG_E(c->log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), path);
        c->errors++; arena_free(a); return;
    }
    c->total++;
    const langpack_t *pack = langpack_detect(a, src, c->pack);
    sb_t sb; sb_init(&sb);
    if (!fmt_source(a, c->log, pack, src, &c->style, &sb)) {
        LOG_E(c->log, PH_DRIVER,
              i18n_tr("cannot format '%s': fix the lexical errors first"), path);
        c->errors++; sb_free(&sb); arena_free(a); return;
    }
    bool changed = !(sb.len == src->len &&
                     memcmp(sb_cstr(&sb), src->text, src->len) == 0);
    if (!changed) {
        c->ok++;
        sb_free(&sb); arena_free(a); return;
    }
    c->changed++;
    if (c->check) {
        printf("%s\n", path);                 
    } else {
        FILE *f = fopen(path, "wb");
        if (!f) {
            LOG_E(c->log, PH_DRIVER, i18n_tr("salam: cannot write '%s': %s\n"),
                  path, strerror(errno));
            c->errors++;
        } else {
            fwrite(sb_cstr(&sb), 1, sb.len, f);
            fclose(f);
            printf(i18n_tr("formatted %s\n"), path);
        }
    }
    sb_free(&sb); arena_free(a);
}

static void fmt_walk(fmt_ctx_t *c, const char *dir)
{
#if defined(_WIN32)
    char pattern[1024];
    sal_snprintf(pattern, sizeof pattern, "%s/*", dir);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern, &fd);
    if (h == -1) return;
    do {
        if (fd.name[0] == '.') continue;
        char child[1024];
        sal_snprintf(child, sizeof child, "%s/%s", dir, fd.name);
        if (fd.attrib & _A_SUBDIR)        fmt_walk(c, child);
        else if (has_salam_ext(fd.name))  fmt_one_file(c, child);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;
        char child[1024];
        sal_snprintf(child, sizeof child, "%s/%s", dir, e->d_name);
        if (path_is_dir(child))             fmt_walk(c, child);
        else if (has_salam_ext(e->d_name))  fmt_one_file(c, child);
    }
    closedir(d);
#endif
}

static int driver_fmt(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        logger_free(log);
        return 2;
    }
    fmt_ctx_t c;
    memset(&c, 0, sizeof c);
    c.pack = pack; c.log = log; c.check = opt->fmt_check;
    c.style.tabs = opt->fmt_tabs;
    c.style.width = opt->fmt_indent_width;
    if (opt->input_count == 0) {
        fmt_walk(&c, ".");                    
    } else {
        { int i = 0; for (; i < opt->input_count; i++) {
            const char *p = opt->inputs[i];
            if (path_is_dir(p))  fmt_walk(&c, p);
            else                 fmt_one_file(&c, p);
        } }
    }
    (void)opt->fmt_recursive;   
    if (c.check) {
        if (c.changed == 0)
            printf(i18n_tr("all %d file(s) already formatted\n"), c.total);
        else
            printf(i18n_tr("%d of %d file(s) need formatting\n"), c.changed, c.total);
    } else {
        printf(i18n_tr("done: %d formatted, %d unchanged\n"), c.changed, c.ok);
    }
    int rc = 0;
    if (c.errors > 0)                 rc = 2;
    else if (c.check && c.changed > 0) rc = 1;
    langpack_free(pack);
    logger_free(log);
    return rc;
}

static void driver_print_version(void)
{
    printf("salam %s\n", SALAM_VERSION);
    printf("commit:  %s%s\n", SALAM_GIT_COMMIT, SALAM_GIT_DIRTY);
    printf("date:    %s\n", SALAM_GIT_DATE);
    printf("built:   %s\n", SALAM_BUILD_DATE);
}

int driver_main(int argc, char **argv)
{
    options_t opt;
    if (!cli_parse(argc, argv, &opt)) {
        return 2;
    }
    i18n_set_lang(opt.lang);
    salam_set_stdlib_root(opt.stdlib_path);
    layout_schema_init(salam_get_stdlib_root());
    
    switch (opt.command) {
    case CMD_HELP:
        cli_print_usage(stdout);
        return 0;
    case CMD_VERSION:
        driver_print_version();
        return 0;
    case CMD_NEW:
        return driver_new(&opt);
    case CMD_FMT:
        return driver_fmt(&opt);
    case CMD_RUN:
        return opt.interp ? driver_interp(&opt) : driver_run(&opt);
    case CMD_REPL:
    case CMD_REPL_LAYOUT:
        return driver_repl(&opt);
    case CMD_DEBUG:
        return driver_debug(&opt);
    case CMD_MEMCHECK:
        return driver_memcheck(&opt);
    case CMD_BUILD:
    case CMD_OBJ:
        if (opt.input_count == 0) {
            fprintf(stderr, i18n_tr("salam: '%s' requires at least one input file\n"),
                    opt.command == CMD_OBJ ? "obj" : "build");
            return 2;
        }
        return driver_build(&opt);
    case CMD_LLVM:
        if (opt.input_count == 0) {
            fprintf(stderr, "%s", i18n_tr("salam: 'llvm' requires an input file\n"));
            return 2;
        }
        return driver_llvm(&opt);
    case CMD_LAYOUT_BUILD:
        if (opt.input_count == 0) {
            fprintf(stderr, "%s", i18n_tr("salam: 'layout build' requires at least one input file\n"));
            return 2;
        }
        return driver_layout_build(&opt);
    case CMD_INSPECT:
        break;   
    }
    if (!opt.input) {
        fprintf(stderr, "%s", i18n_tr("salam: no input file\n\n"));
        cli_print_usage(stderr);
        return 2;
    }
    logger_t *log = logger_new(stderr, opt.log_level, resolve_color(opt.color));
    arena_t  *arena = arena_new(1 << 16);
    int rc = 0;
    langpack_t *pack = langpack_load(opt.lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt.lang);
        rc = 2;
        goto cleanup;
    }
    source_file_t *src = source_load(arena, opt.input);
    if (!src) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), opt.input);
        rc = 2;
        goto cleanup;
    }
    src = preproc_source(arena, log, src, opt.defines, opt.ndefines);
    
    logger_set_diag_source(log, src->text, src->len,
                           opt.diag_style, opt.diag_format);
    LOG_I(log, PH_DRIVER, "compiling %s (lang=%s, %zu bytes)",
          src->path, opt.lang, src->len);
    token_stream_t *toks = NULL;
    bool ok = lexer_run(arena, log, pack, src, &toks);
    LOG_I(log, PH_LEXER, "produced %zu tokens%s",
          token_stream_count(toks), ok ? "" : i18n_tr(" (with errors)"));
    if (opt.emit_tokens_xml) {
        int xrc = emit_tokens_xml(log, toks, opt.xml_out);
        if (xrc != 0) rc = xrc;
    }
    if (opt.emit_ast_xml || opt.emit_symbol_xml) {
        ast_node_t *program = NULL;
        bool pok = parser_run(arena, log, toks, &program);
        sema_result_t *sr = sema_run(arena, log, program, src->path);  
        if (opt.emit_ast_xml) {
            int xrc = emit_ast_xml(log, program, opt.xml_out);
            if (xrc != 0) rc = xrc;
        }
        if (opt.emit_symbol_xml) {
            int xrc = emit_symbol_xml(log, sr, opt.xml_out);
            if (xrc != 0) rc = xrc;
        }
        if (rc == 0 && (!pok || !sr->ok)) rc = 1;
    }
    if (rc == 0 && !ok) rc = 1;
cleanup:
    arena_free(arena);
    logger_free(log);
    return rc;
}

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
#include "driver/layout_build.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "diag/diag.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "layout/layout_gen.h"
#include "layout/layout_expand.h"
#include "condcomp/condcomp.h"
#include "i18n/i18n.h"

static const char *module_of(arena_t *a, const char *path)
{
    const char *base = path, *p;
    if ((p = strrchr(path, '/'))) base = p + 1;
    if ((p = strrchr(base, '\\'))) base = p + 1;
    const char *dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);
    return arena_strndup(a, base, len);
}

static const char *dir_of(arena_t *a, const char *path)
{
    const char *slash = NULL;
    {
        const char *p = path;
        for (; *p; p++)
            if (*p == '/' || *p == '\\') slash = p;
    }
    if (!slash) return "";
    return arena_strndup(a, path, (size_t)(slash - path));
}

static bool write_file(logger_t *log, const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), path);
        return false;
    }
    fputs(content, f);
    fclose(f);
    LOG_I(log, PH_DRIVER, i18n_tr("wrote %s"), path);
    return true;
}

static const char *path_ext(arena_t *a, const char *mod, const char *ext)
{
    size_t n = strlen(mod) + strlen(ext) + 2;
    char *p = (char *)arena_alloc(a, n);
    sal_snprintf(p, n, "%s.%s", mod, ext);
    return p;
}

static ast_node_t *find_layout(ast_node_t *program)
{
    {
        size_t i = 0;
        for (; i < program->list.len; i++) {
            ast_node_t *d = (ast_node_t *)program->list.data[i];
            if (d->kind == AST_LAYOUT_BLOCK) return d;
        }
    }
    return NULL;
}

int driver_layout_build(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, opt->color == 1);
    arena_t *arena = arena_new(1 << 20);
    diag_engine_t *diag = diag_new(arena, log, PH_CODEGEN);
    int rc = 0;
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, "unknown language pack '%s'", opt->lang);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    bool multi = (opt->input_count > 1);
    bool minify_ws = !opt->no_minify;
    const char *modules[SALAM_MAX_INPUTS];
    layout_result_t *results[SALAM_MAX_INPUTS];
    int n = 0;
    {
        int i = 0;
        for (; i < opt->input_count; i++) {
            const char *path = opt->inputs[i];
            source_file_t *src = source_load(arena, path);
            if (!src) {
                LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), path);
                rc = 2;
                continue;
            }
            LOG_I(log, PH_DRIVER, "layout build %s", path);
            const langpack_t *modpack = langpack_detect(arena, src, pack);
            token_stream_t *toks = NULL;
            lexer_run(arena, log, modpack, src, &toks);
            ast_node_t *program = NULL;
            parser_run(arena, log, toks, &program);
            cc_table_t *cc = cc_table_build(arena, NULL, opt->defines, opt->ndefines);
            cc_prune_program(arena, log, src->path, cc, program);

            const char *base_dir = dir_of(arena, src->path);
            if (layout_expand(arena, log, modpack, program, base_dir, cc)) rc = 1;
            sema_run(arena, log, program, src->path, langpack_code(modpack), cc);
            ast_node_t *lb = find_layout(program);
            if (!lb) {
                LOG_W(log, PH_DRIVER, i18n_tr("%s has no layout block"), path);
                continue;
            }
            modules[n] = module_of(arena, path);
            results[n] = layout_generate(arena, log, diag, src->path, lb, minify_ws);
            n++;
        }
    }
    if (diag->errors) rc = 1;

    sb_t mcss, mjs;
    sb_init(&mcss);
    sb_init(&mjs);
    {
        int i = 0;
        for (; i < n; i++) {
            const char *mod = modules[i];
            layout_result_t *r = results[i];
            const char *html_path =
                (opt->output && !multi) ? opt->output : path_ext(arena, mod, "html");
            if (opt->inline_mode) {
                write_file(log, html_path,
                           layout_document(arena, r, true, NULL, NULL, minify_ws));
            } else if (multi) {
                write_file(log, html_path,
                           layout_document(arena, r, false, "style.css", "script.js",
                                           minify_ws));
                if (r->css[0]) sb_puts(&mcss, r->css);
                if (r->js[0]) sb_puts(&mjs, r->js);
            } else {
                const char *css_href = r->css[0] ? path_ext(arena, mod, "css") : NULL;
                const char *js_href = r->js[0] ? path_ext(arena, mod, "js") : NULL;
                write_file(
                    log, html_path,
                    layout_document(arena, r, false, css_href, js_href, minify_ws));
                if (css_href) write_file(log, css_href, r->css);
                if (js_href) write_file(log, js_href, r->js);
            }
        }
    }
    if (multi && !opt->inline_mode) {
        if (mcss.len) write_file(log, "style.css", sb_cstr(&mcss));
        if (mjs.len) write_file(log, "script.js", sb_cstr(&mjs));
    }
    sb_free(&mcss);
    sb_free(&mjs);
    LOG_I(log, PH_DRIVER, "layout build complete (%zu error(s), %zu warning(s))",
          diag->errors, diag->warnings);
    logger_free(log);
    arena_free(arena);
    return rc;
}

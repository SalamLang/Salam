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
#include "driver/web_build.h"
#include "driver/js_build.h"
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

typedef struct {
    const char *paths[SALAM_MAX_INPUTS];
    int n;
} script_list_t;

static const char *module_of(arena_t *a, const char *path)
{
    const char *base = path, *p;
    if ((p = strrchr(path, '/'))) base = p + 1;
    if ((p = strrchr(base, '\\'))) base = p + 1;
    {
        const char *dot = strrchr(base, '.');
        size_t len = dot ? (size_t)(dot - base) : strlen(base);
        return arena_strndup(a, base, len);
    }
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

static const char *base_name(const char *p)
{
    const char *s = strrchr(p, '/');
    const char *b = strrchr(p, '\\');
    const char *cut = s;
    if (b && (!cut || b > cut)) cut = b;
    return cut ? cut + 1 : p;
}

static const char *strip_ext(arena_t *a, const char *p)
{
    const char *dot = strrchr(p, '.');
    const char *s = strrchr(p, '/');
    const char *b = strrchr(p, '\\');
    const char *cut = s;
    if (b && (!cut || b > cut)) cut = b;
    if (dot && (!cut || dot > cut)) return arena_strndup(a, p, (size_t)(dot - p));
    return p;
}

static const char *path_ext(arena_t *a, const char *stem, const char *ext)
{
    size_t n = strlen(stem) + strlen(ext) + 2;
    char *p = (char *)arena_alloc(a, n);
    sal_snprintf(p, n, "%s.%s", stem, ext);
    return p;
}

static const char *join_path(arena_t *a, const char *dir, const char *rel)
{
    size_t n;
    char *p;
    if (!dir || !dir[0]) return rel;
    n = strlen(dir) + strlen(rel) + 2;
    p = (char *)arena_alloc(a, n);
    sal_snprintf(p, n, "%s/%s", dir, rel);
    return p;
}

static bool ends_with(const char *s, const char *suf)
{
    size_t sl = strlen(s), fl = strlen(suf);
    return fl <= sl && strcmp(s + sl - fl, suf) == 0;
}

static bool write_file(logger_t *log, const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), path);
        return false;
    }
    if (fputs(content, f) == EOF) {
        LOG_E(log, PH_DRIVER, i18n_tr("failed to write '%s'"), path);
        fclose(f);
        return false;
    }
    if (fclose(f) != 0) {
        LOG_E(log, PH_DRIVER, i18n_tr("failed to write '%s'"), path);
        return false;
    }
    LOG_I(log, PH_DRIVER, "wrote %s", path);
    return true;
}

static void list_remove_at(vec_t *v, size_t i)
{
    if (i + 1 < v->len)
        memmove(&v->data[i], &v->data[i + 1], (v->len - i - 1) * sizeof(void *));
    v->len--;
}

static const char *script_salam_src(ast_node_t *el)
{
    size_t j = 0;
    for (; j < el->list.len; j++) {
        ast_node_t *attr = (ast_node_t *)el->list.data[j];
        if (attr->kind != AST_LAYOUT_ATTR || !attr->name) continue;
        if (strcmp(attr->name, "src") != 0) continue;
        if (attr->a && attr->a->kind == AST_LITERAL && attr->a->value.kind == TV_STRING &&
            attr->a->value.as.s && ends_with(attr->a->value.as.s, ".salam"))
            return attr->a->value.as.s;
    }
    return NULL;
}

static void collect_scripts(ast_node_t *node, script_list_t *out)
{
    size_t i = 0;
    if (!node) return;
    while (i < node->list.len) {
        ast_node_t *ch = (ast_node_t *)node->list.data[i];
        if (ch && ch->kind == AST_LAYOUT_ELEMENT) {
            if (ch->name && strcmp(ch->name, "script") == 0) {
                const char *src = script_salam_src(ch);
                if (src && out->n < SALAM_MAX_INPUTS) {
                    out->paths[out->n++] = src;
                    list_remove_at(&node->list, i);
                    continue;
                }
            }
            collect_scripts(ch, out);
        }
        i++;
    }
}

int driver_web(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, opt->color == 1);
    arena_t *arena = arena_new(1 << 20);
    diag_engine_t *diag = diag_new(arena, log, PH_CODEGEN);
    int rc = 0;
    langpack_t *pack = langpack_load(opt->lang);
    const char *path = opt->inputs[0];
    source_file_t *src;
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    salam_set_stdlib_root(opt->stdlib_path);
    src = source_load(arena, path);
    if (!src) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), path);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    LOG_I(log, PH_DRIVER, "web build %s", path);
    {
        const langpack_t *modpack = langpack_detect(arena, src, pack);
        token_stream_t *toks = NULL;
        ast_node_t *program = NULL;
        ast_node_t *lb = NULL;
        cc_table_t *cc = cc_table_build(arena, NULL, opt->defines, opt->ndefines);
        const char *base_dir = dir_of(arena, src->path);
        logger_set_diag_source(log, src->text, src->len, opt->diag_style,
                               opt->diag_format);
        lexer_run(arena, log, modpack, src, &toks);
        parser_run(arena, log, toks, &program);
        cc_prune_program(arena, log, src->path, cc, program);
        {
            size_t i = 0;
            for (; i < program->list.len; i++) {
                ast_node_t *d = (ast_node_t *)program->list.data[i];
                if (d->kind == AST_LAYOUT_BLOCK) {
                    lb = d;
                    break;
                }
            }
        }
        if (!lb) {
            static char outbuf[512];
            if (!opt->output) {
                sal_snprintf(outbuf, sizeof outbuf, "%s.html", module_of(arena, path));
                opt->output = outbuf;
            }
            logger_free(log);
            arena_free(arena);
            return driver_js(opt);
        }
        if (layout_expand(arena, log, modpack, program, base_dir, cc)) rc = 1;
        {
            vec_t pkg_cache;
            vec_init(&pkg_cache);
            sema_run_cached(arena, log, program, src->path, langpack_code(modpack), cc,
                            &pkg_cache);
            layout_localize_names(lb);
            {
                script_list_t scripts;
                sb_t bundles;
                layout_result_t *r = NULL;
                scripts.n = 0;
                collect_scripts(lb, &scripts);
                sb_init(&bundles);
                {
                    int i = 0;
                    for (; i < scripts.n && rc == 0; i++) {
                        const char *sp = join_path(arena, base_dir, scripts.paths[i]);
                        const char *entry1[1];
                        const char *bundle;
                        int brc = 0;
                        entry1[0] = sp;
                        LOG_I(log, PH_DRIVER, "compiling page script %s", sp);
                        bundle = js_build_bundle(arena, log, opt, entry1, 1, NULL, &brc,
                                                 &pkg_cache);
                        if (!bundle) {
                            rc = brc ? brc : 1;
                            break;
                        }
                        sb_puts(&bundles, bundle);
                        sb_putc(&bundles, '\n');
                    }
                }
                if (rc == 0) {
                    r = layout_generate(arena, log, diag, src->path, lb);
                    if (diag->errors) rc = 1;
                }
                if (rc == 0 && bundles.len) {
                    if (r->js && r->js[0]) {
                        size_t need = strlen(r->js) + bundles.len + 2;
                        char *m = (char *)arena_alloc(arena, need);
                        sal_snprintf(m, need, "%s\n%s", r->js, sb_cstr(&bundles));
                        r->js = m;
                    } else {
                        r->js = arena_strdup(arena, sb_cstr(&bundles));
                    }
                }
                if (rc == 0) {
                    const char *mod = module_of(arena, path);
                    const char *html_path =
                        opt->output ? opt->output : path_ext(arena, mod, "html");
                    if (opt->split) {
                        const char *stem = strip_ext(arena, html_path);
                        const char *css_path = path_ext(arena, stem, "css");
                        const char *js_path = path_ext(arena, stem, "js");
                        const char *css_href =
                            (r->css && r->css[0]) ? base_name(css_path) : NULL;
                        const char *js_href =
                            (r->js && r->js[0]) ? base_name(js_path) : NULL;
                        if (!write_file(
                                log, html_path,
                                layout_document(arena, r, false, css_href, js_href)))
                            rc = 2;
                        if (rc == 0 && css_href && !write_file(log, css_path, r->css))
                            rc = 2;
                        if (rc == 0 && js_href && !write_file(log, js_path, r->js))
                            rc = 2;
                    } else {
                        if (!write_file(log, html_path,
                                        layout_document(arena, r, true, NULL, NULL)))
                            rc = 2;
                    }
                    if (rc == 0)
                        LOG_I(log, PH_DRIVER,
                              "web build complete: %s (%d page script(s))", html_path,
                              scripts.n);
                }
                sb_free(&bundles);
            }
        }
    }
    logger_free(log);
    arena_free(arena);
    return rc;
}

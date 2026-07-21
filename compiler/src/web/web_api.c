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
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "condcomp/condcomp.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "interp/interp.h"
#include "layout/layout_gen.h"
#include "layout/layout_expand.h"
#include "layout/schema.h"
#include "diag/diag.h"
#include "i18n/i18n.h"
#include "xml/xml.h"
#include "token/token.h"
#include "codegen/codegen.h"
#include "llvm/codegen_llvm.h"
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#else
#  define EMSCRIPTEN_KEEPALIVE
#endif

static char *g_result = NULL;

static const char *set_result(const char *s)
{
    free(g_result);
    g_result = s ? strdup(s) : strdup("");
    return g_result;
}

static void ensure_schema(void)
{
    static bool ready = false;
    if (!ready) {
        layout_schema_init(NULL);
        ready = true;
    }
}

static char *slurp(FILE *f)
{
    if (fflush(f) != 0) {
    }
    long end = ftell(f);
    if (end < 0) end = 0;
    rewind(f);
    char *buf = (char *)malloc((size_t)end + 1);
    if (!buf) return NULL;
    size_t got = fread(buf, 1, (size_t)end, f);
    buf[got] = '\0';
    return buf;
}

static source_file_t *src_from_string(arena_t *a, const char *text)
{
    source_file_t *s = (source_file_t *)arena_alloc(a, sizeof *s);
    s->path = "main.salam";
    s->len = text ? strlen(text) : 0;
    s->text = arena_strndup(a, text ? text : "", s->len);
    return s;
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

static bool front_end(arena_t *arena, logger_t *log, FILE *diagf, const char *lang,
                      const char *source, ast_node_t **program, sema_result_t **sema,
                      const char **out_entry, char **out_diag)
{
    i18n_set_lang(lang);
    salam_set_stdlib_root(NULL);
    langpack_t *pack = langpack_load(lang);
    if (!pack) {
        *out_diag = strdup(i18n_tr("unknown language pack"));
        return false;
    }
    *out_entry = langpack_entry(pack);
    source_file_t *src = src_from_string(arena, source);
    logger_set_diag_source(log, src->text, src->len, DIAG_STYLE_RUST, DIAG_FORMAT_HUMAN);
    const langpack_t *modpack = pack;
    token_stream_t *toks = NULL;
    bool lok = lexer_run(arena, log, modpack, src, &toks);
    *program = NULL;
    bool pok = parser_run(arena, log, toks, program);
    cc_table_t *cc = cc_table_build(arena, NULL, NULL, 0);
    if (!cc_prune_program(arena, log, src->path, cc, *program)) pok = false;

    layout_expand(arena, log, modpack, *program, "", cc);
    *sema = sema_run(arena, log, *program, src->path, langpack_code(modpack), cc);
    if (!lok || !pok || !(*sema)->ok) {
        char *d = slurp(diagf);
        *out_diag = (d && d[0]) ? d : strdup(i18n_tr("compilation failed"));
        if (d != *out_diag) free(d);
        return false;
    }
    return true;
}

EMSCRIPTEN_KEEPALIVE
const char *salam_web_run_app(const char *source, const char *lang)
{
    arena_t *arena = arena_new(1 << 20);
    FILE *diagf = tmpfile();
    logger_t *log = logger_new(diagf ? diagf : stderr, LOG_WARN, false);
    ast_node_t *program;
    sema_result_t *sema;
    const char *entry;
    char *diag = NULL;
    if (!front_end(arena, log, diagf, lang, source, &program, &sema, &entry, &diag)) {
        const char *r = set_result(diag);
        free(diag);
        logger_free(log);
        if (diagf) fclose(diagf);
        arena_free(arena);
        return r;
    }
    FILE *outf = tmpfile();
    interp_options_t io = {outf ? outf : stdout, outf ? outf : stderr, NULL, lang, 3000};
    interp_run(arena, log, program, sema, entry, &io);
    char *out = outf ? slurp(outf) : strdup("");
    char *errs = diagf ? slurp(diagf) : strdup("");
    sb_t b;
    sb_init(&b);
    sb_puts(&b, out);
    if (errs && errs[0]) {
        if (out && out[0] && out[strlen(out) - 1] != '\n') sb_putc(&b, '\n');
        sb_puts(&b, errs);
    }
    const char *r = set_result(sb_cstr(&b));
    sb_free(&b);
    free(out);
    free(errs);
    logger_free(log);
    if (outf) fclose(outf);
    if (diagf) fclose(diagf);
    arena_free(arena);
    return r;
}

EMSCRIPTEN_KEEPALIVE
const char *salam_web_build_layout(const char *source, const char *lang)
{
    salam_set_stdlib_root(NULL);
    ensure_schema();
    arena_t *arena = arena_new(1 << 20);
    FILE *diagf = tmpfile();
    logger_t *log = logger_new(diagf ? diagf : stderr, LOG_WARN, false);
    ast_node_t *program;
    sema_result_t *sema;
    const char *entry;
    char *diag = NULL;
    if (!front_end(arena, log, diagf, lang, source, &program, &sema, &entry, &diag)) {
        const char *r = set_result(diag);
        free(diag);
        logger_free(log);
        if (diagf) fclose(diagf);
        arena_free(arena);
        return r;
    }
    ast_node_t *lb = find_layout(program);
    if (!lb) {
        const char *r =
            set_result(i18n_tr("this program has no layout block; "
                               "use a `layout:` block to build an HTML page"));
        logger_free(log);
        if (diagf) fclose(diagf);
        arena_free(arena);
        return r;
    }
    diag_engine_t *diag_e = diag_new(arena, log, PH_CODEGEN);
    layout_result_t *res = layout_generate(arena, log, diag_e, "main.salam", lb);
    const char *html = layout_document(arena, res, true, NULL, NULL);
    const char *r;
    if (diag_e->errors) {
        char *d = slurp(diagf);
        r = set_result((d && d[0]) ? d : i18n_tr("layout build failed"));
        free(d);
    } else
        r = set_result(html);
    logger_free(log);
    if (diagf) fclose(diagf);
    arena_free(arena);
    return r;
}

static const char *emit_xml_result(void (*serialize)(xml_writer_t *, const void *),
                                   const void *obj)
{
    sb_t b;
    sb_init(&b);
    xml_writer_t *w = xml_new(&b);
    xml_decl(w);
    serialize(w, obj);
    xml_free(w);
    const char *r = set_result(sb_cstr(&b));
    sb_free(&b);
    return r;
}

static void ser_tokens(xml_writer_t *w, const void *o)
{
    tokens_to_xml(w, (const token_stream_t *)o);
}

static void ser_ast(xml_writer_t *w, const void *o)
{
    ast_to_xml(w, (const ast_node_t *)o);
}

static void ser_symbols(xml_writer_t *w, const void *o)
{
    symbols_to_xml(w, (const sema_result_t *)o);
}

EMSCRIPTEN_KEEPALIVE
const char *salam_web_emit(const char *source, const char *lang, const char *phase)
{
    i18n_set_lang(lang);
    salam_set_stdlib_root(NULL);
    arena_t *arena = arena_new(1 << 20);
    FILE *diagf = tmpfile();
    logger_t *log = logger_new(diagf ? diagf : stderr, LOG_WARN, false);
    ast_node_t *program = NULL;
    sema_result_t *sema = NULL;
    const char *r = NULL;
    langpack_t *pack = langpack_load(lang);
    if (!pack) {
        r = set_result(i18n_tr("unknown language pack"));
        goto done;
    }
    const char *entry = langpack_entry(pack);
    source_file_t *src = src_from_string(arena, source);
    logger_set_diag_source(log, src->text, src->len, DIAG_STYLE_RUST, DIAG_FORMAT_HUMAN);
    const langpack_t *modpack = pack;
    token_stream_t *toks = NULL;
    bool lok = lexer_run(arena, log, modpack, src, &toks);
    if (!strcmp(phase, "tokens")) {
        r = emit_xml_result(ser_tokens, toks);
        goto done;
    }
    bool pok = parser_run(arena, log, toks, &program);
    cc_table_t *cc = cc_table_build(arena, NULL, NULL, 0);
    if (!cc_prune_program(arena, log, src->path, cc, program)) pok = false;
    if (!strcmp(phase, "ast")) {
        r = emit_xml_result(ser_ast, program);
        goto done;
    }
    ensure_schema();
    layout_expand(arena, log, modpack, program, "", cc);
    sema = sema_run(arena, log, program, src->path, langpack_code(modpack), cc);
    if (!strcmp(phase, "symbols")) {
        r = emit_xml_result(ser_symbols, sema);
        goto done;
    }

    if (!lok || !pok || !sema->ok) {
        char *d = slurp(diagf);
        r = set_result((d && d[0]) ? d : i18n_tr("compilation failed"));
        free(d);
        goto done;
    }
    if (!strcmp(phase, "html") || !strcmp(phase, "css") || !strcmp(phase, "js")) {
        ensure_schema();
        ast_node_t *lb = find_layout(program);
        if (!lb) {
            r = set_result(i18n_tr("this program has no layout block; "
                                   "use a `layout:` block to build an HTML page"));
            goto done;
        }
        diag_engine_t *diag_e = diag_new(arena, log, PH_CODEGEN);
        layout_result_t *res = layout_generate(arena, log, diag_e, "main.salam", lb);
        if (diag_e->errors) {
            char *d = slurp(diagf);
            r = set_result((d && d[0]) ? d : i18n_tr("layout build failed"));
            free(d);
            goto done;
        }
        const char *css_href = (res->css && res->css[0]) ? "style.css" : NULL;
        const char *js_href = (res->js && res->js[0]) ? "script.js" : NULL;
        const char *part = !strcmp(phase, "html")
                               ? layout_document(arena, res, false, css_href, js_href)
                           : !strcmp(phase, "css") ? res->css
                                                   : res->js;
        r = set_result(part ? part : "");
        goto done;
    }
    if (!strcmp(phase, "c")) {
        codegen_output_t *cg = codegen_run(arena, log, program, sema, "main", false,
                                           false, "main.salam", entry, NULL);
        sb_t b;
        sb_init(&b);
        if (cg && cg->h_src) {
            sb_puts(&b, "/* ===== salam_mod_main.h ===== */\n");
            sb_puts(&b, cg->h_src);
            sb_putc(&b, '\n');
        }
        if (cg && cg->c_src) {
            sb_puts(&b, "/* ===== salam_mod_main.c ===== */\n");
            sb_puts(&b, cg->c_src);
        }
        r = set_result(sb_cstr(&b));
        sb_free(&b);
        goto done;
    }
    if (!strcmp(phase, "llvm")) {
        llvm_output_t *ll = codegen_llvm_run(arena, log, program, sema, "main", entry);
        sb_t b;
        sb_init(&b);

        if (ll && !ll->ok) {
            sb_puts(&b, "; note: the LLVM backend supports a subset of Salam.\n; ");
            sb_puts(&b, ll->error ? ll->error : "unsupported construct");
            sb_puts(&b, "\n\n");
        }
        if (ll && ll->ll_src && ll->ll_src[0])
            sb_puts(&b, ll->ll_src);
        else if (!ll || ll->ok)
            sb_puts(&b, "; (no IR produced)\n");
        r = set_result(sb_cstr(&b));
        sb_free(&b);
        goto done;
    }
    r = set_result(i18n_tr("compilation failed"));
done:
    logger_free(log);
    if (diagf) fclose(diagf);
    arena_free(arena);
    return r;
}

EMSCRIPTEN_KEEPALIVE
const char *salam_web_version(void)
{
    return set_result("Salam WebAssembly playground");
}

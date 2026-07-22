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
#include "driver/js_build.h"
#include "core/arena.h"
#include "core/sb.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "jsgen/jsgen.h"
#include "condcomp/condcomp.h"
#include "i18n/i18n.h"
#include "minify/minify.h"

typedef struct {
    const char *name;
    const char *text;
} js_seg_t;

static const js_seg_t k_prelude[] = {
    {"salam_panic", "function salam_panic(message) {\n"
                    "    throw new Error(\"Salam panic: \" + String(message));\n"
                    "}\n"},
    {"__salam_unsupported",
     "function __salam_unsupported(what) {\n"
     "    throw new Error(\"not supported in the browser build: \" + what);\n"
     "}\n"},
    {"salam_input",
     "function salam_input() {\n"
     "    return typeof prompt === \"function\" ? String(prompt(\"\") || \"\") : "
     "\"\";\n"
     "}\n"},
    {"salam_strcat", "function salam_strcat(a, b) {\n"
                     "    return String(a) + String(b);\n"
                     "}\n"},
    {"salam_str_hash", "function salam_str_hash(s) {\n"
                       "    let h = 2166136261 >>> 0;\n"
                       "    for (let i = 0; i < s.length; i++) {\n"
                       "        h = Math.imul(h ^ s.charCodeAt(i), 16777619) >>> 0;\n"
                       "    }\n"
                       "    return h;\n"
                       "}\n"},
    {"salam_hash_int", "function salam_hash_int(v) {\n"
                       "    return Math.abs(Math.trunc(v)) >>> 0;\n"
                       "}\n"},
    {"sqrt", "const sqrt = Math.sqrt;\n"},
    {"cbrt", "const cbrt = Math.cbrt;\n"},
    {"sin", "const sin = Math.sin;\n"},
    {"cos", "const cos = Math.cos;\n"},
    {"tan", "const tan = Math.tan;\n"},
    {"asin", "const asin = Math.asin;\n"},
    {"acos", "const acos = Math.acos;\n"},
    {"atan", "const atan = Math.atan;\n"},
    {"atan2", "const atan2 = Math.atan2;\n"},
    {"exp", "const exp = Math.exp;\n"},
    {"log", "const log = Math.log;\n"},
    {"log2", "const log2 = Math.log2;\n"},
    {"log10", "const log10 = Math.log10;\n"},
    {"floor", "const floor = Math.floor;\n"},
    {"ceil", "const ceil = Math.ceil;\n"},
    {"round", "const round = Math.round;\n"},
    {"trunc", "const trunc = Math.trunc;\n"},
    {"hypot", "const hypot = Math.hypot;\n"},
    {"fabs", "const fabs = Math.abs;\n"},
    {"pow", "const pow = Math.pow;\n"},
    {"fmod", "function fmod(a, b) {\n    return a % b;\n}\n"},
    {"__sal_loop", "function __sal_loop(f) {\n"
                   "    let prev = performance.now();\n"
                   "    const step = function (t) {\n"
                   "        let dt = t - prev;\n"
                   "        prev = t;\n"
                   "        if (dt > 100) dt = 100;\n"
                   "        f(dt);\n"
                   "        requestAnimationFrame(step);\n"
                   "    };\n"
                   "    requestAnimationFrame(step);\n"
                   "}\n"},
    {"__sal_tone",
     "let __sal_audio = null;\n"
     "function __sal_tone(freq, ms) {\n"
     "    try {\n"
     "        if (!__sal_audio)\n"
     "            __sal_audio = new (window.AudioContext || "
     "window.webkitAudioContext)();\n"
     "        const t = __sal_audio.currentTime;\n"
     "        const osc = __sal_audio.createOscillator();\n"
     "        const gain = __sal_audio.createGain();\n"
     "        osc.type = \"square\";\n"
     "        osc.frequency.value = freq;\n"
     "        gain.gain.setValueAtTime(0.04, t);\n"
     "        gain.gain.exponentialRampToValueAtTime(0.001, t + ms / 1000);\n"
     "        osc.connect(gain);\n"
     "        gain.connect(__sal_audio.destination);\n"
     "        osc.start(t);\n"
     "        osc.stop(t + ms / 1000);\n"
     "    } catch (e) {\n"
     "        console.warn(\"__sal_tone: audio playback failed\", e);\n"
     "    }\n"
     "}\n"},
    {"__sal_img", "function __sal_img(src) {\n"
                  "    const img = new Image();\n"
                  "    img.src = src;\n"
                  "    return img;\n"
                  "}\n"},
    {"__sal_rrect", "function __sal_rrect(c, x, y, w, h, r) {\n"
                    "    if (c.roundRect) c.roundRect(x, y, w, h, r);\n"
                    "    else c.rect(x, y, w, h);\n"
                    "}\n"},
    {"__sal_lsget_i",
     "function __sal_lsget_i(k) {\n"
     "    try {\n"
     "        return Math.trunc(Number(localStorage.getItem(k) || "
     "0)) || 0;\n"
     "    } catch (e) {\n"
     "        console.warn(\"__sal_lsget_i: localStorage read failed\", e);\n"
     "        return 0;\n"
     "    }\n"
     "}\n"},
    {"__sal_lsget_s",
     "function __sal_lsget_s(k) {\n"
     "    try {\n"
     "        return String(localStorage.getItem(k) || \"\");\n"
     "    } catch (e) {\n"
     "        console.warn(\"__sal_lsget_s: localStorage read failed\", e);\n"
     "        return \"\";\n"
     "    }\n"
     "}\n"},
    {"__sal_lsset",
     "function __sal_lsset(k, v) {\n"
     "    try {\n"
     "        localStorage.setItem(k, String(v));\n"
     "    } catch (e) {\n"
     "        console.warn(\"__sal_lsset: localStorage write failed\", e);\n"
     "    }\n"
     "}\n"},
    {"__sal_touch", "function __sal_touch(ev, axis) {\n"
                    "    const t = (ev.changedTouches && ev.changedTouches[0]) || ev;\n"
                    "    return Number((axis ? t.clientY : t.clientX) || 0);\n"
                    "}\n"},
};

static const char *module_of(arena_t *a, const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *base = path;
    if (slash && slash + 1 > base) base = slash + 1;
    if (bslash && bslash + 1 > base) base = bslash + 1;
    {
        const char *dot = strrchr(base, '.');
        size_t len = dot ? (size_t)(dot - base) : strlen(base);
        return arena_strndup(a, base, len);
    }
}

static const char *dir_of(arena_t *a, const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bs = strrchr(path, '\\');
    const char *cut = slash;
    if (bs && (!slash || bs > slash)) cut = bs;
    if (!cut) return "";
    return arena_strndup(a, path, (size_t)(cut - path));
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
    LOG_I(log, PH_DRIVER, i18n_tr("wrote %s"), path);
    return true;
}

static bool has_suffix_ci(const char *s, const char *suf)
{
    size_t sl = strlen(s), fl = strlen(suf);
    if (fl > sl) return false;
    {
        size_t i = 0;
        for (; i < fl; i++)
            if (tolower((unsigned char)s[sl - fl + i]) != tolower((unsigned char)suf[i]))
                return false;
    }
    return true;
}

static bool js_refs(const char *text, const char *name)
{
    size_t nl = strlen(name);
    const char *p = text;
    if (!nl) return false;
    while ((p = strstr(p, name)) != NULL) {
        unsigned char before = (p == text) ? 0 : (unsigned char)p[-1];
        unsigned char after = (unsigned char)p[nl];
        bool bok = !(isalnum(before) || before == '_' || before == '$' || before == '.');
        bool aok = !(isalnum(after) || after == '_' || after == '$');
        if (bok && aok) return true;
        p += 1;
    }
    return false;
}

const char *js_build_bundle(arena_t *arena, logger_t *log, options_t *opt,
                            const char **entries, int nentries, const char **module_out,
                            int *rc_out, vec_t *pkg_cache)
{
    langpack_t *pack = langpack_load(opt->lang);
    *rc_out = 0;
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        *rc_out = 2;
        return NULL;
    }
    salam_set_stdlib_root(opt->stdlib_path);
    {
        const char *defs[SALAM_MAX_INPUTS];
        int ndefs = 0;
        const char *work[SALAM_MAX_INPUTS];
        int nwork = 0;
        jsgen_output_t *outputs[SALAM_MAX_INPUTS];
        int nout = 0;
        const char *first_module = NULL;
        const char *entry_name = NULL;
        bool all_ok = true;
        bool minify_ws = !opt->no_minify;
        bool minify_names = minify_ws && !opt->no_js_minify_names;
        const char *minify_last = NULL;
        vec_t minify_keys, minify_vals;
        vec_init(&minify_keys);
        vec_init(&minify_vals);
        {
            int i = 0;
            for (; i < opt->ndefines && ndefs < SALAM_MAX_INPUTS - 1; i++)
                defs[ndefs++] = opt->defines[i];
        }
        defs[ndefs++] = "SALAM_JS";
        {
            int i = 0;
            for (; i < nentries && nwork < SALAM_MAX_INPUTS; i++)
                work[nwork++] = entries[i];
        }
        {
            int wi = 0;
            for (; wi < nwork; wi++) {
                const char *path = work[wi];
                bool dup = false;
                source_file_t *src;
                {
                    int j = 0;
                    for (; j < wi; j++)
                        if (strcmp(work[j], path) == 0) dup = true;
                }
                if (dup) continue;
                src = source_load(arena, path);
                if (!src) {
                    LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), path);
                    all_ok = false;
                    continue;
                }
                {
                    cc_table_t *cc = cc_table_build(arena, NULL, defs, ndefs);
                    const langpack_t *modpack = langpack_detect(arena, src, pack);
                    const char *modentry = langpack_entry(modpack);
                    const char *module = module_of(arena, path);
                    token_stream_t *toks = NULL;
                    ast_node_t *program = NULL;
                    bool lok, pok;
                    logger_set_diag_source(log, src->text, src->len, opt->diag_style,
                                           opt->diag_format);
                    if (!first_module) first_module = module;
                    LOG_I(log, PH_DRIVER, i18n_tr("compiling %s -> js"), path);
                    lok = lexer_run(arena, log, modpack, src, &toks);
                    pok = parser_run(arena, log, toks, &program);
                    if (!cc_prune_program(arena, log, path, cc, program)) pok = false;
                    {
                        const char *pfiles[SALAM_MAX_INPUTS];
                        int npf =
                            salam_package_files(arena, path, pfiles, SALAM_MAX_INPUTS);
                        int pi = 1;
                        for (; pi < npf; pi++) {
                            source_file_t *psrc = source_load(arena, pfiles[pi]);
                            token_stream_t *ptoks = NULL;
                            ast_node_t *pprog = NULL;
                            if (!psrc) {
                                LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"),
                                      pfiles[pi]);
                                all_ok = false;
                                continue;
                            }
                            if (!lexer_run(arena, log, modpack, psrc, &ptoks))
                                lok = false;
                            if (!parser_run(arena, log, ptoks, &pprog)) pok = false;
                            if (!cc_prune_program(arena, log, pfiles[pi], cc, pprog))
                                pok = false;
                            salam_merge_program(arena, program, pprog);
                        }
                    }
                    {
                        sema_result_t *sr =
                            sema_run_cached(arena, log, program, src->path,
                                            langpack_code(modpack), cc, pkg_cache);
                        if (!lok || !pok || !sr->ok) {
                            all_ok = false;
                            continue;
                        }
                        {
                            const char *idir = dir_of(arena, path);
                            size_t k = 0;
                            for (; k < program->list.len && nwork < SALAM_MAX_INPUTS;
                                 k++) {
                                ast_node_t *d = (ast_node_t *)program->list.data[k];
                                const char *ipath;
                                if (d->kind != AST_IMPORT) continue;
                                ipath = d->type_str
                                            ? d->type_str
                                            : salam_resolve_import_node(
                                                  arena, idir, d, langpack_code(modpack));
                                if (!ipath) continue;
                                {
                                    bool known = false;
                                    int j = 0;
                                    for (; j < nwork; j++)
                                        if (strcmp(work[j], ipath) == 0) known = true;
                                    if (!known) work[nwork++] = ipath;
                                }
                            }
                        }
                        {
                            jsgen_output_t *out = jsgen_run(
                                arena, log, program, sr, module, modentry, minify_names,
                                minify_ws, &minify_last, &minify_keys, &minify_vals);
                            outputs[nout++] = out;
                            if (!entry_name && wi < nentries && out->entry_mangled)
                                entry_name = out->entry_mangled;
                        }
                    }
                }
            }
        }
        if (!all_ok) {
            LOG_E(log, PH_DRIVER, i18n_tr("build aborted: errors in source"));
            *rc_out = 1;
            return NULL;
        }
        if (!entry_name) {
            LOG_E(log, PH_DRIVER, i18n_tr("no entry point: define a '%s' function"),
                  langpack_entry(pack));
            *rc_out = 1;
            return NULL;
        }
        {
            vec_t seg_names, seg_texts, seg_used;
            sb_t roots;
            const char *boot;
            vec_init(&seg_names);
            vec_init(&seg_texts);
            vec_init(&seg_used);
            {
                size_t i = 0;
                for (; i < sizeof(k_prelude) / sizeof(k_prelude[0]); i++) {
                    const char *ptext = minify_ws ? minify_js(arena, k_prelude[i].text)
                                                  : k_prelude[i].text;
                    vec_push(arena, &seg_names, CONST_CAST(k_prelude[i].name));
                    vec_push(arena, &seg_texts, CONST_CAST(ptext));
                    vec_push(arena, &seg_used, NULL);
                }
            }
            {
                int m = nout - 1;
                for (; m >= 0; m--) {
                    jsgen_output_t *out = outputs[m];
                    size_t i = 0;
                    for (; i < out->fn_names.len; i++) {
                        vec_push(arena, &seg_names, out->fn_names.data[i]);
                        vec_push(arena, &seg_texts, out->fn_texts.data[i]);
                        vec_push(arena, &seg_used, NULL);
                    }
                }
            }
            boot = entry_name;
            sb_init(&roots);
            {
                int m = nout - 1;
                for (; m >= 0; m--)
                    sb_puts(&roots, outputs[m]->globals_src);
            }
            sb_puts(&roots, boot);
            sb_putc(&roots, '\n');
            {
                bool changed = true;
                while (changed) {
                    changed = false;
                    {
                        size_t s = 0;
                        for (; s < seg_names.len; s++) {
                            const char *nm;
                            if (seg_used.data[s]) continue;
                            nm = (const char *)seg_names.data[s];
                            if (js_refs(sb_cstr(&roots), nm)) {
                                seg_used.data[s] = (void *)1;
                                changed = true;
                                continue;
                            }
                            {
                                size_t t = 0;
                                for (; t < seg_names.len; t++) {
                                    if (t == s || !seg_used.data[t]) continue;
                                    if (js_refs((const char *)seg_texts.data[t], nm)) {
                                        seg_used.data[s] = (void *)1;
                                        changed = true;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            {
                sb_t js;
                bool have_globals = false;
                const char *text;
                sb_init(&js);
                sb_puts(&js, minify_ws ? "(function(){\"use strict\";"
                                       : "(function () {\n\"use strict\";\n\n");
                {
                    int m = nout - 1;
                    for (; m >= 0; m--)
                        if (outputs[m]->globals_src[0]) have_globals = true;
                }
                if (have_globals) {
                    int m = nout - 1;
                    for (; m >= 0; m--)
                        sb_puts(&js, outputs[m]->globals_src);
                    sb_putc(&js, minify_ws ? ' ' : '\n');
                }
                {
                    size_t s = 0;
                    for (; s < seg_names.len; s++) {
                        if (!seg_used.data[s]) continue;
                        sb_puts(&js, (const char *)seg_texts.data[s]);
                        sb_putc(&js, minify_ws ? ' ' : '\n');
                    }
                }
                if (!minify_ws)
                    sb_puts(&js, "// Run the program once the DOM is ready.\n");
                sb_printf(&js,
                          minify_ws
                              ? "if(document.readyState===\"loading\"){document."
                                "addEventListener(\"DOMContentLoaded\",%s);}else{%"
                                "s();}"
                              : "if (document.readyState === \"loading\") {\n"
                                "    document.addEventListener(\"DOMContentLoaded\", "
                                "%s);\n"
                                "} else {\n"
                                "    %s();\n"
                                "}\n",
                          boot, boot);
                sb_puts(&js, minify_ws ? "})();" : "})();\n");
                text = arena_strdup(arena, sb_cstr(&js));
                sb_free(&js);
                sb_free(&roots);
                if (module_out) *module_out = first_module;
                return text;
            }
        }
    }
    *rc_out = 1;
    return NULL;
}

int driver_js(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, opt->color == 1);
    arena_t *arena = arena_new(1 << 20);
    int rc = 0;
    const char *module = NULL;
    vec_t pkg_cache;
    vec_init(&pkg_cache);
    const char *js = js_build_bundle(arena, log, opt, opt->inputs, opt->input_count,
                                     &module, &rc, &pkg_cache);
    if (!js) {
        logger_free(log);
        arena_free(arena);
        return rc ? rc : 1;
    }
    {
        const char *output = opt->output;
        if (!output) {
            size_t ocap = strlen(module) + 4;
            char *o = (char *)arena_alloc(arena, ocap);
            sal_snprintf(o, ocap, "%s.js", module);
            output = o;
        }
        if (has_suffix_ci(output, ".html") || has_suffix_ci(output, ".htm")) {
            sb_t page;
            bool minify_ws = !opt->no_minify;
            sb_init(&page);
            if (minify_ws) {
                sb_puts(&page, "<!doctype html><html lang=\"en\"><head>");
                sb_puts(&page, "<meta charset=\"utf-8\">");
                sb_puts(&page, "<meta name=\"viewport\" content=\"width=device-width, "
                               "initial-scale=1\">");
                sb_printf(&page, "<title>%s</title>", module);
                sb_puts(&page, "</head><body><script>");
                sb_puts(&page, js);
                sb_puts(&page, "</script></body></html>");
            } else {
                sb_puts(&page, "<!doctype html>\n<html lang=\"en\">\n<head>\n");
                sb_puts(&page, "<meta charset=\"utf-8\">\n");
                sb_puts(&page, "<meta name=\"viewport\" "
                               "content=\"width=device-width, "
                               "initial-scale=1\">\n");
                sb_printf(&page, "<title>%s</title>\n", module);
                sb_puts(&page, "</head>\n<body>\n<script>\n");
                sb_puts(&page, js);
                sb_puts(&page, "</script>\n</body>\n</html>\n");
            }
            if (!write_file(log, output, sb_cstr(&page))) rc = 2;
            sb_free(&page);
            if (rc == 0)
                LOG_I(log, PH_DRIVER, i18n_tr("built self-contained page: %s"), output);
        } else {
            if (!write_file(log, output, js)) rc = 2;
            if (rc == 0) LOG_I(log, PH_DRIVER, i18n_tr("built JavaScript: %s"), output);
        }
    }
    logger_free(log);
    arena_free(arena);
    return rc;
}

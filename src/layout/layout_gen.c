#include "core/prelude.h"
#include "layout/layout_gen.h"
#include "layout/layout_internal.h"
#include "layout/registry.h"
#include "core/sb.h"
#include "core/vec.h"
#include "i18n/i18n.h"

void layout_localize_names(ast_node_t *node)
{
    if (!node) return;
    if (node->kind == AST_LAYOUT_ELEMENT || node->kind == AST_LAYOUT_ATTR)
        node->name = i18n_layout_word(node->name);
    for (size_t i = 0; i < node->list.len; i++)
        layout_localize_names((ast_node_t *)node->list.data[i]);
}
layout_result_t *layout_generate(arena_t *a, logger_t *log, diag_engine_t *diag,
                                 const char *file, ast_node_t *layout_block)
{
    layout_ctx_t cx; memset(&cx, 0, sizeof(cx));
    cx.a = a; cx.log = log; cx.diag = diag; cx.file = file;
    cx.title = "Salam Page"; cx.lang = "en"; cx.dir = "ltr";
    vec_init(&cx.css_seen);
    sb_t html, css, js; sb_init(&html); sb_init(&css); sb_init(&js);
    cx.html = &html; cx.css = &css; cx.js = &js; cx.indent = 0;
    layout_localize_names(layout_block);   
    LOG_I(log, PH_CODEGEN, "generating layout output");
    
    long body_repeat = 1;
    for (size_t i = 0; i < layout_block->list.len; i++) {
        ast_node_t *item = (ast_node_t *)layout_block->list.data[i];
        if (item->kind != AST_LAYOUT_ATTR) continue;
        const char *v = val_str(&cx, item->a);
        if (!strcmp(item->name,"title"))     cx.title = v;
        else if (!strcmp(item->name,"lang")) { const char *l = layout_map_lang(i18n_layout_value(v)); if (l) cx.lang = l; }
        else if (!strcmp(item->name,"dir"))  { const char *d = layout_map_dir(i18n_layout_value(v)); if (d) cx.dir = d; }
        else if (!strcmp(item->name,"repeat")) {
            if (item->a && item->a->kind == AST_LITERAL && item->a->value.kind == TV_INT)
                body_repeat = (long)item->a->value.as.i < 1 ? 1 : (long)item->a->value.as.i;
        }
    }
    
    for (long r = 0; r < body_repeat; r++) {
        for (size_t i = 0; i < layout_block->list.len; i++) {
            ast_node_t *item = (ast_node_t *)layout_block->list.data[i];
            if (item->kind == AST_LAYOUT_ATTR) {
                
                if (!strcmp(item->name,"content"))
                    html_line(&cx, "%s", html_escape(&cx, val_str(&cx, item->a)));
                else if (strcmp(item->name,"title") && strcmp(item->name,"lang") &&
                         strcmp(item->name,"dir") && strcmp(item->name,"repeat"))
                    LOG_T(log, PH_CODEGEN, "ignoring document attribute '%s'", item->name);
            } else if (item->kind == AST_LAYOUT_ELEMENT) {
                gen_element_n(&cx, item, "layout", NULL);
            }
        }
    }
    layout_result_t *r = (layout_result_t *)arena_alloc(a, sizeof(*r));
    r->html  = arena_strdup(a, sb_cstr(&html));
    r->css   = arena_strdup(a, sb_cstr(&css));
    r->js    = arena_strdup(a, sb_cstr(&js));
    r->title = cx.title; r->lang = cx.lang; r->dir = cx.dir;
    sb_free(&html); sb_free(&css); sb_free(&js);
    LOG_I(log, PH_CODEGEN, "layout: %zu bytes HTML, %zu bytes CSS, %zu bytes JS",
          strlen(r->html), strlen(r->css), strlen(r->js));
    return r;
}

const char *layout_document(arena_t *a, const layout_result_t *r, bool inl,
                            const char *css_href, const char *js_href)
{
    sb_t d; sb_init(&d);
    sb_puts(&d, "<!doctype html>\n");
    sb_puts(&d, "<html lang=\""); sb_puts(&d, r->lang);
    sb_puts(&d, "\" dir=\""); sb_puts(&d, r->dir); sb_puts(&d, "\">\n");
    sb_puts(&d, "<head>\n");
    sb_puts(&d, "  <meta charset=\"utf-8\">\n");
    sb_puts(&d, "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
    sb_puts(&d, "  <title>"); sb_puts(&d, r->title); sb_puts(&d, "</title>\n");
    if (inl) {
        if (r->css && r->css[0]) {
            sb_puts(&d, "  <style>\n"); sb_puts(&d, r->css); sb_puts(&d, "  </style>\n");
        }
    } else if (css_href) {
        sb_puts(&d, "  <link rel=\"stylesheet\" href=\""); sb_puts(&d, css_href); sb_puts(&d, "\">\n");
    }
    sb_puts(&d, "</head>\n<body>\n");
    sb_puts(&d, r->html);
    if (inl) {
        if (r->js && r->js[0]) {
            sb_puts(&d, "  <script>\n"); sb_puts(&d, r->js); sb_puts(&d, "  </script>\n");
        }
    } else if (js_href) {
        sb_puts(&d, "  <script src=\""); sb_puts(&d, js_href); sb_puts(&d, "\"></script>\n");
    }
    sb_puts(&d, "</body>\n</html>\n");
    const char *out = arena_strdup(a, sb_cstr(&d));
    sb_free(&d);
    return out;
}

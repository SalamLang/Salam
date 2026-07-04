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
#include "layout/layout_internal.h"
#include "layout/registry.h"

static void gen_element(layout_ctx_t *cx, ast_node_t *el, const char *parent,
                        const char *parent_class);
static long repeat_count(ast_node_t *el)
{
    { size_t i = 0; for (; i < el->list.len; i++) {
        ast_node_t *a = (ast_node_t *)el->list.data[i];
        if (a->kind == AST_LAYOUT_ATTR && a->name && !strcmp(a->name, "repeat") &&
            a->a && a->a->kind == AST_LITERAL && a->a->value.kind == TV_INT)
            return (long)a->a->value.as.i < 1 ? 1 : (long)a->a->value.as.i;
    } }
    return 1;
}

void gen_element_n(layout_ctx_t *cx, ast_node_t *el, const char *parent, const char *parent_class)
{
    long n = repeat_count(el);
    { long i = 0; for (; i < n; i++) gen_element(cx, el, parent, parent_class); }
}

static void gen_children(layout_ctx_t *cx, ast_node_t *el, const char *klass)
{
    { size_t i = 0; for (; i < el->list.len; i++) {
        ast_node_t *c = (ast_node_t *)el->list.data[i];
        if (c->kind == AST_LAYOUT_ELEMENT) gen_element_n(cx, c, el->name, klass);
    } }
}

static bool has_child_elements(ast_node_t *el)
{
    { size_t i = 0; for (; i < el->list.len; i++)
        if (((ast_node_t *)el->list.data[i])->kind == AST_LAYOUT_ELEMENT) return true; }
    return false;
}

static void gen_element(layout_ctx_t *cx, ast_node_t *el, const char *parent, const char *parent_class)
{
    (void)parent;
    const layout_elem_def_t *def = layout_elem_lookup(el->name);
    if (!def) LWARN(cx, 2, &el->span, "unrecognized layout element '%s' (rendered as <div>)", el->name);
    layout_elem_type_t type = def ? def->type : LE_MOTHER;
    const char *tag = (def && def->tag) ? def->tag : "div";
    
    if (!strcmp(el->name, "heading")) {
        long sz = 1;
        { size_t i = 0; for (; i < el->list.len; i++) {
            ast_node_t *attr = (ast_node_t *)el->list.data[i];
            if (attr->kind == AST_LAYOUT_ATTR && attr->name && !strcmp(attr->name, "size") &&
                attr->a && attr->a->kind == AST_LITERAL && attr->a->value.kind == TV_INT)
                sz = (long)attr->a->value.as.i;
        } }
        if (sz < 1) sz = 1;
        if (sz > 6) sz = 6;
        tag = lfmt(cx, "h%ld", sz);
    }
    LOG_T(cx->log, PH_CODEGEN, "layout <%s> -> <%s>", el->name, tag);
    sb_t attrs, css, hover, focus, active, before, after;
    sb_init(&attrs); sb_init(&css); sb_init(&hover); sb_init(&focus);
    sb_init(&active); sb_init(&before); sb_init(&after);
    const char *content = NULL, *userclass = NULL, *source = NULL;
    const char *src = NULL, *type_val = NULL, *condition = NULL;
    { size_t i = 0; for (; i < el->list.len; i++) {
        ast_node_t *attr = (ast_node_t *)el->list.data[i];
        if (attr->kind != AST_LAYOUT_ATTR) continue;
        const char *nm = attr->name;
        const char *v = val_str(cx, attr->a);
        if (!strcmp(nm, "condition")) { condition = v; continue; }
        if (starts_with(nm,"hover_"))  { sb_puts(&hover,  lfmt(cx,"%s: %s; ", hyphenate(cx,nm+6), v)); continue; }
        if (starts_with(nm,"focus_"))  { sb_puts(&focus,  lfmt(cx,"%s: %s; ", hyphenate(cx,nm+6), v)); continue; }
        if (starts_with(nm,"active_")) { sb_puts(&active, lfmt(cx,"%s: %s; ", hyphenate(cx,nm+7), v)); continue; }
        if (starts_with(nm,"before_")) {
            const char *rest = nm + 7;
            if (!strcmp(rest,"content")) sb_puts(&before, lfmt(cx,"content: \"%s\"; ", v));
            else sb_puts(&before, lfmt(cx,"%s: %s; ", hyphenate(cx,rest), v));
            continue;
        }
        if (starts_with(nm,"after_")) {
            const char *rest = nm + 6;
            if (!strcmp(rest,"content")) sb_puts(&after, lfmt(cx,"content: \"%s\"; ", v));
            else sb_puts(&after, lfmt(cx,"%s: %s; ", hyphenate(cx,rest), v));
            continue;
        }
        if (nm[0]=='o' && nm[1]=='n') { sb_puts(&attrs, lfmt(cx," %s=\"%s\"", nm, html_escape(cx, v))); continue; }
        const layout_attr_def_t *ad = layout_attr_lookup(nm);
        layout_attr_dest_t dest = ad ? ad->dest : LA_CSS;
        switch (dest) {
            case LA_CONTENT: content = v; break;
            case LA_CLASS:   userclass = v; break;
            case LA_STYLE:   sb_puts(&attrs, lfmt(cx," style=\"%s\"", html_escape(cx, v))); break;
            case LA_DIR:  sb_puts(&attrs, lfmt(cx," dir=\"%s\"",  layout_attr_value_map(ad, v))); break;
            case LA_LANG: sb_puts(&attrs, lfmt(cx," lang=\"%s\"", layout_attr_value_map(ad, v))); break;
            case LA_SOURCE:  source = v; break;
            case LA_HTML:
                if (!strcmp(nm,"src"))       src = v;
                else if (!strcmp(nm,"type"))      type_val = layout_attr_value_map(ad, v);
                else if (!strcmp(nm,"condition")) condition = v;
                else if (is_bool_attr(nm) && !strcmp(v,"true")) sb_puts(&attrs, lfmt(cx," %s", ad->out));
                
                else sb_puts(&attrs, lfmt(cx," %s=\"%s\"", ad->out, html_escape(cx, layout_attr_value_map(ad, v))));
                break;
            case LA_CSS: {
                
                const char *prop = (ad && ad->out) ? ad->out : hyphenate(cx, nm);
                
                sb_puts(&css, lfmt(cx,"%s: %s; ", prop, layout_attr_value_map(ad, v)));
                break;
            }
            case LA_META: break;   
        }
    } }
    bool has_css = css.len > 0;
    bool needs_class = has_css || hover.len || focus.len || active.len || before.len || after.len;
    const char *klass = userclass;
    if (needs_class && !klass) {
        sb_t k; sb_init(&k);
        sb_puts(&k, sb_cstr(&css));    sb_putc(&k, '\x1f'); sb_puts(&k, sb_cstr(&hover));
        sb_putc(&k, '\x1f'); sb_puts(&k, sb_cstr(&focus));  sb_putc(&k, '\x1f'); sb_puts(&k, sb_cstr(&active));
        sb_putc(&k, '\x1f'); sb_puts(&k, sb_cstr(&before)); sb_putc(&k, '\x1f'); sb_puts(&k, sb_cstr(&after));
        klass = class_for_key(cx, sb_cstr(&k));
        sb_free(&k);
    }
    
    if (!strcmp(el->name, "media")) {
        const char *sel = parent_class ? parent_class : "body";
        const char *cond = condition ? condition : "all";
        emit_rule(cx, lfmt(cx, "@media %s { .%s { %s } }", cond, sel, sb_cstr(&css)));
        goto done;
    }
    if (!strcmp(el->name, "style")) {
        if (content) { sb_puts(cx->css, content); sb_putc(cx->css, '\n'); }
        goto done;
    }
    if (!strcmp(el->name, "script")) {
        if (src) html_line(cx, "<script src=\"%s\"></script>", html_escape(cx, src));
        else if (content) { sb_puts(cx->js, content); sb_putc(cx->js, '\n'); }
        goto done;
    }
    
    if (has_css)    emit_rule(cx, lfmt(cx, ".%s { %s }", klass, sb_cstr(&css)));
    if (hover.len)  emit_rule(cx, lfmt(cx, ".%s:hover { %s }",  klass, sb_cstr(&hover)));
    if (focus.len)  emit_rule(cx, lfmt(cx, ".%s:focus { %s }",  klass, sb_cstr(&focus)));
    if (active.len) emit_rule(cx, lfmt(cx, ".%s:active { %s }", klass, sb_cstr(&active)));
    if (before.len) emit_rule(cx, lfmt(cx, ".%s::before { %s }", klass, sb_cstr(&before)));
    if (after.len)  emit_rule(cx, lfmt(cx, ".%s::after { %s }",  klass, sb_cstr(&after)));
    const char *classattr = klass ? lfmt(cx, " class=\"%s\"", klass) : "";
    const char *A = sb_cstr(&attrs);
    const char *esc = content ? html_escape(cx, content) : "";
    
    if (!strcmp(el->name, "link")) {
        html_line(cx, "<a href=\"%s\"%s%s>%s</a>", source ? html_escape(cx, source) : "#", classattr, A, esc);
        goto done;
    }
    if (!strcmp(el->name, "image")) {
        html_line(cx, "<img src=\"%s\"%s%s alt=\"%s\" />", source ? html_escape(cx, source) : "", classattr, A, esc);
        goto done;
    }
    if (!strcmp(el->name, "input")) {
        if (type_val && !strcmp(type_val, "textarea"))
            html_line(cx, "<textarea%s%s>%s</textarea>", classattr, A, esc);
        else
            html_line(cx, "<input%s%s%s />", type_val ? lfmt(cx," type=\"%s\"", type_val) : "", classattr, A);
        goto done;
    }
    
    if (type == LE_SINGLE) {
        html_line(cx, "<%s%s%s />", tag, classattr, A);
        goto done;
    }
    if (has_child_elements(el)) {
        html_line(cx, "<%s%s%s>", tag, classattr, A);
        cx->indent++;
        if (content) html_line(cx, "%s", esc);
        gen_children(cx, el, klass);
        cx->indent--;
        html_line(cx, "</%s>", tag);
    } else {
        html_line(cx, "<%s%s%s>%s</%s>", tag, classattr, A, esc, tag);
    }
done:
    sb_free(&attrs); sb_free(&css); sb_free(&hover); sb_free(&focus);
    sb_free(&active); sb_free(&before); sb_free(&after);
}

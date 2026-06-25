#include "core/prelude.h"
#include "semantic/sema_internal.h"
#include "layout/registry.h"
#include "layout/layout_gen.h"
#include "i18n/i18n.h"

static bool csv_contains(const char *csv, const char *name)
{
    if (!csv || !name) return false;
    size_t nl = strlen(name);
    const char *p = csv;
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t len = comma ? (size_t)(comma - p) : strlen(p);
        if (len == nl && strncmp(p, name, nl) == 0) return true;
        if (!comma) break;
        p = comma + 1;
    }
    return false;
}

static const char *attr_string(ast_node_t *attr)
{
    ast_node_t *v = attr->a;
    if (v && v->kind == AST_LITERAL && v->value.kind == TV_STRING) return v->value.as.s;
    return NULL;
}

static ast_node_t *elem_attr(ast_node_t *el, const char *name)
{
    { size_t i = 0; for (; i < el->list.len; i++) {
        ast_node_t *c = (ast_node_t *)el->list.data[i];
        if (c->kind == AST_LAYOUT_ATTR && c->name && !strcmp(c->name, name)) return c;
    } }
    return NULL;
}

static bool subtree_has_id(ast_node_t *el)
{
    { size_t i = 0; for (; i < el->list.len; i++) {
        ast_node_t *c = (ast_node_t *)el->list.data[i];
        if (c->kind != AST_LAYOUT_ELEMENT) continue;
        if (elem_attr(c, "id") || subtree_has_id(c)) return true;
    } }
    return false;
}

static void check_repeat(sema_t *s, ast_node_t *element)
{
    if (!elem_attr(element, "repeat")) return;
    if (elem_attr(element, "id"))
        SERR(s, 19, &element->span,
             "element '%s' cannot use 'repeat' together with 'id'", element->name);
    else if (subtree_has_id(element))
        SERR(s, 19, &element->span,
             "element '%s' has 'repeat' but a nested element has an 'id'", element->name);
}

static void check_attr(sema_t *s, ast_node_t *attr)
{
    const char *name = attr->name;
    if (!name) return;
    const layout_attr_def_t *ad = layout_attr_lookup(name);
    if (!ad) return;                       
    const char *v = attr_string(attr);
    if (!v) return;                        
    if (!layout_attr_value_ok(ad, v))
        SERR(s, 18, &attr->span, "invalid value for '%s': '%s' is not %s",
             name, v, layout_value_type_name(ad->vtype));
}

static void check_required(sema_t *s, ast_node_t *element, const layout_elem_def_t *reg)
{
    const layout_elem_def_t *def = (const layout_elem_def_t *)reg;
    if (!def || !def->required) return;
    const char *p = def->required;
    while (*p) {
        const char *comma = strchr(p, ',');
        size_t len = comma ? (size_t)(comma - p) : strlen(p);
        bool found = false;
        { size_t i = 0; for (; i < element->list.len; i++) {
            ast_node_t *c = (ast_node_t *)element->list.data[i];
            if (c->kind == AST_LAYOUT_ATTR && c->name &&
                strlen(c->name) == len && strncmp(c->name, p, len) == 0) { found = true; break; }
        } }
        if (!found)
            SERR(s, 5, &element->span, "element '%s' is missing required attribute '%.*s'",
                 element->name, (int)len, p);
        if (!comma) break;
        p = comma + 1;
    }
}

void sema_check_layout(sema_t *s, ast_node_t *node, const char *parent)
{
    if (node->kind == AST_LAYOUT_BLOCK) {
        layout_localize_names(node);   
        LOG_D(s->log, PH_SEMANTIC, "validating layout block");
        
        if (elem_attr(node, "repeat") && subtree_has_id(node))
            SERR(s, 19, &node->span,
                 "layout uses 'repeat' but a nested element has an 'id'");
        { size_t i = 0; for (; i < node->list.len; i++) {
            ast_node_t *c = (ast_node_t *)node->list.data[i];
            if (c->kind == AST_LAYOUT_ELEMENT) sema_check_layout(s, c, "layout");
            else if (c->kind == AST_LAYOUT_ATTR) check_attr(s, c);
        } }
        return;
    }
    if (node->kind == AST_LAYOUT_ELEMENT) {
        const layout_elem_def_t *reg = layout_elem_lookup(node->name);
        if (!reg) {
            SWARN(s, 2, &node->span, "unrecognized layout element '%s' (ignored)", node->name);
        } else {
            if (reg->parents && !csv_contains(reg->parents, parent ? parent : ""))
                SERR(s, 4, &node->span, "element '%s' cannot be placed inside '%s'",
                     node->name, parent ? parent : "<root>");
            check_required(s, node, reg);
        }
        check_repeat(s, node);
        { size_t i = 0; for (; i < node->list.len; i++) {
            ast_node_t *c = (ast_node_t *)node->list.data[i];
            if (c->kind == AST_LAYOUT_ELEMENT) sema_check_layout(s, c, node->name);
            else if (c->kind == AST_LAYOUT_ATTR) check_attr(s, c);
        } }
        return;
    }
}

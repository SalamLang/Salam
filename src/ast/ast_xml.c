#include "core/prelude.h"
#include "ast/ast.h"

static void ast_emit_value(xml_writer_t *w, const token_value_t *v)
{
    char buf[64];
    switch (v->kind) {
        case TV_NONE: return;
        case TV_INT:
            xml_open(w, "value"); xml_attr(w, "kind", "int");
            snprintf(buf, sizeof(buf), "%llu", (unsigned long long)v->as.i);
            xml_text(w, buf); xml_close(w); break;
        case TV_FLOAT:
            xml_open(w, "value"); xml_attr(w, "kind", "float");
            snprintf(buf, sizeof(buf), "%g", v->as.f);
            xml_text(w, buf); xml_close(w); break;
        case TV_STRING:
            xml_open(w, "value"); xml_attr(w, "kind", "string");
            xml_text(w, v->as.s ? v->as.s : ""); xml_close(w); break;
        case TV_CHAR:
            xml_open(w, "value"); xml_attr(w, "kind", "char");
            snprintf(buf, sizeof(buf), "%llu", (unsigned long long)v->as.i);
            xml_text(w, buf); xml_close(w); break;
        case TV_BOOL:
            xml_open(w, "value"); xml_attr(w, "kind", "bool");
            xml_text(w, v->as.b ? "true" : "false"); xml_close(w); break;
    }
}

static void ast_loc(xml_writer_t *w, const ast_node_t *n)
{
    xml_open(w, "location");
        xml_leaf_int(w, "begin_line",   n->span.begin.line);
        xml_leaf_int(w, "begin_column", n->span.begin.col);
        xml_leaf_int(w, "begin_index",  n->span.begin.index);
        xml_leaf_int(w, "end_line",     n->span.end.line);
        xml_leaf_int(w, "end_column",   n->span.end.col);
    xml_close(w);
}

static void ast_child(xml_writer_t *w, const char *role, const ast_node_t *n)
{
    if (!n) return;
    xml_open(w, role);
        ast_to_xml(w, n);
    xml_close(w);
}

static void ast_child_list(xml_writer_t *w, const char *role, const vec_t *list)
{
    xml_open(w, role);
    for (size_t i = 0; i < list->len; i++) {
        ast_to_xml(w, (const ast_node_t *)list->data[i]);
    }
    xml_close(w);
}

static bool ast_kind_has_op(ast_kind_t k)
{
    return k == AST_BINARY || k == AST_UNARY || k == AST_ASSIGN || k == AST_LITERAL;
}

void ast_to_xml(xml_writer_t *w, const ast_node_t *n)
{
    if (!n) return;
    xml_open(w, "node");
    xml_attr(w, "kind", ast_kind_name(n->kind));
    if (n->name) xml_attr(w, "name", n->name);
    if (n->type_str) xml_attr(w, "type", n->type_str);   
    if (ast_kind_has_op(n->kind)) xml_attr(w, "op", token_kind_name(n->op));
    if (n->kind == AST_VAR_DECL) xml_attr(w, "mut", n->is_mut ? "true" : "false");
    if (n->kind == AST_TYPE && n->is_pointer) xml_attr(w, "pointer", "true");
    if (n->is_extern)   xml_attr(w, "extern", "true");
    if (n->is_variadic) xml_attr(w, "variadic", "true");
    if (n->is_pub)      xml_attr(w, "pub", "true");
    ast_loc(w, n);
    if (n->value.kind != TV_NONE) ast_emit_value(w, &n->value);
    switch (n->kind) {
        case AST_PROGRAM:       ast_child_list(w, "defs", &n->list); break;
        case AST_TYPE_ALIAS:    ast_child(w, "type", n->type); break;
        case AST_CONST_DECL:    ast_child(w, "type", n->type); ast_child(w, "value", n->a); break;
        case AST_FUNC_DEF:
            ast_child(w, "return_type", n->type);
            ast_child_list(w, "params", &n->list);
            ast_child(w, "body", n->a);
            break;
        case AST_PARAM:         ast_child(w, "type", n->type); ast_child(w, "default", n->a); break;
        case AST_STRUCT_DEF:    ast_child_list(w, "members", &n->list); break;
        case AST_INTERFACE_DEF: ast_child_list(w, "methods", &n->list); break;
        case AST_FIELD:         ast_child(w, "type", n->type); ast_child(w, "default", n->a); break;
        case AST_ENUM_DEF:      ast_child_list(w, "members", &n->list); break;
        case AST_ENUM_MEMBER:   ast_child(w, "value", n->a); break;
        case AST_BLOCK:         ast_child_list(w, "statements", &n->list); break;
        case AST_VAR_DECL:      ast_child(w, "type", n->type); ast_child(w, "init", n->a); break;
        case AST_ASSIGN:        ast_child(w, "target", n->a); ast_child(w, "value", n->b); break;
        case AST_IF:
            ast_child(w, "condition", n->a);
            ast_child(w, "then", n->b);
            ast_child(w, "else", n->c);
            break;
        case AST_WHILE:         ast_child(w, "condition", n->a); ast_child(w, "body", n->b); break;
        case AST_REPEAT:        ast_child(w, n->c ? "from" : "count", n->a);
                                ast_child(w, "to", n->c); ast_child(w, "step", n->d);
                                ast_child(w, "body", n->b); break;
        case AST_FOR:
            ast_child(w, "init", n->a);
            ast_child(w, "condition", n->b);
            ast_child(w, "post", n->c);
            ast_child(w, "body", n->d);
            break;
        case AST_RETURN:        ast_child(w, "value", n->a); break;
        case AST_DEFER:         ast_child(w, "statement", n->a); break;
        case AST_EXPR_STMT:     ast_child(w, "expr", n->a); break;
        case AST_BINARY:        ast_child(w, "left", n->a); ast_child(w, "right", n->b); break;
        case AST_UNARY:         ast_child(w, "operand", n->a); break;
        case AST_CAST:          ast_child(w, "operand", n->a); ast_child(w, "type", n->type); break;
        case AST_CALL:          ast_child(w, "callee", n->a); ast_child_list(w, "args", &n->list); break;
        case AST_MEMBER:        ast_child(w, "object", n->a); break;
        case AST_INDEX:         ast_child(w, "object", n->a); ast_child(w, "index", n->b); break;
        case AST_ARRAY_LIT:     ast_child_list(w, "elements", &n->list); break;
        case AST_STRUCT_LIT:    ast_child_list(w, "fields", &n->list); break;
        case AST_FIELD_INIT:    ast_child(w, "value", n->a); break;
        case AST_TYPE:          if (n->dims.len) ast_child_list(w, "dims", &n->dims); break;
        case AST_LAYOUT_BLOCK:  ast_child_list(w, "items", &n->list); break;
        case AST_LAYOUT_ELEMENT:ast_child_list(w, "children", &n->list); break;
        case AST_LAYOUT_ATTR:   ast_child(w, "value", n->a); break;
        case AST_LAYOUT_COMPONENT: ast_child_list(w, "params", &n->list); ast_child(w, "body", n->a); break;
        default: break;   
    }
    xml_close(w);
}

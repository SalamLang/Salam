#include "core/prelude.h"
#include "token/token.h"

static void token_value_to_xml(xml_writer_t *w, const token_value_t *v)
{
    char buf[64];
    switch (v->kind) {
        case TV_NONE:
            return;
        case TV_INT:
            xml_open(w, "value"); xml_attr(w, "kind", "int");
            snprintf(buf, sizeof(buf), "%llu", (unsigned long long)v->as.i);
            xml_text(w, buf); xml_close(w);
            break;
        case TV_FLOAT:
            xml_open(w, "value"); xml_attr(w, "kind", "float");
            snprintf(buf, sizeof(buf), "%g", v->as.f);
            xml_text(w, buf); xml_close(w);
            break;
        case TV_STRING:
            xml_open(w, "value"); xml_attr(w, "kind", "string");
            xml_text(w, v->as.s ? v->as.s : "");
            xml_close(w);
            break;
        case TV_CHAR:
            xml_open(w, "value"); xml_attr(w, "kind", "char");
            snprintf(buf, sizeof(buf), "%llu", (unsigned long long)v->as.i);
            xml_text(w, buf); xml_close(w);
            break;
        case TV_BOOL:
            xml_open(w, "value"); xml_attr(w, "kind", "bool");
            xml_text(w, v->as.b ? "true" : "false");
            xml_close(w);
            break;
    }
}

void tokens_to_xml(xml_writer_t *w, const token_stream_t *ts)
{
    xml_open(w, "tokens");
    xml_attr(w, "source", token_stream_file(ts) ? token_stream_file(ts) : "");
    xml_attr_int(w, "count", (long long)token_stream_count(ts));
    for (size_t i = 0; i < token_stream_count(ts); i++) {
        const token_t *t = token_stream_at(ts, i);
        xml_open(w, "token");
            if (t->layout_mode) xml_attr(w, "mode", "layout");
            xml_leaf(w, "base", "token_t");
            xml_leaf(w, "type", token_kind_name(t->kind));
            xml_leaf(w, "source", t->lexeme ? t->lexeme : "");
            token_value_to_xml(w, &t->value);
            xml_open(w, "location");
                xml_leaf_int(w, "begin_line",   t->span.begin.line);
                xml_leaf_int(w, "begin_column", t->span.begin.col);
                xml_leaf_int(w, "end_line",     t->span.end.line);
                xml_leaf_int(w, "end_column",   t->span.end.col);
            xml_close(w);
        xml_close(w);
    }
    xml_close(w);
}

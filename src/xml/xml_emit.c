#include "xml/xml_internal.h"

static void xml_escaped(xml_writer_t *w, const char *s)
{
    { const char *p = s; for (; *p; p++) {
        switch (*p) {
            case '&':  sb_puts(w->out, "&amp;");  break;
            case '<':  sb_puts(w->out, "&lt;");   break;
            case '>':  sb_puts(w->out, "&gt;");   break;
            case '"':  sb_puts(w->out, "&quot;"); break;
            case '\'': sb_puts(w->out, "&apos;"); break;
            default:   sb_putc(w->out, *p);       break;
        }
    } }
}

void xml_attr(xml_writer_t *w, const char *key, const char *value)
{
    sb_putc(w->out, ' ');
    sb_puts(w->out, key);
    sb_puts(w->out, "=\"");
    xml_escaped(w, value);
    sb_putc(w->out, '"');
}

void xml_attr_int(xml_writer_t *w, const char *key, long long value)
{
    sb_putc(w->out, ' ');
    sb_puts(w->out, key);
    sb_printf(w->out, "=\"%lld\"", value);
}

void xml_text(xml_writer_t *w, const char *text)
{
    if (w->depth <= 0) return;   
    xml_frame_t *f = &w->stack[w->depth - 1];
    if (!f->printed_gt) {
        sb_putc(w->out, '>');
        f->printed_gt = true;
    }
    f->inlined = true;
    xml_escaped(w, text);
}

void xml_leaf(xml_writer_t *w, const char *tag, const char *text)
{
    xml_open(w, tag);
    xml_text(w, text);
    xml_close(w);
}

void xml_leaf_int(xml_writer_t *w, const char *tag, long long value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", value);
    xml_leaf(w, tag, buf);
}

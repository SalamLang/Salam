#include "xml/xml_internal.h"

#define XML_INIT_DEPTH 128

static char *xstrdup(const char *s)
{
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) abort();
    memcpy(p, s, n + 1);
    return p;
}

xml_writer_t *xml_new(sb_t *out)
{
    xml_writer_t *w = (xml_writer_t *)malloc(sizeof(*w));
    if (!w) abort();
    w->out = out;
    w->depth = 0;
    w->cap = XML_INIT_DEPTH;
    w->stack = (xml_frame_t *)malloc(salam_size_mul((size_t)w->cap, sizeof(xml_frame_t)));
    if (!w->stack) abort();
    return w;
}

void xml_free(xml_writer_t *w)
{
    
    while (w->depth > 0) {
        free(w->stack[--w->depth].name);
    }
    free(w->stack);
    free(w);
}

static void xml_indent(xml_writer_t *w, int depth)
{
    for (int i = 0; i < depth; i++) sb_puts(w->out, "  ");
}

void xml_decl(xml_writer_t *w)
{
    sb_puts(w->out, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
}

void xml_open(xml_writer_t *w, const char *tag)
{
    if (w->depth > 0) {
        xml_frame_t *parent = &w->stack[w->depth - 1];
        if (!parent->printed_gt) {
            sb_puts(w->out, ">\n");
            parent->printed_gt = true;
        }
    }
    xml_indent(w, w->depth);
    sb_putc(w->out, '<');
    sb_puts(w->out, tag);
    if (w->depth >= w->cap) {            
        w->cap *= 2;
        w->stack = (xml_frame_t *)realloc(w->stack,
                       salam_size_mul((size_t)w->cap, sizeof(xml_frame_t)));
        if (!w->stack) abort();
    }
    xml_frame_t *f = &w->stack[w->depth++];
    f->name = xstrdup(tag);
    f->printed_gt = false;
    f->inlined = false;
}

void xml_close(xml_writer_t *w)
{
    xml_frame_t *f = &w->stack[--w->depth];
    if (!f->printed_gt) {
        sb_puts(w->out, " />\n");
    } else if (f->inlined) {
        sb_puts(w->out, "</");
        sb_puts(w->out, f->name);
        sb_puts(w->out, ">\n");
    } else {
        xml_indent(w, w->depth);
        sb_puts(w->out, "</");
        sb_puts(w->out, f->name);
        sb_puts(w->out, ">\n");
    }
    free(f->name);
}

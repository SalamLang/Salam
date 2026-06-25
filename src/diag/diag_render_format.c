#include "core/prelude.h"
#include "diag/diag_internal.h"

static void json_str(FILE *sink, const char *s)
{
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if      (c == '"')  fputs("\\\"", sink);
        else if (c == '\\') fputs("\\\\", sink);
        else if (c == '\n') fputs("\\n",  sink);
        else if (c == '\r') fputs("\\r",  sink);
        else if (c == '\t') fputs("\\t",  sink);
        else if (c < 0x20)  fprintf(sink, "\\u%04x", (unsigned)c);
        else                fputc(c, sink);
    }
}

void diag_render_json(FILE *sink,
                      const char *level, int code, bool is_error,
                      const char *file, const src_span_t *span,
                      const char *body, const char *help)
{
    fputs("{\"level\":\"", sink); json_str(sink, level); fputc('"', sink);
    if (code > 0) {
        char cs[16];
        snprintf(cs, sizeof(cs), "%c%03d", is_error ? 'E' : 'W', code);
        fputs(",\"code\":\"", sink); json_str(sink, cs); fputc('"', sink);
    }
    fputs(",\"message\":\"", sink); json_str(sink, body); fputc('"', sink);
    if (file) {
        fputs(",\"file\":\"", sink); json_str(sink, file); fputc('"', sink);
    }
    if (span && span->begin.line > 0) {
        fprintf(sink, ",\"line\":%u,\"column\":%u", span->begin.line, span->begin.col);
        if (span->length > 0)
            fprintf(sink, ",\"length\":%u", span->length);
    }
    if (help) {
        fputs(",\"help\":\"", sink); json_str(sink, help); fputc('"', sink);
    }
    fputs("}\n", sink);
}

static void xml_str(FILE *sink, const char *s)
{
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if      (c == '&')  fputs("&amp;",  sink);
        else if (c == '<')  fputs("&lt;",   sink);
        else if (c == '>')  fputs("&gt;",   sink);
        else if (c == '"')  fputs("&quot;", sink);
        else if (c == '\'') fputs("&apos;", sink);
        else                fputc(c, sink);
    }
}

void diag_render_xml(FILE *sink,
                     const char *level, int code, bool is_error,
                     const char *file, const src_span_t *span,
                     const char *body, const char *help)
{
    fputs("<diagnostic level=\"", sink); xml_str(sink, level); fputc('"', sink);
    if (code > 0)
        fprintf(sink, " code=\"%c%03d\"", is_error ? 'E' : 'W', code);
    if (file) {
        fputs(" file=\"", sink); xml_str(sink, file); fputc('"', sink);
    }
    if (span && span->begin.line > 0) {
        fprintf(sink, " line=\"%u\" column=\"%u\"", span->begin.line, span->begin.col);
        if (span->length > 0)
            fprintf(sink, " length=\"%u\"", span->length);
    }
    fputs("><message>", sink); xml_str(sink, body); fputs("</message>", sink);
    if (help) {
        fputs("<help>", sink); xml_str(sink, help); fputs("</help>", sink);
    }
    fputs("</diagnostic>\n", sink);
}

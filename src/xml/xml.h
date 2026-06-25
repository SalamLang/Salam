#ifndef SALAM_XML_XML_H
#define SALAM_XML_XML_H
#include "core/sb.h"

typedef struct xml_writer xml_writer_t;
xml_writer_t *xml_new(sb_t *out);
void          xml_free(xml_writer_t *w);
void xml_decl(xml_writer_t *w);                                 /* <?xml ...?> */
void xml_open(xml_writer_t *w, const char *tag);
void xml_attr(xml_writer_t *w, const char *key, const char *value);
void xml_attr_int(xml_writer_t *w, const char *key, long long value);
void xml_text(xml_writer_t *w, const char *text);
void xml_leaf(xml_writer_t *w, const char *tag, const char *text);
void xml_leaf_int(xml_writer_t *w, const char *tag, long long value);
void xml_close(xml_writer_t *w);
#endif /* SALAM_XML_XML_H */

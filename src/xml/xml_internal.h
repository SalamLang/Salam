#ifndef SALAM_XML_INTERNAL_H
#define SALAM_XML_INTERNAL_H

#include "core/prelude.h"
#include "xml/xml.h"

typedef struct {
    char *name;        /* heap copy of the tag name */
    bool  printed_gt;  /* has the start tag's '>' been emitted? */
    bool  inlined;     /* does this element hold inline text (close on same line)? */
} xml_frame_t;

struct xml_writer {
    sb_t        *out;
    xml_frame_t *stack;   /* grows on demand: deeply nested input must not overflow */
    int          depth;
    int          cap;
};

#endif /* SALAM_XML_INTERNAL_H */

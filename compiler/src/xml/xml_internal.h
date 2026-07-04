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

#ifndef SALAM_XML_INTERNAL_H
#define SALAM_XML_INTERNAL_H

#include "core/prelude.h"
#include "xml/xml.h"

typedef struct {
    char *name;
    bool printed_gt;
    bool inlined;
} xml_frame_t;

struct xml_writer {
    sb_t *out;
    xml_frame_t *stack;
    int depth;
    int cap;
};

#endif /* SALAM_XML_INTERNAL_H */

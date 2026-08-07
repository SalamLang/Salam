/*
 * Salam Programming Language (2024-2026)
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

#ifndef SALAM_DOC_DOCGEN_H
#define SALAM_DOC_DOCGEN_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"

/* Declaration kinds. Mirrors compiler/docgen.salam's DOC_* constants; the
 * numeric values never leave the process (the rendered JSON carries the
 * spelled-out kind name from doc_kind_name()), so only the set matters. */
typedef enum {
    DOC_CONST = 0,
    DOC_VAR,
    DOC_TYPE,
    DOC_STRUCT,
    DOC_INTERFACE,
    DOC_ENUM,
    DOC_IMPL,
    DOC_FUNC,
    DOC_FIELD,
    DOC_ENUM_MEMBER
} doc_kind_t;

/* One `@lang`-tagged alias or doc-text section. `lang` is "" for the
 * untagged/default bucket. */
typedef struct {
    const char *lang;
    const char *text;
} doc_pair_t;

typedef struct doc_decl doc_decl_t;

struct doc_decl {
    doc_kind_t kind;
    const char *name;
    const char *sig;
    bool is_pub;
    bool is_deprecated;
    int file_idx;
    int line;
    int self;   /* own index into doc_db_t.decls, for the emitted JSON links */
    int parent; /* -1 = top level, else index of the owning struct/enum/impl */
    vec_t members; /* doc_decl_t*, child rows in declaration order */
    vec_t aliases; /* doc_pair_t*, from @lang name metas */
    vec_t doc;     /* doc_pair_t*, from the preceding doc comment */
};

typedef struct {
    arena_t *arena;
    vec_t decls; /* doc_decl_t* */
    vec_t files; /* const char*, display paths in discovery order */
} doc_db_t;

doc_db_t *docgen_db_new(void);

void docgen_db_free(doc_db_t *db);

/* Lexes, parses, prunes platform-conditional blocks and (best-effort) runs the
 * semantic pass over one file, then walks its top-level declarations into `db`
 * under the display name `rel`. Never aborts the whole run on one file's
 * errors: a file that fails to parse cleanly still contributes whatever it
 * could produce, and the return value is purely informational (the CLI turns
 * it into a warning count).
 *
 * Takes no logger on purpose. Each call parses into its own arena and releases
 * it before returning, but logger_add_diag_source() retains borrowed pointers
 * to every source file the semantic pass loads - a caller-owned logger would
 * outlive that arena and dangle. The silent logger this creates internally is
 * freed with the arena it borrows from. */
bool docgen_parse_file(doc_db_t *db, const langpack_t *pack, const char *path,
                       const char *rel, const char *const *defines, int ndefines);

/* Appends the complete self-contained HTML document (inline <style> and
 * <script>, no external assets) to `out`. */
void docgen_render(const doc_db_t *db, const char *title, sb_t *out);

/* doc_assets.c: NULL-terminated arrays of already-newline-terminated lines. */
extern const char *const doc_css_lines[];
extern const char *const doc_js_lines[];
extern const char *const doc_body_lines[];

#endif /* SALAM_DOC_DOCGEN_H */

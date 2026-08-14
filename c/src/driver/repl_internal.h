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

#ifndef SALAM_DRIVER_REPL_INTERNAL_H
#define SALAM_DRIVER_REPL_INTERNAL_H

#include "core/prelude.h"
#include "langpack/langpack.h"

/* --- repl_line.c: the prompt's line editor ------------------------- */

/* Reads one line with history and in-place editing when stdin is a
 * terminal, and with plain fgets when it is a pipe. Returns a malloc'd
 * line, or NULL on EOF / Ctrl-C / Ctrl-D on an empty line. */
char *repl_readline(const char *prompt);

void repl_hist_push(const char *line);

void repl_hist_clear(void);

/* --- repl_session.c: what the session accumulates ------------------ */

/* Buckets, in the order the language demands they appear in a file:
 * `package` first, then imports, then externs (E083), then globals
 * (E085), then everything else. A REPL user types definitions in
 * whatever order the thought arrives, so the session sorts them back
 * into a legal program on the way out instead of rejecting the input. */
typedef enum {
    RS_PKG = 0,
    RS_IMPORT,
    RS_EXTERN,
    RS_GLOBAL,
    RS_TYPE, /* struct/enum/type/interface/impl - E087 wants these first */
    RS_DEF,  /* functions, private ones before pub ones (E088) */
    RS_BUCKETS,
    /* Not a bucket: a statement, which lands in the synthesized main. */
    RS_STMT
} repl_kind_t;

typedef struct {
    repl_kind_t kind;
    char *name; /* what this entry defines; "" when it names nothing */
    char *text; /* raw source, newline-terminated */
} repl_entry_t;

typedef struct {
    repl_entry_t *defs;
    size_t ndefs, capdefs;
    char **stmts;
    size_t nstmts, capstmts;
    /* The user wrote their own entry function, so no main is synthesized
     * and bare statements have nowhere to go. */
    bool has_main;
} repl_session_t;

void repl_session_init(repl_session_t *s);

void repl_session_free(repl_session_t *s);

/* Which bucket does `line` (the first line of a typed block) belong to,
 * and what does it define? `name` gets the defined symbol, or "" when
 * the entry names nothing that a later redefinition could replace.
 * Keywords are matched through `pack`, so this reads fa/ar sessions the
 * same way it reads en ones. */
repl_kind_t repl_classify(const langpack_t *pack, const char *line, char *name,
                          size_t namesz);

/* Is `line` a bare expression - no keyword opener, no assignment, one
 * line? Those are questions, and the REPL answers them by printing the
 * value instead of evaluating for effect. */
bool repl_is_expr_line(const langpack_t *pack, const char *line);

/* Accepts an entry into the session. A definition with the same name as
 * an existing one replaces it in place - redefining a function is the
 * most ordinary thing a REPL user does. */
void repl_session_add(repl_session_t *s, const langpack_t *pack, repl_kind_t kind,
                      const char *name, const char *text);

void repl_session_add_stmt(repl_session_t *s, const char *text);

/* Drops the newest statement. Returns false when there was none. */
bool repl_session_undo(repl_session_t *s);

/* The session as one compilable program: buckets in file order, then a
 * synthesized `func main:` holding every accepted statement. `cand` is
 * an entry that has not been accepted yet (NULL for the session alone);
 * a candidate that redefines an existing name shadows it here too, so
 * the check that follows sees the program the user is asking for rather
 * than a duplicate-symbol error. Caller frees. */
char *repl_render(const repl_session_t *s, const langpack_t *pack,
                  const repl_entry_t *cand);

#endif /* SALAM_DRIVER_REPL_INTERNAL_H */

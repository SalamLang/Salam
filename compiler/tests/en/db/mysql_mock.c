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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct sqlite3 sqlite3;

typedef struct sqlite3_stmt sqlite3_stmt;

extern int sqlite3_open(const char *, sqlite3 **);
extern int sqlite3_close(sqlite3 *);
extern int sqlite3_prepare_v2(sqlite3 *, const char *, int, sqlite3_stmt **, const char **);
extern int sqlite3_step(sqlite3_stmt *);
extern int sqlite3_finalize(sqlite3_stmt *);
extern int sqlite3_reset(sqlite3_stmt *);
extern int sqlite3_column_count(sqlite3_stmt *);
extern const unsigned char *sqlite3_column_text(sqlite3_stmt *, int);
extern const char *sqlite3_column_name(sqlite3_stmt *, int);
extern int sqlite3_changes(sqlite3 *);
extern long long sqlite3_last_insert_rowid(sqlite3 *);
extern const char *sqlite3_errmsg(sqlite3 *);
extern int sqlite3_exec(sqlite3 *, const char *, void *, void *, char **);

#define MOCK_SQLITE_ROW 100

typedef struct {
    sqlite3 *db;
    sqlite3_stmt *pending;
    int pending_ncols;
    long long changes;
    long long insert_id;
    int err;
    char errbuf[256];
} MockConn;

typedef struct {
    sqlite3_stmt *stmt;
    int ncols;
    char **rowbuf;
    char *namecell;
} MockRes;

void *mysql_init(void *unused) {
    (void)unused;
    return calloc(1, sizeof(MockConn));
}

void *mysql_real_connect(void *conn, const char *host, const char *user,
                         const char *passwd, const char *db, uint32_t port,
                         void *unix_socket, uint64_t client_flag) {
    MockConn *c = (MockConn *)conn;
    (void)host; (void)user; (void)passwd; (void)db;
    (void)port; (void)unix_socket; (void)client_flag;
    if (!c) return 0;
    if (sqlite3_open(":memory:", &c->db) != 0) return 0;
    return c;
}

void mysql_close(void *conn) {
    MockConn *c = (MockConn *)conn;
    if (!c) return;
    if (c->pending) sqlite3_finalize(c->pending);
    if (c->db) sqlite3_close(c->db);
    free(c);
}

int32_t mysql_ping(void *conn) {
    MockConn *c = (MockConn *)conn;
    return (c && c->db) ? 0 : 1;
}

int32_t mysql_select_db(void *conn, const char *db) { (void)conn; (void)db; return 0; }

int32_t mysql_set_character_set(void *conn, const char *cs) { (void)conn; (void)cs; return 0; }

int32_t mysql_query(void *conn, const char *stmt) {
    MockConn *c = (MockConn *)conn;
    if (!c || !c->db) return 1;
    if (c->pending) { sqlite3_finalize(c->pending); c->pending = 0; }
    const char *sql = stmt;
    if (strncmp(stmt, "START TRANSACTION", 17) == 0) sql = "BEGIN";
    sqlite3_stmt *st = 0;
    if (sqlite3_prepare_v2(c->db, sql, -1, &st, 0) != 0 || !st) {
        c->err = 1;
        snprintf(c->errbuf, sizeof(c->errbuf), "%s", sqlite3_errmsg(c->db));
        if (st) sqlite3_finalize(st);
        return 1;
    }
    c->err = 0;
    c->errbuf[0] = 0;
    int ncols = sqlite3_column_count(st);
    if (ncols == 0) {
        sqlite3_step(st);
        c->changes = sqlite3_changes(c->db);
        c->insert_id = sqlite3_last_insert_rowid(c->db);
        sqlite3_finalize(st);
        return 0;
    }
    c->pending = st;
    c->pending_ncols = ncols;
    return 0;
}

void *mysql_store_result(void *conn) {
    MockConn *c = (MockConn *)conn;
    if (!c || !c->pending) return 0;
    MockRes *r = (MockRes *)calloc(1, sizeof(MockRes));
    r->stmt = c->pending;
    r->ncols = c->pending_ncols;
    r->rowbuf = (char **)calloc((size_t)(r->ncols > 0 ? r->ncols : 1), sizeof(char *));
    c->pending = 0;
    return r;
}

void mysql_free_result(void *result) {
    MockRes *r = (MockRes *)result;
    if (!r) return;
    if (r->stmt) sqlite3_finalize(r->stmt);
    free(r->rowbuf);
    free(r);
}

void *mysql_fetch_row(void *result) {
    MockRes *r = (MockRes *)result;
    if (!r || !r->stmt) return 0;
    if (sqlite3_step(r->stmt) != MOCK_SQLITE_ROW) return 0;
    for (int i = 0; i < r->ncols; i++)
        r->rowbuf[i] = (char *)sqlite3_column_text(r->stmt, i);
    return r->rowbuf;
}

uint32_t mysql_num_fields(void *result) {
    MockRes *r = (MockRes *)result;
    return r ? (uint32_t)r->ncols : 0;
}

uint64_t mysql_num_rows(void *result) {
    MockRes *r = (MockRes *)result;
    if (!r || !r->stmt) return 0;
    uint64_t n = 0;
    sqlite3_reset(r->stmt);
    while (sqlite3_step(r->stmt) == MOCK_SQLITE_ROW) n++;
    sqlite3_reset(r->stmt);
    return n;
}

uint32_t mysql_field_count(void *conn) {
    MockConn *c = (MockConn *)conn;
    return c ? (uint32_t)c->pending_ncols : 0;
}

void *mysql_fetch_field_direct(void *result, uint32_t fieldnr) {
    MockRes *r = (MockRes *)result;
    if (!r || !r->stmt) return 0;
    r->namecell = (char *)sqlite3_column_name(r->stmt, (int)fieldnr);
    return &r->namecell;
}

uint64_t mysql_affected_rows(void *conn) {
    MockConn *c = (MockConn *)conn;
    return c ? (uint64_t)c->changes : 0;
}

uint64_t mysql_insert_id(void *conn) {
    MockConn *c = (MockConn *)conn;
    return c ? (uint64_t)c->insert_id : 0;
}

unsigned char mysql_autocommit(void *conn, unsigned char mode) {
    MockConn *c = (MockConn *)conn;
    if (!c || !c->db) return 1;
    sqlite3_exec(c->db, mode ? "COMMIT" : "BEGIN", 0, 0, 0);
    return 0;
}

unsigned char mysql_commit(void *conn) {
    MockConn *c = (MockConn *)conn;
    if (!c || !c->db) return 1;
    sqlite3_exec(c->db, "COMMIT", 0, 0, 0);
    return 0;
}

unsigned char mysql_rollback(void *conn) {
    MockConn *c = (MockConn *)conn;
    if (!c || !c->db) return 1;
    sqlite3_exec(c->db, "ROLLBACK", 0, 0, 0);
    return 0;
}

uint64_t mysql_real_escape_string(void *conn, void *to, const char *from, uint64_t length) {
    (void)conn;
    char *out = (char *)to;
    uint64_t o = 0;
    for (uint64_t i = 0; i < length; i++) {
        char ch = from[i];
        if (ch == '\'') out[o++] = '\'';
        out[o++] = ch;
    }
    out[o] = '\0';
    return o;
}

const char *mysql_error(void *conn) {
    MockConn *c = (MockConn *)conn;
    return (c && c->errbuf[0]) ? c->errbuf : "";
}

uint32_t mysql_errno(void *conn) {
    MockConn *c = (MockConn *)conn;
    return c ? (uint32_t)c->err : 0;
}

const char *mysql_get_client_info(void) { return "mock-mariadb (sqlite-backed)"; }

const char *mysql_get_server_info(void *conn) { (void)conn; return "mock-server"; }

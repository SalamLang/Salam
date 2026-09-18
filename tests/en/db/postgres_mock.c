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

/*
 * Enough of libpq to exercise std/db/postgres without a Postgres server,
 * implemented over sqlite3 - the same trick tests/en/db/mysql_mock.c plays for
 * libmysqlclient, and for the same reason: CI has no server, and the driver's
 * own logic (result walking, type mapping, transactions, quoting) is worth
 * testing regardless.
 *
 * The difference from the mysql mock is that libpq MATERIALISES a result:
 * PQexec hands back every row at once and the caller indexes into it, so this
 * slurps the sqlite cursor into an array up front. That is exactly the path
 * the real driver takes, which is the part worth covering.
 *
 * SQL still has to be sqlite-compatible; these tests stick to the subset both
 * engines share.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;

extern int sqlite3_open(const char *, sqlite3 **);
extern int sqlite3_close(sqlite3 *);
extern int sqlite3_prepare_v2(sqlite3 *, const char *, int, sqlite3_stmt **,
                              const char **);
extern int sqlite3_step(sqlite3_stmt *);
extern int sqlite3_finalize(sqlite3_stmt *);
extern int sqlite3_column_count(sqlite3_stmt *);
extern const unsigned char *sqlite3_column_text(sqlite3_stmt *, int);
extern const char *sqlite3_column_name(sqlite3_stmt *, int);
extern int sqlite3_column_type(sqlite3_stmt *, int);
extern int sqlite3_changes(sqlite3 *);
extern long long sqlite3_last_insert_rowid(sqlite3 *);
extern const char *sqlite3_errmsg(sqlite3 *);

#define MOCK_SQLITE_ROW 100

/* PQresultStatus */
#define PGRES_EMPTY_QUERY 0
#define PGRES_COMMAND_OK 1
#define PGRES_TUPLES_OK 2
#define PGRES_FATAL_ERROR 7

/* pg_type OIDs the driver maps to db.TYPE_* */
#define OID_BYTEA 17
#define OID_INT4 23
#define OID_TEXT 25
#define OID_FLOAT8 701

typedef struct {
    sqlite3 *db;
    char errbuf[512];
} PgConn;

typedef struct {
    int status;
    int ntuples;
    int nfields;
    char **names;        /* nfields */
    unsigned int *types; /* nfields */
    char **cells;        /* ntuples * nfields, NULL for SQL NULL */
    char cmdtuples[32];
    char errbuf[512];
} PgResult;

static char *mock_dup(const char *s)
{
    size_t n;
    char *p;
    if (!s) return 0;
    n = strlen(s) + 1;
    p = (char *)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static unsigned int mock_oid(int sqlite_type)
{
    switch (sqlite_type) {
    case 1:
        return OID_INT4;
    case 2:
        return OID_FLOAT8;
    case 4:
        return OID_BYTEA;
    default:
        return OID_TEXT;
    }
}

static PgResult *mock_result_new(int status)
{
    PgResult *r = (PgResult *)calloc(1, sizeof(PgResult));
    if (r) r->status = status;
    return r;
}

void *PQconnectdb(const char *conninfo)
{
    PgConn *c = (PgConn *)calloc(1, sizeof(PgConn));
    (void)conninfo;
    if (!c) return 0;
    if (sqlite3_open(":memory:", &c->db) != 0) {
        snprintf(c->errbuf, sizeof(c->errbuf), "could not open mock database");
        c->db = 0;
    }
    return c;
}

int PQstatus(void *conn)
{
    PgConn *c = (PgConn *)conn;
    return (c && c->db) ? 0 : 1; /* CONNECTION_OK == 0 */
}

void PQfinish(void *conn)
{
    PgConn *c = (PgConn *)conn;
    if (!c) return;
    if (c->db) sqlite3_close(c->db);
    free(c);
}

void PQreset(void *conn)
{
    (void)conn;
}

const char *PQerrorMessage(void *conn)
{
    PgConn *c = (PgConn *)conn;
    return c ? c->errbuf : "";
}

void *PQexec(void *conn, const char *query)
{
    PgConn *c = (PgConn *)conn;
    sqlite3_stmt *st = 0;
    PgResult *r;
    int ncols, cap, n;

    if (!c || !c->db) return 0;
    c->errbuf[0] = 0;

    if (sqlite3_prepare_v2(c->db, query, -1, &st, 0) != 0 || !st) {
        snprintf(c->errbuf, sizeof(c->errbuf), "%s", sqlite3_errmsg(c->db));
        if (st) sqlite3_finalize(st);
        r = mock_result_new(PGRES_FATAL_ERROR);
        if (r) snprintf(r->errbuf, sizeof(r->errbuf), "%s", c->errbuf);
        return r;
    }

    ncols = sqlite3_column_count(st);
    if (ncols == 0) {
        sqlite3_step(st);
        sqlite3_finalize(st);
        r = mock_result_new(PGRES_COMMAND_OK);
        if (r) snprintf(r->cmdtuples, sizeof(r->cmdtuples), "%d", sqlite3_changes(c->db));
        return r;
    }

    r = mock_result_new(PGRES_TUPLES_OK);
    if (!r) {
        sqlite3_finalize(st);
        return 0;
    }
    r->nfields = ncols;
    r->names = (char **)calloc((size_t)ncols, sizeof(char *));
    r->types = (unsigned int *)calloc((size_t)ncols, sizeof(unsigned int));
    for (n = 0; n < ncols; n++) {
        r->names[n] = mock_dup(sqlite3_column_name(st, n));
        r->types[n] = OID_TEXT;
    }

    cap = 8;
    r->cells = (char **)calloc((size_t)(cap * ncols), sizeof(char *));
    while (sqlite3_step(st) == MOCK_SQLITE_ROW) {
        if (r->ntuples == cap) {
            char **grown;
            cap *= 2;
            grown = (char **)calloc((size_t)(cap * ncols), sizeof(char *));
            memcpy(grown, r->cells, (size_t)(r->ntuples * ncols) * sizeof(char *));
            free(r->cells);
            r->cells = grown;
        }
        for (n = 0; n < ncols; n++) {
            int t = sqlite3_column_type(st, n);
            const unsigned char *v = sqlite3_column_text(st, n);
            /* The first row that carries a value decides the column's type,
             * the way a real Postgres column has one declared type. */
            if (t != 5 && r->types[n] == OID_TEXT && r->ntuples == 0)
                r->types[n] = mock_oid(t);
            r->cells[r->ntuples * ncols + n] = (t == 5) ? 0 : mock_dup((const char *)v);
        }
        r->ntuples++;
    }
    sqlite3_finalize(st);
    snprintf(r->cmdtuples, sizeof(r->cmdtuples), "%d", r->ntuples);
    return r;
}

int PQresultStatus(void *res)
{
    PgResult *r = (PgResult *)res;
    return r ? r->status : PGRES_FATAL_ERROR;
}

const char *PQresultErrorMessage(void *res)
{
    PgResult *r = (PgResult *)res;
    return r ? r->errbuf : "";
}

int PQntuples(void *res)
{
    PgResult *r = (PgResult *)res;
    return r ? r->ntuples : 0;
}

int PQnfields(void *res)
{
    PgResult *r = (PgResult *)res;
    return r ? r->nfields : 0;
}

const char *PQfname(void *res, int col)
{
    PgResult *r = (PgResult *)res;
    if (!r || col < 0 || col >= r->nfields || !r->names[col]) return "";
    return r->names[col];
}

unsigned int PQftype(void *res, int col)
{
    PgResult *r = (PgResult *)res;
    if (!r || col < 0 || col >= r->nfields) return OID_TEXT;
    return r->types[col];
}

const char *PQgetvalue(void *res, int row, int col)
{
    PgResult *r = (PgResult *)res;
    char *cell;
    if (!r || row < 0 || row >= r->ntuples || col < 0 || col >= r->nfields) return "";
    cell = r->cells[row * r->nfields + col];
    return cell ? cell : ""; /* libpq reports NULL as an empty string */
}

int PQgetisnull(void *res, int row, int col)
{
    PgResult *r = (PgResult *)res;
    if (!r || row < 0 || row >= r->ntuples || col < 0 || col >= r->nfields) return 1;
    return r->cells[row * r->nfields + col] ? 0 : 1;
}

const char *PQcmdTuples(void *res)
{
    PgResult *r = (PgResult *)res;
    return r ? r->cmdtuples : "";
}

void PQclear(void *res)
{
    PgResult *r = (PgResult *)res;
    int i;
    if (!r) return;
    for (i = 0; i < r->nfields; i++)
        free(r->names ? r->names[i] : 0);
    for (i = 0; i < r->ntuples * r->nfields; i++)
        free(r->cells ? r->cells[i] : 0);
    free(r->names);
    free(r->types);
    free(r->cells);
    free(r);
}

int PQlibVersion(void)
{
    return 160002;
}

int PQserverVersion(void *conn)
{
    PgConn *c = (PgConn *)conn;
    return (c && c->db) ? 160002 : 0;
}

/* Single-quoted, with embedded quotes and backslashes doubled - the shape
 * libpq's own escaper produces for a standard_conforming_strings session. */
void *PQescapeLiteral(void *conn, const char *s, size_t length)
{
    size_t i, o = 0;
    char *out;
    (void)conn;
    if (!s) return 0;
    out = (char *)malloc(length * 2 + 3);
    if (!out) return 0;
    out[o++] = '\'';
    for (i = 0; i < length; i++) {
        if (s[i] == '\'') out[o++] = '\'';
        out[o++] = s[i];
    }
    out[o++] = '\'';
    out[o] = 0;
    return out;
}

void PQfreemem(void *p)
{
    free(p);
}

const char *PQdb(void *conn)
{
    return conn ? "mockdb" : "";
}

const char *PQuser(void *conn)
{
    return conn ? "mockuser" : "";
}

const char *PQhost(void *conn)
{
    return conn ? "127.0.0.1" : "";
}

const char *PQport(void *conn)
{
    return conn ? "5432" : "";
}

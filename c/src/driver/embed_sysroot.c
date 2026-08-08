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

#include "core/prelude.h"
#include "core/sal_format.h"
#include "driver/embed_sysroot.h"

#include <errno.h>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <direct.h>
#  include <process.h>
#  define salam_mkdir(p) _mkdir(p)
#  define salam_getpid() _getpid()
#else
#  include <unistd.h>
#  define salam_mkdir(p) mkdir((p), 0755)
#  define salam_getpid() getpid()
#endif

static int path_exists(const char *p)
{
    struct stat st;
    return stat(p, &st) == 0;
}

static int mkdir_p(const char *path)
{
    char buf[1024];
    size_t i, len = strlen(path);
    if (len == 0 || len >= sizeof buf) return -1;
    memcpy(buf, path, len + 1);
    for (i = 1; i < len; i++) {
        if (buf[i] == '/' || buf[i] == '\\') {
            char sep = buf[i];
            buf[i] = '\0';
            if (buf[0] && buf[i - 1] != ':' && !path_exists(buf) &&
                salam_mkdir(buf) != 0 && errno != EEXIST)
                return -1;
            buf[i] = sep;
        }
    }
    if (!path_exists(buf) && salam_mkdir(buf) != 0 && errno != EEXIST) return -1;
    return 0;
}

static int mkdir_parent(const char *file)
{
    char buf[1024];
    char *slash, *bs, *cut;
    size_t len = strlen(file);
    if (len >= sizeof buf) return -1;
    memcpy(buf, file, len + 1);
    slash = strrchr(buf, '/');
    bs = strrchr(buf, '\\');
    cut = (bs && (!slash || bs > slash)) ? bs : slash;
    if (!cut) return 0;
    *cut = '\0';
    return mkdir_p(buf);
}

static unsigned long tar_oct(const unsigned char *p, int n)
{
    unsigned long v = 0;
    int i = 0;
    while (i < n && (p[i] == ' ' || p[i] == '\0'))
        i++;
    for (; i < n && p[i] >= '0' && p[i] <= '7'; i++)
        v = (v << 3) | (unsigned long)(p[i] - '0');
    return v;
}

static int untar(const unsigned char *tar, size_t len, const char *dest)
{
    size_t off = 0;
    char longname[512];
    int have_long = 0;
    while (off + 512 <= len) {
        const unsigned char *h = tar + off;
        char name[512], full[1200];
        unsigned long size;
        char type;
        size_t blocks;
        if (h[0] == '\0') break;
        if (have_long) {
            sal_snprintf(name, sizeof name, "%s", longname);
            have_long = 0;
        } else {
            char nm[101], pfx[156];
            memcpy(nm, h, 100);
            nm[100] = '\0';
            memcpy(pfx, h + 345, 155);
            pfx[155] = '\0';
            if (pfx[0])
                sal_snprintf(name, sizeof name, "%s/%s", pfx, nm);
            else
                sal_snprintf(name, sizeof name, "%s", nm);
        }
        size = tar_oct(h + 124, 12);
        type = (char)h[156];
        off += 512;
        if (size > len - off) break;
        blocks = (size_t)((size + 511) / 512);
        if (off + blocks * 512 > len) break;

        if (type == 'L') {
            size_t cp = size < sizeof longname ? (size_t)size : sizeof longname - 1;
            memcpy(longname, tar + off, cp);
            longname[cp] = '\0';
            have_long = 1;
            off += blocks * 512;
            continue;
        }
        if (name[0] == '\0' || name[0] == '/' || name[0] == '\\' || strstr(name, "..")) {
            off += blocks * 512;
            continue;
        }
        sal_snprintf(full, sizeof full, "%s/%s", dest, name);
        if (type == '5') {
            if (mkdir_p(full) != 0) return -1;
        } else if (type == '0' || type == '\0' || type == '7') {
            FILE *f;
            if (mkdir_parent(full) != 0) return -1;
            f = fopen(full, "wb");
            if (!f) return -1;
            if (size > 0) fwrite(tar + off, 1, (size_t)size, f);
            fclose(f);
        }
        off += blocks * 512;
    }
    return 0;
}

static const char *cache_root(void)
{
    const char *e;
    if ((e = getenv("SALAM_CACHE")) && e[0]) return e;
    if ((e = getenv("XDG_CACHE_HOME")) && e[0]) return e;
#if defined(_WIN32)
    if ((e = getenv("LOCALAPPDATA")) && e[0]) return e;
    if ((e = getenv("TEMP")) && e[0]) return e;
#else
    if ((e = getenv("HOME")) && e[0]) {
        static char hc[1024];
        sal_snprintf(hc, sizeof hc, "%s/.cache", e);
        return hc;
    }
    if ((e = getenv("TMPDIR")) && e[0]) return e;
    return "/tmp";
#endif
    return ".";
}

int salam_materialize_sysroot(const char *name, const unsigned char *tar, size_t tar_len,
                              char *out, size_t out_n)
{
    char marker[1200];
    if (!tar || tar_len < 512) return 0;
    sal_snprintf(out, out_n, "%s/salam/sysroots/%s_%lu", cache_root(), name,
                 (unsigned long)tar_len);
    sal_snprintf(marker, sizeof marker, "%s/.salam-ok", out);
    if (path_exists(marker)) return 1;

    /* Cold cache: another `salam build` process may be materializing the
     * same sysroot at the same moment - this runs once per invocation,
     * and a parallel test-suite worker pool routinely launches many
     * builds at once against a shared, initially-empty cache dir.
     * Extracting straight into `out` let two such processes' fopen(...,
     * "wb") calls on the same destination files race each other,
     * producing a truncated/corrupted extraction for whichever one lost
     * (observed as `-lsqlite3` failing to link, sporadically, only under
     * `run-tests.sh`'s parallel dispatch - never in a single serial
     * build). Extracting into a private, PID-suffixed staging directory
     * first and renaming it into place avoids that: concurrent
     * extractions never touch the same file, and the rename is the only
     * point of contention, which is atomic (or simply fails, handled
     * below) rather than interleavable. */
    char staging[1200];
    sal_snprintf(staging, sizeof staging, "%s.tmp.%ld", out, (long)salam_getpid());
    if (mkdir_p(staging) != 0) return 0;
    if (untar(tar, tar_len, staging) != 0) return 0;
    {
        char stage_marker[1200];
        sal_snprintf(stage_marker, sizeof stage_marker, "%s/.salam-ok", staging);
        FILE *f = fopen(stage_marker, "wb");
        if (f) {
            fputs("ok\n", f);
            fclose(f);
        }
    }
    if (rename(staging, out) != 0) {
        /* Lost the race to another process (Windows' rename() fails
         * outright when the destination already exists, unlike POSIX's
         * atomic replace) - if a complete extraction is already sitting
         * at `out`, that's fine, use it. The now-orphaned staging
         * directory this process extracted is harmless cache clutter,
         * not a correctness problem, and not worth a recursive-delete
         * implementation to clean up. */
        if (!path_exists(marker)) {
            /*
             * Neither the rename nor an already-complete extraction at
             * `out` is usable - but this process's own staging directory
             * is itself a complete, marker-verified extraction, so use it
             * directly instead of reporting failure.
             *
             * rename() can fail on Windows for reasons beyond "destination
             * exists": a virus scanner or a sibling build holding a handle
             * on the directory is enough, and a stale `out` left without
             * its marker by an earlier crashed run fails here forever
             * rather than transiently. Returning 0 turned any of those
             * into a spurious "no mingw sysroot ... install mingw-w64" on
             * a salam that had the sysroot embedded all along - observed
             * as sporadic build failures in the middle of an otherwise
             * green test run.
             */
            sal_snprintf(out, out_n, "%s", staging);
        }
    }
    return 1;
}

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
 * `salam serve [dir]` - a small static HTTP file server, the Salam
 * equivalent of `python -m http.server` or `php -S`.
 *
 * Rather than reimplementing sockets/threading/HTTP parsing here in C,
 * this generates a tiny Salam program that wires up the real std/web
 * router (web.Static + web.EnableListing, which own the actual
 * path-traversal safety and directory-listing logic - see
 * std/web/web.salam's _safe_static_path/_dir_listing_html) and builds +
 * runs it exactly like `salam run` does. That keeps exactly one
 * implementation of "serve files safely over HTTP" for both this C
 * toolchain and the self-hosted compiler/driver.salam to share (the
 * latter can call into std/web directly, without needing to shell out).
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "driver/serve_build.h"
#include "driver/build.h"
#include "driver/driver.h"
#include "core/sb.h"
#include "i18n/i18n.h"
#include "logger/logger.h"
#include <sys/stat.h>
#if defined(_WIN32)
#  include <process.h>
#else
#  include <unistd.h>
#  include <sys/wait.h>
#endif

static bool path_is_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0) return false;
    return (st.st_mode & S_IFMT) == S_IFDIR;
}

/* Appends `text` as a double-quoted Salam string literal, escaping the
 * two characters that would otherwise break out of it. */
static void sb_put_salam_string_literal(sb_t *s, const char *text)
{
    const unsigned char *p = (const unsigned char *)text;
    sb_putc(s, '"');
    for (; *p; p++) {
        if (*p == '\\' || *p == '"') sb_putc(s, '\\');
        sb_putc(s, (char)*p);
    }
    sb_putc(s, '"');
}

int driver_serve(options_t *opt)
{
    /* Same as driver_run: the default INFO level would otherwise print a
     * line for every token/module of the entire stdlib this generated
     * program transitively pulls in (compiled fresh on every `serve`
     * invocation - see the file header comment). */
    if (opt->log_level == LOG_INFO) opt->log_level = LOG_WARN;

    const char *dir = opt->input ? opt->input : ".";
    if (!path_is_dir(dir)) {
        fprintf(stderr, i18n_tr("salam: serve directory '%s' does not exist\n"), dir);
        return 2;
    }
    const char *host = opt->serve_host ? opt->serve_host : "127.0.0.1";
    int port = opt->serve_port > 0 ? opt->serve_port : 8000;

    sb_t src;
    sb_init(&src);
    sb_puts(&src, "import web\n"
                  "\n"
                  "func main:\n"
                  "    r := web.NewRouter()\n"
                  "    web.EnableListing(r)\n"
                  "    web.Static(r, \"/\", ");
    sb_put_salam_string_literal(&src, dir);
    sb_puts(&src, ")\n    s := web.NewServer(");
    {
        char portbuf[16];
        sal_snprintf(portbuf, sizeof portbuf, "%d", port);
        sb_puts(&src, portbuf);
    }
    sb_puts(&src, ")\n    web.SetHost(s, ");
    sb_put_salam_string_literal(&src, host);
    sb_puts(&src,
            ")\n    web.EnableAccessLog(s)\n    web.Use(s, r)\n    web.Run(s)\nend\n");

    char salam_path[600];
#if defined(_WIN32)
    sal_snprintf(salam_path, sizeof salam_path, "%s\\salam-serve-%lu.salam",
                 salam_scratch_dir(), (unsigned long)_getpid());
#else
    sal_snprintf(salam_path, sizeof salam_path, "%s/salam-serve-%lu.salam",
                 salam_scratch_dir(), (unsigned long)getpid());
#endif
    FILE *f = fopen(salam_path, "wb");
    if (!f) {
        fprintf(stderr, i18n_tr("salam: cannot write '%s'\n"), salam_path);
        sb_free(&src);
        return 2;
    }
    fputs(sb_cstr(&src), f);
    fclose(f);
    sb_free(&src);

    char exe_path[600];
    bool temp_exe = (opt->output == NULL);
    if (temp_exe) {
#if defined(_WIN32)
        sal_snprintf(exe_path, sizeof exe_path, "%s\\salam-serve-%lu.exe",
                     salam_scratch_dir(), (unsigned long)_getpid());
#else
        sal_snprintf(exe_path, sizeof exe_path, "%s/salam-serve-%lu", salam_scratch_dir(),
                     (unsigned long)getpid());
#endif
        opt->output = exe_path;
    }

    opt->command = CMD_BUILD;
    opt->input = salam_path;
    opt->inputs[0] = salam_path;
    opt->input_count = 1;
    opt->keep_c = false;

    int rc = driver_build(opt);
    if (rc != 0) {
        remove(salam_path);
        return rc;
    }

    const char *exe = opt->exe_path[0] ? opt->exe_path : opt->output;
    fprintf(stdout, i18n_tr("salam: serving '%s' at http://%s:%d/ (Ctrl+C to stop)\n"),
            dir, host, port);
    fflush(stdout);

    sb_t cmd;
    sb_init(&cmd);
    sb_put_shell_arg(&cmd, exe);
    int run_rc = system(sb_cstr(&cmd));
    sb_free(&cmd);

    remove(exe);
    remove(salam_path);

#if defined(_WIN32)
    return run_rc;
#else
    return WIFEXITED(run_rc) ? WEXITSTATUS(run_rc) : 1;
#endif
}

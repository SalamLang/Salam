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

#include "core/prelude.h"
#include "core/sal_format.h"
#include "driver/debug_cmd.h"
#include "driver/build.h"
#include "core/sb.h"
#include "logger/logger.h"

static void path_stem(const char *path, char *out, size_t cap)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *base = path;
    if (slash && slash + 1 > base) base = slash + 1;
    if (bslash && bslash + 1 > base) base = bslash + 1;
    const char *dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);
    if (len >= cap) len = cap - 1;
    memcpy(out, base, len);
    out[len] = '\0';
}

int driver_debug(options_t *opt)
{
    if (opt->input_count == 0) {
        fprintf(stderr, "salam debug: no input file\n");
        return 2;
    }

    opt->debug_info = true;
    opt->command = CMD_BUILD;

    char exe[512];
    exe[0] = '\0';
    if (opt->output) {
        sal_snprintf(exe, sizeof(exe), "%s", opt->output);
    } else {
        char stem[256];
        path_stem(opt->inputs[0], stem, sizeof(stem));
        sal_snprintf(exe, sizeof(exe), "%s.exe", stem);
        opt->output = exe;
    }
    int rc = driver_build(opt);
    if (rc != 0) return rc;

    const char *target = opt->exe_path[0] ? opt->exe_path : exe;

#if defined(__APPLE__)

    const char *dbg = "lldb";
    sb_t cmd;
    sb_init(&cmd);
    sb_puts(&cmd, dbg);
    sb_puts(&cmd, " -o 'breakpoint set --name main' -o 'run' -- ");
    sb_put_shell_arg(&cmd, target);
    fprintf(stdout,
            "\nsalam: launching %s\n"
            "  (lldb) breakpoint set --name main  → already set\n"
            "  (lldb) next / step                 → step over / into\n"
            "  (lldb) frame variable <var>        → inspect a variable\n"
            "  (lldb) source list                 → show Salam source (via #line)\n"
            "  (lldb) continue / quit             → resume / exit\n\n",
            dbg);
    fflush(stdout);
    rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
#elif defined(_WIN32)

    const char *dbg = "gdb";
    sb_t cmd;
    sb_init(&cmd);
    sb_puts(&cmd, dbg);
    sb_puts(&cmd, " ");
    sb_put_shell_arg(&cmd, target);
    fprintf(stdout,
            "\nsalam: launching %s\n"
            "  Tip: (gdb) break main   → set a breakpoint\n"
            "       (gdb) run          → start the program\n"
            "       (gdb) next / step  → step over / into\n"
            "       (gdb) print <var>  → inspect a variable\n\n",
            sb_cstr(&cmd));
    fflush(stdout);
    rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
#else

    const char *dbg = "gdb";
    sb_t cmd;
    sb_init(&cmd);
    sb_puts(&cmd, dbg);
    sb_puts(&cmd, " -ex 'break main' -ex 'run' ");
    sb_put_shell_arg(&cmd, target);
    fprintf(stdout,
            "\nsalam: launching %s\n"
            "  (gdb) break main  → already set\n"
            "  (gdb) next / step → step over / into\n"
            "  (gdb) print <var> → inspect a variable\n"
            "  (gdb) list        → show Salam source (via #line)\n\n",
            sb_cstr(&cmd));
    fflush(stdout);
    rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
#endif
    if (rc == 127) {
        fprintf(
            stderr,
            "salam debug: '%s' not found in PATH.\n"
            "  Linux:  sudo apt-get install gdb      (or: sudo apt-get install lldb)\n"
            "  macOS:  xcode-select --install         (ships lldb)\n"
            "          or: brew install llvm           (provides a newer lldb)\n"
            "  Win:    pacman -S mingw-w64-x86_64-gdb (MSYS2)\n",
            dbg);
        return 3;
    }
    return rc == 0 ? 0 : 1;
}

int driver_memcheck(options_t *opt)
{
    if (opt->input_count == 0) {
        fprintf(stderr, "salam memcheck: no input file\n");
        return 2;
    }

    if (strstr(opt->cc, "tcc")) {
        fprintf(stderr,
                "salam memcheck: tcc does not support AddressSanitizer.\n"
                "  Use --cc=gcc or --cc=clang:\n"
                "    salam memcheck %s --cc=gcc\n",
                opt->inputs[0]);
        return 2;
    }
    opt->debug_info = true;
    opt->asan = true;
    opt->command = CMD_BUILD;
    char exe[512];
    exe[0] = '\0';
    if (opt->output) {
        sal_snprintf(exe, sizeof(exe), "%s", opt->output);
    } else {
        char stem[256];
        path_stem(opt->inputs[0], stem, sizeof(stem));
        sal_snprintf(exe, sizeof(exe), "%s.exe", stem);
        opt->output = exe;
    }
    fprintf(stdout,
            "salam memcheck: building with AddressSanitizer + debug symbols...\n");
    fflush(stdout);
    int rc = driver_build(opt);
    if (rc != 0) return rc;
    const char *target = opt->exe_path[0] ? opt->exe_path : exe;
    sb_t cmd;
    sb_init(&cmd);
#if defined(_WIN32)

    fprintf(stdout, "salam memcheck: Valgrind is not available on Windows.\n"
                    "  Running binary with ASAN error reporting...\n\n");
    sb_put_shell_arg(&cmd, target);
#else

    if (system("valgrind --version > /dev/null 2>&1") == 0) {
        fprintf(stdout, "salam memcheck: running under Valgrind...\n\n");
        sb_puts(&cmd, "valgrind --leak-check=full --track-origins=yes "
                      "--show-leak-kinds=all --error-exitcode=1 ");
        sb_put_shell_arg(&cmd, target);
    } else {
        fprintf(stdout,
                "salam memcheck: valgrind not found; running with ASAN error reporting.\n"
                "  (Install: sudo apt-get install valgrind)\n\n");
        sb_put_shell_arg(&cmd, target);
    }
#endif
    fflush(stdout);
    rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    if (rc == 0)
        fprintf(stdout, "\nsalam memcheck: no memory errors detected.\n");
    else
        fprintf(stderr,
                "\nsalam memcheck: errors detected (exit %d). "
                "Review the output above.\n",
                rc);
    return rc == 0 ? 0 : 1;
}

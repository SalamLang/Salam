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
#include "core/sal_path.h"
#include "driver/debug_cmd.h"
#include "driver/build.h"
#include "core/sb.h"
#include "logger/logger.h"

int driver_debug(options_t *opt)
{
    if (opt->input_count == 0) {
        fprintf(stderr, "salam debug: no input file\n");
        return 2;
    }

    opt->debug_info = true;
    if (!opt->safe) {
        opt->safe = true;
        if (opt->ndefines < SALAM_MAX_INPUTS)
            opt->defines[opt->ndefines++] = "SALAM_SAFE";
    }
    opt->command = CMD_BUILD;

    char exe[512];
    exe[0] = '\0';
    if (opt->output) {
        sal_snprintf(exe, sizeof(exe), "%s", opt->output);
    } else {
        char stem[256];
        sal_path_stem_buf(opt->inputs[0], stem, sizeof(stem));
        sal_snprintf(exe, sizeof(exe), "%s%s", stem, driver_exe_suffix(opt->llvm_target));
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

    /* Which checker gets the process is decided here, before the build, and
       exactly one of them does. AddressSanitizer and Valgrind each replace the
       allocator, and an ASan-instrumented binary run under Valgrind aborts on
       startup with "ASan runtime does not come first in initial library list"
       having checked nothing at all - which is what this command used to do
       whenever both were available. Valgrind wins where it exists, because it
       also reports reads of uninitialised memory and ASan does not. */
    bool use_valgrind = false;
#if !defined(_WIN32)
    use_valgrind = system("valgrind --version > /dev/null 2>&1") == 0;
#endif

    /* tcc cannot instrument for ASan, but it emits a perfectly good binary for
       Valgrind to watch, so it is only a hard stop when ASan is the plan. */
    if (!use_valgrind && strstr(opt->cc, "tcc")) {
        fprintf(stderr,
                "salam memcheck: tcc does not support AddressSanitizer, and no\n"
                "  Valgrind was found to check the binary instead.\n"
                "  Use --cc=gcc or --cc=clang:\n"
                "    salam memcheck %s --cc=gcc\n"
                "  or install Valgrind: sudo apt-get install valgrind\n",
                opt->inputs[0]);
        return 2;
    }
    opt->debug_info = true;
    opt->asan = !use_valgrind;
    if (!opt->safe) {
        opt->safe = true;
        if (opt->ndefines < SALAM_MAX_INPUTS)
            opt->defines[opt->ndefines++] = "SALAM_SAFE";
    }
    opt->command = CMD_BUILD;
    char exe[512];
    exe[0] = '\0';
    if (opt->output) {
        sal_snprintf(exe, sizeof(exe), "%s", opt->output);
    } else {
        char stem[256];
        sal_path_stem_buf(opt->inputs[0], stem, sizeof(stem));
        sal_snprintf(exe, sizeof(exe), "%s%s", stem, driver_exe_suffix(opt->llvm_target));
        opt->output = exe;
    }
    fprintf(stdout,
            use_valgrind
                ? "salam memcheck: building with debug symbols for Valgrind...\n"
                : "salam memcheck: building with AddressSanitizer + debug symbols...\n");
    fflush(stdout);
    int rc = driver_build(opt);
    if (rc != 0) return rc;
    const char *built = opt->exe_path[0] ? opt->exe_path : exe;
    /* Spell a bare name as "./name" before handing it to the shell. Without
       the prefix the shell looks the binary up on $PATH, which does not
       include the current directory on POSIX, so a build that landed in the
       working directory fails with "valgrind: app: command not found" rather
       than running. driver_run guards the same way; cmd.exe searches the
       working directory itself, so Windows needs nothing. */
    char target[600];
#if defined(_WIN32)
    sal_snprintf(target, sizeof target, "%s", built);
#else
    if (strchr(built, '/'))
        sal_snprintf(target, sizeof target, "%s", built);
    else
        sal_snprintf(target, sizeof target, "./%s", built);
#endif
    sb_t cmd;
    sb_init(&cmd);
    if (use_valgrind) {
        fprintf(stdout, "salam memcheck: running under Valgrind...\n\n");
        sb_puts(&cmd, "valgrind --leak-check=full --track-origins=yes "
                      "--show-leak-kinds=all --error-exitcode=1 ");
        sb_put_shell_arg(&cmd, target);
    } else {
#if defined(_WIN32)
        fprintf(stdout, "salam memcheck: Valgrind is not available on Windows.\n"
                        "  Running binary with ASAN error reporting...\n\n");
#else
        fprintf(stdout,
                "salam memcheck: valgrind not found; running with ASAN error reporting.\n"
                "  (Install: sudo apt-get install valgrind)\n\n");
#endif
        sb_put_shell_arg(&cmd, target);
    }
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

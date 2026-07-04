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
#include "cli/cli_internal.h"

int cli_dispatch_command(int argc, char **argv, options_t *out)
{
    int start = 1;
    const char *cmd = argc > 1 ? argv[1] : NULL;
    if (cmd == NULL) {
        out->command = CMD_HELP;
    } else if (strcmp(cmd, "build") == 0) {
        out->command = CMD_BUILD;
        start = 2;
    } else if (strcmp(cmd, "obj") == 0) {
        out->command = CMD_OBJ;
        start = 2;
    } else if (strcmp(cmd, "llvm") == 0 || strcmp(cmd, "emit-llvm") == 0) {
        out->command = CMD_LLVM;
        start = 2;
    } else if (strcmp(cmd, "run") == 0) {
        out->command = CMD_RUN;
        start = 2;
    } else if (strcmp(cmd, "exec") == 0 || strcmp(cmd, "jit") == 0) {
        out->command = CMD_RUN;
        out->interp = true;
        start = 2;
    } else if (strcmp(cmd, "debug") == 0) {
        out->command = CMD_DEBUG;
        out->debug_info = true;
        start = 2;
    } else if (strcmp(cmd, "memcheck") == 0) {
        out->command = CMD_MEMCHECK;
        out->debug_info = true;
        out->asan = true;
        start = 2;
    } else if (strcmp(cmd, "new") == 0) {
        out->command = CMD_NEW;
        start = 2;
    } else if (strcmp(cmd, "fmt") == 0 || strcmp(cmd, "format") == 0) {
        out->command = CMD_FMT;
        start = 2;
    } else if (strcmp(cmd, "help") == 0) {
        out->command = CMD_HELP;
        start = 2;
    } else if (strcmp(cmd, "version") == 0) {
        out->command = CMD_VERSION;
        start = 2;
    } else if (strcmp(cmd, "layout") == 0) {
        if (argc > 2 && strcmp(argv[2], "build") == 0) {
            out->command = CMD_LAYOUT_BUILD;
            start = 3;
        } else {
            out->command = CMD_REPL_LAYOUT;
            start = 2;
        }
    } else if (strcmp(cmd, "cli") == 0) {
        if (argc > 2 && strcmp(argv[2], "build") == 0) {
            out->command = CMD_BUILD;
            start = 3;
        } else if (argc > 2 && strcmp(argv[2], "obj") == 0) {
            out->command = CMD_OBJ;
            start = 3;
        } else {
            out->command = CMD_REPL;
            start = 2;
        }
    }

    return start;
}

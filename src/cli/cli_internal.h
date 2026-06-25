#ifndef SALAM_CLI_INTERNAL_H
#define SALAM_CLI_INTERNAL_H

#include "core/prelude.h"
#include "cli/options.h"

int cli_dispatch_command(int argc, char **argv, options_t *out);

bool cli_parse_options(int argc, char **argv, int start, options_t *out);

const char *cli_opt_value(const char *arg, const char *key);

#endif /* SALAM_CLI_INTERNAL_H */

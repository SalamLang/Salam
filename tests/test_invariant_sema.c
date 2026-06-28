#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "core/arena.h"
#include "semantic/sema.h"

#ifndef _WIN32
#  include <sys/stat.h>
#  include <unistd.h>
#endif

static int test_path_traversal_containment(void)
{
    const char *payloads[] = {
        "../../../etc/passwd.salam",
        "....//....//etc/passwd.salam",
        "subdir/../../rootfile.salam",
    };
    int num_payloads = (int)(sizeof(payloads) / sizeof(payloads[0]));

#ifndef _WIN32
    char test_root[PATH_MAX];
    strncpy(test_root, "/tmp/test_root_XXXXXX", sizeof(test_root) - 1);
    test_root[sizeof(test_root) - 1] = '\0';
    if (!mkdtemp(test_root)) return 1;

    char safe_dir[PATH_MAX];
    if (snprintf(safe_dir, sizeof(safe_dir), "%s/safe", test_root) < 0) {
        rmdir(test_root);
        return 1;
    }
    if (mkdir(safe_dir, 0755) != 0) {
        rmdir(test_root);
        return 1;
    }
#else
    const char *safe_dir = "C:\\Temp\\test_safe";
    const char *test_root = "C:\\Temp";
#endif

    arena_t *a = arena_new(4096);
    int failures = 0;

    int i;
    for (i = 0; i < num_payloads; i++) {
        const char *result = salam_resolve_import(a, safe_dir, payloads[i]);
        if (result && result[0] != '\0') {
            if (strncmp(result, test_root, strlen(test_root)) != 0) {
                failures++;
            }
        }
    }

    const char *valid = salam_resolve_import(a, safe_dir, "valid/file.salam");
    if (!valid || valid[0] == '\0') failures++;

#ifndef _WIN32
    rmdir(safe_dir);
    rmdir(test_root);
#endif

    return failures;
}

int main(void)
{
    int failed = test_path_traversal_containment();
    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

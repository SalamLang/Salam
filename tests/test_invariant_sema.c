#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "core/arena.h"
#include "semantic/sema.h"

#ifndef _WIN32
#  include <sys/stat.h>
#  include <unistd.h>
#endif

static int path_contained(const char *path, const char *dir)
{
    size_t dlen = strlen(dir);
    while (dlen > 0 && (dir[dlen-1] == '/' || dir[dlen-1] == '\\')) dlen--;
    if (strlen(path) < dlen) return 0;
    size_t i;
    for (i = 0; i < dlen; i++) {
        char dc = (dir[i]  == '\\') ? '/' : dir[i];
        char pc = (path[i] == '\\') ? '/' : path[i];
        if (dc != pc) return 0;
    }
    char next = (path[dlen] == '\\') ? '/' : path[dlen];
    return (next == '/' || next == '\0');
}

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
    int sret = snprintf(safe_dir, sizeof(safe_dir), "%s/safe", test_root);
    if (sret < 0 || sret >= (int)sizeof(safe_dir)) {
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
    (void)test_root;
#endif

    arena_t *a = arena_new(4096);
    int failures = 0;

    int i;
    for (i = 0; i < num_payloads; i++) {
        const char *result = salam_resolve_import(a, safe_dir, payloads[i]);
        if (result && result[0] != '\0') {
            if (!path_contained(result, safe_dir))
                failures++;
        }
    }

    const char *valid = salam_resolve_import(a, safe_dir, "valid/file.salam");
    if (!valid || valid[0] == '\0') {
        failures++;
    } else {
        if (!path_contained(valid, safe_dir))
            failures++;
    }

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

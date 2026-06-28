#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

// Include the actual production header
#include "compiler/src/semantic/sema.h"

START_TEST(test_path_traversal_containment)
{
    // Invariant: Path resolution must never escape the declared root directory
    const char *payloads[] = {
        "../../../etc/passwd",           // Classic traversal
        "....//....//etc/passwd",        // Double dot obfuscation
        "valid/file.txt",                // Valid input (should pass)
        "%2e%2e%2fetc%2fpasswd",         // URL encoded traversal
        "subdir/../../rootfile"          // Mixed traversal
    };
    int num_payloads = sizeof(payloads) / sizeof(payloads[0]);
    
    // Create a test root directory
    char test_root[PATH_MAX];
    strcpy(test_root, "/tmp/test_root_XXXXXX");
    mkdtemp(test_root);
    
    // Create expected safe subdirectory
    char safe_dir[PATH_MAX];
    snprintf(safe_dir, sizeof(safe_dir), "%s/safe", test_root);
    mkdir(safe_dir, 0755);
    
    for (int i = 0; i < num_payloads; i++) {
        char resolved[PATH_MAX];
        char full_path[PATH_MAX];
        
        // Call the actual production function
        int result = salam_resolve_import(payloads[i], safe_dir, resolved, sizeof(resolved));
        
        if (result == 0) {
            // Path was resolved - verify it stays within test_root
            realpath(resolved, full_path);
            ck_assert_msg(strncmp(full_path, test_root, strlen(test_root)) == 0,
                         "Path traversal vulnerability: %s resolved to %s which escapes %s",
                         payloads[i], full_path, test_root);
        } else {
            // Resolution failed - this is acceptable for malicious payloads
            // For valid input we expect success
            if (strcmp(payloads[i], "valid/file.txt") == 0) {
                ck_abort_msg("Valid path '%s' was rejected", payloads[i]);
            }
        }
    }
    
    // Cleanup
    rmdir(safe_dir);
    rmdir(test_root);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_path_traversal_containment);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
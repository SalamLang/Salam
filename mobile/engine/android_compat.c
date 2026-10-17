#include <errno.h>
#include <unistd.h>

__attribute__((weak)) int posix_spawnp(int *pid, const char *file, const void *file_actions, const void *attrp,
                                       char *const argv[], char *const envp[]) {
    (void)file_actions;
    (void)attrp;
    (void)envp;
    pid_t child = fork();
    if (child < 0) {
        return errno;
    }
    if (child == 0) {
        execvp(file, argv);
        _exit(127);
    }
    if (pid != NULL) {
        *pid = child;
    }
    return 0;
}

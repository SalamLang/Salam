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

#include "interp/interp_internal.h"

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#  include <process.h>
#  include <windows.h>
#else
#  include <dirent.h>
#  include <sys/stat.h>
#  include <time.h>
#  include <unistd.h>
#endif

typedef int64_t (*native_fn8_t)(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t,
                                int64_t, int64_t);

/*
 * Every native symbol below is called through the single uniform
 * native_fn8_t signature (see call_native_extern). Casting the *real*
 * libc function pointers (malloc, realloc, fopen, ...) to that shape and
 * calling through them is undefined behavior: it happens to work on
 * native targets because their ABI passes integer args in registers
 * regardless of the declared signature, but under WebAssembly an
 * indirect call is type-checked against the callee's actual function
 * type, so a real 1- or 2-argument libc export called via an 8-argument
 * pointer traps with "null function or function signature mismatch".
 * These trampolines are each genuinely declared with the native_fn8_t
 * shape, so the indirect call site and the callee's real wasm type
 * always agree.
 */
static int64_t nf_getenv(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)getenv((const char *)(intptr_t)a0);
}
static int64_t nf_malloc(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)malloc((size_t)a0);
}
static int64_t nf_realloc(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)realloc((void *)(intptr_t)a0, (size_t)a1);
}
static int64_t nf_free(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                       int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    free((void *)(intptr_t)a0);
    return 0;
}
static int64_t nf_memset(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)memset((void *)(intptr_t)a0, (int)a1, (size_t)a2);
}
static int64_t nf_memcpy(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)memcpy((void *)(intptr_t)a0, (const void *)(intptr_t)a1,
                                     (size_t)a2);
}
static int64_t nf_memmove(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)memmove((void *)(intptr_t)a0, (const void *)(intptr_t)a1,
                                      (size_t)a2);
}
static int64_t nf_remove(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)remove((const char *)(intptr_t)a0);
}
static int64_t nf_rename(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)rename((const char *)(intptr_t)a0, (const char *)(intptr_t)a1);
}
static int64_t nf_fopen(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)fopen((const char *)(intptr_t)a0,
                                    (const char *)(intptr_t)a1);
}
static int64_t nf_fclose(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fclose((FILE *)(intptr_t)a0);
}
static int64_t nf_fseek(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fseek((FILE *)(intptr_t)a0, (long)a1, (int)a2);
}
static int64_t nf_ftell(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)ftell((FILE *)(intptr_t)a0);
}
static int64_t nf_fread(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fread((void *)(intptr_t)a0, (size_t)a1, (size_t)a2,
                          (FILE *)(intptr_t)a3);
}
static int64_t nf_fwrite(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fwrite((const void *)(intptr_t)a0, (size_t)a1, (size_t)a2,
                           (FILE *)(intptr_t)a3);
}
static int64_t nf_fgetc(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fgetc((FILE *)(intptr_t)a0);
}
static int64_t nf_fgets(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)fgets((char *)(intptr_t)a0, (int)a1, (FILE *)(intptr_t)a2);
}
static int64_t nf_exit(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                       int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    exit((int)a0);
    return 0;
}
static int64_t nf_abort(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    abort();
    return 0;
}
#ifdef _WIN32
static int64_t nf__mkdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_mkdir((const char *)(intptr_t)a0);
}
static int64_t nf__rmdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_rmdir((const char *)(intptr_t)a0);
}
static int64_t nf__chdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_chdir((const char *)(intptr_t)a0);
}
static int64_t nf_GetCurrentDirectoryA(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                       int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)GetCurrentDirectoryA((DWORD)a0, (LPSTR)(intptr_t)a1);
}
static int64_t nf_GetFileAttributesA(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                     int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)GetFileAttributesA((LPCSTR)(intptr_t)a0);
}
static int64_t nf__fseeki64(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                            int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_fseeki64((FILE *)(intptr_t)a0, (__int64)a1, (int)a2);
}
static int64_t nf_FindFirstFileA(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                 int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)FindFirstFileA((LPCSTR)(intptr_t)a0,
                                             (LPWIN32_FIND_DATAA)(intptr_t)a1);
}
static int64_t nf_FindNextFileA(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)FindNextFileA((HANDLE)(intptr_t)a0, (LPWIN32_FIND_DATAA)(intptr_t)a1);
}
static int64_t nf_FindClose(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                            int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)FindClose((HANDLE)(intptr_t)a0);
}
static int64_t nf_GetSystemTimeAsFileTime(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                          int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    GetSystemTimeAsFileTime((LPFILETIME)(intptr_t)a0);
    return 0;
}
static int64_t nf__getpid(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_getpid();
}
static int64_t nf__popen(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)_popen((const char *)(intptr_t)a0,
                                     (const char *)(intptr_t)a1);
}
static int64_t nf__pclose(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)_pclose((FILE *)(intptr_t)a0);
}
#else
static int64_t nf_mkdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)mkdir((const char *)(intptr_t)a0, (mode_t)a1);
}
static int64_t nf_rmdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)rmdir((const char *)(intptr_t)a0);
}
static int64_t nf_chdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)chdir((const char *)(intptr_t)a0);
}
static int64_t nf_access(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)access((const char *)(intptr_t)a0, (int)a1);
}
static int64_t nf_getcwd(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)getcwd((char *)(intptr_t)a0, (size_t)a1);
}
static int64_t nf_opendir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)opendir((const char *)(intptr_t)a0);
}
static int64_t nf_readdir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                          int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)readdir((DIR *)(intptr_t)a0);
}
static int64_t nf_closedir(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                           int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)closedir((DIR *)(intptr_t)a0);
}
static int64_t nf_fseeko(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)fseeko((FILE *)(intptr_t)a0, (off_t)a1, (int)a2);
}
static int64_t nf_clock_gettime(int64_t a0, int64_t a1, int64_t a2, int64_t a3,
                                int64_t a4, int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)clock_gettime((clockid_t)a0, (struct timespec *)(intptr_t)a1);
}
static int64_t nf_popen(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                        int64_t a5, int64_t a6, int64_t a7)
{
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)(intptr_t)popen((const char *)(intptr_t)a0,
                                    (const char *)(intptr_t)a1);
}
static int64_t nf_pclose(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)pclose((FILE *)(intptr_t)a0);
}
static int64_t nf_getpid(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a0;
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)getpid();
}
static int64_t nf_system(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4,
                         int64_t a5, int64_t a6, int64_t a7)
{
    (void)a1;
    (void)a2;
    (void)a3;
    (void)a4;
    (void)a5;
    (void)a6;
    (void)a7;
    return (int64_t)system((const char *)(intptr_t)a0);
}
#endif

typedef struct {
    const char *name;
    native_fn8_t fnptr;
} native_sym_t;

static const native_sym_t NATIVE_SYMS[] = {
    {"getenv", nf_getenv},
    {"malloc", nf_malloc},
    {"realloc", nf_realloc},
    {"free", nf_free},
    {"memset", nf_memset},
    {"memcpy", nf_memcpy},
    {"memmove", nf_memmove},
    {"remove", nf_remove},
    {"rename", nf_rename},
    {"fopen", nf_fopen},
    {"fclose", nf_fclose},
    {"fseek", nf_fseek},
    {"ftell", nf_ftell},
    {"fread", nf_fread},
    {"fwrite", nf_fwrite},
    {"fgetc", nf_fgetc},
    {"fgets", nf_fgets},
    {"exit", nf_exit},
    {"abort", nf_abort},
#ifdef _WIN32
    {"_mkdir", nf__mkdir},
    {"_rmdir", nf__rmdir},
    {"_chdir", nf__chdir},
    {"GetCurrentDirectoryA", nf_GetCurrentDirectoryA},
    {"GetFileAttributesA", nf_GetFileAttributesA},
    {"_fseeki64", nf__fseeki64},
    {"FindFirstFileA", nf_FindFirstFileA},
    {"FindNextFileA", nf_FindNextFileA},
    {"FindClose", nf_FindClose},
    {"GetSystemTimeAsFileTime", nf_GetSystemTimeAsFileTime},
    {"_getpid", nf__getpid},
    {"_popen", nf__popen},
    {"_pclose", nf__pclose},
#else
    {"mkdir", nf_mkdir},     {"rmdir", nf_rmdir},
    {"chdir", nf_chdir},     {"access", nf_access},
    {"getcwd", nf_getcwd},   {"opendir", nf_opendir},
    {"readdir", nf_readdir}, {"closedir", nf_closedir},
    {"fseeko", nf_fseeko},   {"clock_gettime", nf_clock_gettime},
    {"popen", nf_popen},     {"pclose", nf_pclose},
    {"getpid", nf_getpid},   {"system", nf_system},
#endif
};

static native_fn8_t find_native_sym(const char *name)
{
    size_t i = 0;
    for (; i < sizeof NATIVE_SYMS / sizeof NATIVE_SYMS[0]; i++)
        if (!strcmp(NATIVE_SYMS[i].name, name)) return NATIVE_SYMS[i].fnptr;
    return NULL;
}

static bool typestr_is_float(const char *ts)
{
    char base[96];
    base_typename(ts, base, sizeof base);
    return is_float_typename(base);
}

static int64_t marshal_arg(value_t v)
{
    switch (v.kind) {
    case VAL_STR:
        return (int64_t)(intptr_t)(v.as.s ? v.as.s : "");
    case VAL_PTR:
        return (int64_t)(intptr_t)v.as.ptr.addr;
    case VAL_BOOL:
        return v.as.b ? 1 : 0;
    case VAL_CHAR:
    case VAL_INT:
        return v.as.i;
    default:
        return 0;
    }
}

value_t call_native_extern(interp_t *I, ast_node_t *call, ast_node_t *decl, value_t *args,
                           size_t nargs)
{
    if (decl->is_variadic)
        rt_error(I, call,
                 "extern function '%s' is variadic, which the interpreter does not "
                 "support (use 'salam run' instead)",
                 decl->name);
    if (decl->list.len > 8)
        rt_error(I, call,
                 "extern function '%s' takes too many parameters for the "
                 "interpreter's native call support",
                 decl->name);
    {
        size_t i = 0;
        for (; i < decl->list.len; i++) {
            ast_node_t *p = (ast_node_t *)decl->list.data[i];
            if (typestr_is_float(p->type ? p->type->type_str : NULL))
                rt_error(I, call,
                         "extern function '%s' takes a floating-point argument, which "
                         "the interpreter does not support (use 'salam run' instead)",
                         decl->name);
        }
    }
    const char *rts = decl->type ? decl->type->type_str : NULL;
    if (rts && typestr_is_float(rts))
        rt_error(I, call,
                 "extern function '%s' returns a floating-point value, which the "
                 "interpreter does not support (use 'salam run' instead)",
                 decl->name);

    native_fn8_t f = find_native_sym(decl->name);
    if (!f)
        rt_error(I, call,
                 "extern function '%s' is not supported by the interpreter (use "
                 "'salam run' instead)",
                 decl->name);

    int64_t a[8] = {0};
    {
        size_t i = 0;
        for (; i < decl->list.len && i < nargs; i++)
            a[i] = marshal_arg(args[i]);
    }
    int64_t r = f(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);

    if (!rts) return val_null();
    if (rts[strlen(rts) - 1] == '*')
        return val_ptr((void *)(intptr_t)r, ptr_elem_from_typestr(rts));
    char rbase[96];
    base_typename(rts, rbase, sizeof rbase);
    if (!strcmp(rbase, "bool")) return val_bool(r != 0);
    /* Tag the result with the declared return type so an extern returning
     * u32/u64 does not read back as a negative i64. */
    return val_int_ty(r, int_ty_from_typestr(rbase));
}

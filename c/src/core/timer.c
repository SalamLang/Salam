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

#include "core/timer.h"
#include <time.h>

#if defined(_WIN32)
#  include <windows.h>
#else
#  include <sys/time.h>
#  include <sys/resource.h>
#  if defined(__APPLE__)
#    include <mach/mach_time.h>
#  endif
#endif

#if defined(_WIN32)

/* QueryPerformanceFrequency is fixed for the lifetime of the system, so it is
 * queried once. 0 means "not yet queried". */
static LONGLONG g_qpc_freq = 0;

uint64_t salam_mono_ns(void)
{
    LARGE_INTEGER c;
    LONGLONG whole;
    LONGLONG frac;
    if (g_qpc_freq == 0) {
        LARGE_INTEGER f;
        if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) {
            /* No QPC at all (not possible on anything we support, but be
             * safe): fall back to the tick counter's millisecond resolution. */
            return (uint64_t)GetTickCount() * 1000000ULL;
        }
        g_qpc_freq = f.QuadPart;
    }
    if (!QueryPerformanceCounter(&c)) return 0;
    /* Split the conversion so it does not overflow. c.QuadPart * 1e9 wraps a
     * signed 64-bit value after a few seconds of uptime at a 10 MHz QPC. */
    whole = c.QuadPart / g_qpc_freq;
    frac = c.QuadPart % g_qpc_freq;
    return (uint64_t)whole * 1000000000ULL +
           (uint64_t)((frac * 1000000000LL) / g_qpc_freq);
}

uint64_t salam_cpu_ns(void)
{
    FILETIME creation;
    FILETIME exit_t;
    FILETIME kernel;
    FILETIME user;
    ULARGE_INTEGER k;
    ULARGE_INTEGER u;
    if (!GetProcessTimes(GetCurrentProcess(), &creation, &exit_t, &kernel, &user))
        return 0;
    k.LowPart = kernel.dwLowDateTime;
    k.HighPart = kernel.dwHighDateTime;
    u.LowPart = user.dwLowDateTime;
    u.HighPart = user.dwHighDateTime;
    /* FILETIME ticks are 100 ns. */
    return (uint64_t)(k.QuadPart + u.QuadPart) * 100ULL;
}

/* Layout of PROCESS_MEMORY_COUNTERS, redeclared so this file does not need
 * psapi.h (tcc's headers do not always carry it). The function is resolved at
 * runtime so the compiler never has to link against psapi. */
typedef struct {
    DWORD cb;
    DWORD PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} salam_pmc_t;

typedef BOOL(WINAPI *salam_gpmi_fn)(HANDLE, salam_pmc_t *, DWORD);

uint64_t salam_peak_rss(void)
{
    /* -1 means "tried and failed", 0 means "not tried yet". */
    static salam_gpmi_fn fn = NULL;
    static int tried = 0;
    salam_pmc_t pmc;
    if (!tried) {
        HMODULE h;
        tried = 1;
        /* Windows 7+ exports K32GetProcessMemoryInfo straight from kernel32,
         * which avoids loading psapi.dll at all. Fall back to psapi.dll for
         * older systems. */
        h = GetModuleHandleA("kernel32.dll");
        if (h) fn = (salam_gpmi_fn)(void *)GetProcAddress(h, "K32GetProcessMemoryInfo");
        if (!fn) {
            h = LoadLibraryA("psapi.dll");
            if (h) fn = (salam_gpmi_fn)(void *)GetProcAddress(h, "GetProcessMemoryInfo");
        }
    }
    if (!fn) return 0;
    memset(&pmc, 0, sizeof pmc);
    pmc.cb = (DWORD)sizeof pmc;
    if (!fn(GetCurrentProcess(), &pmc, (DWORD)sizeof pmc)) return 0;
    return (uint64_t)pmc.PeakWorkingSetSize;
}

#else /* POSIX */

uint64_t salam_mono_ns(void)
{
#  if defined(CLOCK_MONOTONIC)
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#  endif
#  if defined(__APPLE__)
    {
        /* Pre-10.12 macOS has no clock_gettime. */
        static mach_timebase_info_data_t tb;
        if (tb.denom == 0) mach_timebase_info(&tb);
        if (tb.denom != 0) return (uint64_t)mach_absolute_time() * tb.numer / tb.denom;
    }
#  endif
    {
        /* Last resort. gettimeofday is a wall clock, so elapsed times can go
         * backwards across an NTP step; nothing better is available here. */
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == 0)
            return (uint64_t)tv.tv_sec * 1000000000ULL + (uint64_t)tv.tv_usec * 1000ULL;
    }
    return 0;
}

uint64_t salam_cpu_ns(void)
{
#  if defined(CLOCK_PROCESS_CPUTIME_ID)
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == 0)
            return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
    }
#  endif
    {
        struct rusage ru;
        if (getrusage(RUSAGE_SELF, &ru) == 0)
            return ((uint64_t)ru.ru_utime.tv_sec + (uint64_t)ru.ru_stime.tv_sec) *
                       1000000000ULL +
                   ((uint64_t)ru.ru_utime.tv_usec + (uint64_t)ru.ru_stime.tv_usec) *
                       1000ULL;
    }
    return 0;
}

uint64_t salam_peak_rss(void)
{
    struct rusage ru;
    if (getrusage(RUSAGE_SELF, &ru) != 0) return 0;
#  if defined(__APPLE__)
    /* macOS reports ru_maxrss in bytes; every other Unix uses kilobytes. */
    return (uint64_t)ru.ru_maxrss;
#  else
    return (uint64_t)ru.ru_maxrss * 1024ULL;
#  endif
}

#endif /* _WIN32 */

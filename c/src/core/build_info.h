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

#ifndef SALAM_BUILD_INFO_H
#define SALAM_BUILD_INFO_H

/*
 * SALAM_VERSION is stamped by the build from the repo's VERSION file, which
 * is the one place the release number lives. Nothing here duplicates it: a
 * second copy in this header went stale every release and made `salam
 * version` under-report the build. The fallback below is deliberately not a
 * plausible release number, so an unstamped binary is obvious rather than
 * quietly wrong.
 */
#ifndef SALAM_VERSION
#  define SALAM_VERSION "0.0.0-dev"
#endif

#ifndef SALAM_GIT_COMMIT
#  define SALAM_GIT_COMMIT "unknown"
#endif

#ifndef SALAM_GIT_DATE
#  define SALAM_GIT_DATE "unknown"
#endif

#ifndef SALAM_GIT_DIRTY
#  define SALAM_GIT_DIRTY ""
#endif

#define SALAM_BUILD_DATE __DATE__ " " __TIME__

#endif /* SALAM_BUILD_INFO_H */

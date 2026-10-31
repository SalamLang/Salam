#include "salam_engine.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int salam_engine_main(int argc, char **argv);
extern void salam_m_init(const char *std_root);
extern int32_t salam_m_cli(const char *args);
extern void salam_m_stop(void);
extern void salam_m_clear_stop(void);
extern const char *salam_m_tokens(const char *source, const char *lang);
extern const char *salam_m_check(const char *source, const char *path, const char *lang);
extern const char *salam_m_detect(const char *source);
extern const char *salam_m_version(void);

static int booted;
static char boot_arg0[] = "salam";
static char *boot_argv[] = {boot_arg0, NULL};

int salam_engine_boot(const char *std_root) {
    if (!booted) {
        salam_engine_main(1, boot_argv);
        booted = 1;
    }
    salam_m_init(std_root ? std_root : "");
    return 0;
}

int salam_engine_cli(const char *args) {
    return (int)salam_m_cli(args ? args : "");
}

void salam_engine_stop(void) {
    salam_m_stop();
}

void salam_engine_clear_stop(void) {
    salam_m_clear_stop();
}

char *salam_engine_tokens(const char *source, const char *lang) {
    return (char *)salam_m_tokens(source ? source : "", lang ? lang : "");
}

char *salam_engine_check(const char *source, const char *path, const char *lang) {
    return (char *)salam_m_check(source ? source : "", path ? path : "", lang ? lang : "");
}

char *salam_engine_detect(const char *source) {
    return (char *)salam_m_detect(source ? source : "");
}

char *salam_engine_version(void) {
    return (char *)salam_m_version();
}

void salam_engine_free(char *p) {
    free(p);
}

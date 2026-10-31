#ifndef SALAM_ENGINE_H
#define SALAM_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

int salam_engine_boot(const char *std_root);
int salam_engine_cli(const char *args);
void salam_engine_stop(void);
void salam_engine_clear_stop(void);
char *salam_engine_tokens(const char *source, const char *lang);
char *salam_engine_check(const char *source, const char *path, const char *lang);
char *salam_engine_detect(const char *source);
char *salam_engine_version(void);
void salam_engine_free(char *p);

#ifdef __cplusplus
}
#endif

#endif

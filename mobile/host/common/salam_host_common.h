#ifndef SALAM_HOST_COMMON_H
#define SALAM_HOST_COMMON_H

#define SALAM_HOST_NV 8
#define SALAM_HOST_TCAP 512

enum {
    SALAM_EV_RESIZE = 1,
    SALAM_EV_POINTER = 2,
    SALAM_EV_KEY = 3,
    SALAM_EV_TEXT = 4,
    SALAM_EV_INSETS = 5,
    SALAM_EV_LIFECYCLE = 6,
    SALAM_EV_SCHEME = 8,
    SALAM_EV_PREEDIT = 9,
    SALAM_EV_DEVICE = 10
};

void salam_host_post(double kind, double a, double b, double c, double d, double e, const char *text);
void salam_host_set_surface(int w, int h, double density);

int salam_host_poll(double *out);
const char *salam_host_text(void);
void salam_host_wait(int ms);
void salam_host_wake(void);
int salam_host_ready(void);
int salam_host_width(void);
int salam_host_height(void);
double salam_host_density(void);

#endif

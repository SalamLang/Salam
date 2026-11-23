#include "salam_host_common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QCAP 512

typedef struct {
    double v[SALAM_HOST_NV];
    char text[SALAM_HOST_TCAP];
} Ev;

static Ev q[QCAP];
static int qh;
static int qt;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int woken;
static int surf_w;
static int surf_h;
static double surf_density = 1.0;
static char cur_text[SALAM_HOST_TCAP];

void salam_host_post(double kind, double a, double b, double c, double d, double e, const char *text)
{
    Ev ev;
    memset(&ev, 0, sizeof ev);
    ev.v[0] = kind;
    ev.v[1] = a;
    ev.v[2] = b;
    ev.v[3] = c;
    ev.v[4] = d;
    ev.v[5] = e;
    if (text != NULL) {
        strncpy(ev.text, text, SALAM_HOST_TCAP - 1);
    }
    pthread_mutex_lock(&mu);
    int n = (qt + 1) % QCAP;
    if (n == qh) {
        pthread_mutex_unlock(&mu);
        return;
    }
    if (kind == SALAM_EV_POINTER && a == 2 && qh != qt) {
        Ev *last = &q[(qt + QCAP - 1) % QCAP];
        if (last->v[0] == SALAM_EV_POINTER && last->v[1] == 2 && last->v[2] == b) {
            *last = ev;
            pthread_cond_signal(&cv);
            pthread_mutex_unlock(&mu);
            return;
        }
    }
    q[qt] = ev;
    qt = n;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mu);
}

void salam_host_set_surface(int w, int h, double density)
{
    pthread_mutex_lock(&mu);
    surf_w = w;
    surf_h = h;
    if (density > 0) {
        surf_density = density;
    }
    pthread_mutex_unlock(&mu);
}

int salam_host_poll(double *out)
{
    int kind = 0;
    pthread_mutex_lock(&mu);
    if (qh != qt) {
        memcpy(out, &q[qh].v[1], sizeof(double) * (SALAM_HOST_NV - 1));
        memcpy(cur_text, q[qh].text, SALAM_HOST_TCAP);
        kind = (int)q[qh].v[0];
        qh = (qh + 1) % QCAP;
    }
    pthread_mutex_unlock(&mu);
    return kind;
}

const char *salam_host_text(void)
{
    return cur_text;
}

void salam_host_wait(int ms)
{
    pthread_mutex_lock(&mu);
    if (qh == qt && !woken && ms != 0) {
        if (ms < 0) {
            pthread_cond_wait(&cv, &mu);
        } else {
            struct timespec t;
            clock_gettime(CLOCK_REALTIME, &t);
            t.tv_sec += ms / 1000;
            t.tv_nsec += (long)(ms % 1000) * 1000000L;
            if (t.tv_nsec >= 1000000000L) {
                t.tv_sec++;
                t.tv_nsec -= 1000000000L;
            }
            pthread_cond_timedwait(&cv, &mu, &t);
        }
    }
    woken = 0;
    pthread_mutex_unlock(&mu);
}

void salam_host_wake(void)
{
    pthread_mutex_lock(&mu);
    woken = 1;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mu);
}

int salam_host_ready(void)
{
    return surf_w > 0 && surf_h > 0;
}

int salam_host_width(void)
{
    return surf_w;
}

int salam_host_height(void)
{
    return surf_h;
}

double salam_host_density(void)
{
    return surf_density;
}

#define SCRIPT_CAP 4096

static char script_text[SCRIPT_CAP];
static int script_pending;
static int script_used;
static struct timespec script_t0;
static char script_marked[SALAM_HOST_TCAP];

static void script_at(double ms)
{
    struct timespec t = script_t0;
    long long ns = (long long)(ms * 1000000.0);
    t.tv_sec += (time_t)(ns / 1000000000LL);
    t.tv_nsec += (long)(ns % 1000000000LL);
    if (t.tv_nsec >= 1000000000L) {
        t.tv_sec++;
        t.tv_nsec -= 1000000000L;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
}

static void script_pointer(double ms, int action, int id, double x, double y)
{
    double d = salam_host_density();
    script_at(ms);
    salam_host_post(SALAM_EV_POINTER, action, id, x * d, y * d, 0, NULL);
}

static void script_post_text(double ms, int kind, int cursor, const char *s)
{
    script_at(ms);
    salam_host_post(kind, cursor, 0, 0, 0, 0, s);
}

static int script_nums(const char *arg, double *out, int max)
{
    int n = 0;
    const char *p = arg;
    while (n < max && *p != 0) {
        char *end;
        double v = strtod(p, &end);
        if (end == p) {
            break;
        }
        out[n++] = v;
        p = end;
        if (*p == ',') {
            p++;
        }
    }
    return n;
}

static void script_step(const char *cmd, const char *arg, double *t)
{
    double a[6];
    int n = script_nums(arg, a, 6);
    if (strcmp(cmd, "tap") == 0 && n >= 2) {
        script_pointer(*t, 0, 0, a[0], a[1]);
        script_pointer(*t + 40, 1, 0, a[0], a[1]);
        *t += 200;
    } else if (strcmp(cmd, "long") == 0 && n >= 2) {
        double hold = n >= 3 ? a[2] : 900;
        script_pointer(*t, 0, 0, a[0], a[1]);
        script_pointer(*t + hold, 1, 0, a[0], a[1]);
        *t += hold + 200;
    } else if (strcmp(cmd, "swipe") == 0 && n >= 4) {
        double dur = n >= 5 ? a[4] : 300;
        script_pointer(*t, 0, 0, a[0], a[1]);
        for (int i = 1; i <= 20; i++) {
            double f = i / 20.0;
            script_pointer(*t + dur * f, 2, 0, a[0] + (a[2] - a[0]) * f, a[1] + (a[3] - a[1]) * f);
        }
        script_pointer(*t + dur + 16, 1, 0, a[2], a[3]);
        *t += dur + 200;
    } else if (strcmp(cmd, "pinch") == 0 && n >= 3) {
        double d0 = 60;
        double dur = n >= 4 ? a[3] : 400;
        script_pointer(*t, 0, 0, a[0] - d0, a[1]);
        script_pointer(*t + 8, 0, 1, a[0] + d0, a[1]);
        for (int i = 1; i <= 20; i++) {
            double f = i / 20.0;
            double d = d0 + (d0 * a[2] - d0) * f;
            script_pointer(*t + 16 + dur * f, 2, 0, a[0] - d, a[1]);
            script_pointer(*t + 16 + dur * f, 2, 1, a[0] + d, a[1]);
        }
        script_pointer(*t + dur + 40, 1, 1, a[0] + d0 * a[2], a[1]);
        script_pointer(*t + dur + 48, 1, 0, a[0] - d0 * a[2], a[1]);
        *t += dur + 300;
    } else if (strcmp(cmd, "wait") == 0) {
        *t += atof(arg);
    } else if (strcmp(cmd, "type") == 0) {
        script_post_text(*t, SALAM_EV_TEXT, 0, arg);
        *t += 200;
    } else if (strcmp(cmd, "mark") == 0) {
        strncpy(script_marked, arg, SALAM_HOST_TCAP - 1);
        script_post_text(*t, SALAM_EV_PREEDIT, (int)strlen(script_marked), script_marked);
        *t += 200;
    } else if (strcmp(cmd, "unmark") == 0) {
        script_post_text(*t, SALAM_EV_TEXT, 1, script_marked);
        script_marked[0] = 0;
        *t += 200;
    } else if (strcmp(cmd, "back") == 0) {
        size_t len = strlen(script_marked);
        if (len > 0) {
            while (len > 1 && (((unsigned char)script_marked[len - 1]) & 0xC0) == 0x80) {
                len--;
            }
            script_marked[len - 1] = 0;
            script_post_text(*t, SALAM_EV_PREEDIT, (int)(len - 1), script_marked);
        } else {
            script_at(*t);
            salam_host_post(SALAM_EV_KEY, 1, 67, 0, 0, 0, NULL);
            salam_host_post(SALAM_EV_KEY, 0, 67, 0, 0, 0, NULL);
        }
        *t += 200;
    }
}

static void *script_run(void *arg)
{
    char *s = (char *)arg;
    double t = 1500;
    char *save = NULL;
    clock_gettime(CLOCK_MONOTONIC, &script_t0);
    for (char *step = strtok_r(s, ";", &save); step != NULL; step = strtok_r(NULL, ";", &save)) {
        while (*step == ' ') {
            step++;
        }
        char *sp = strchr(step, ' ');
        const char *a = "";
        if (sp != NULL) {
            *sp = 0;
            a = sp + 1;
        }
        script_step(step, a, &t);
    }
    free(s);
    return NULL;
}

void salam_host_script_set(const char *script)
{
    pthread_mutex_lock(&mu);
    if (script != NULL && script[0] != 0 && !script_used) {
        strncpy(script_text, script, SCRIPT_CAP - 1);
        script_pending = 1;
    }
    pthread_mutex_unlock(&mu);
}

void salam_host_script_frame(void)
{
    char *copy = NULL;
    pthread_mutex_lock(&mu);
    if (script_pending) {
        script_pending = 0;
        script_used = 1;
        copy = strdup(script_text);
    }
    pthread_mutex_unlock(&mu);
    if (copy != NULL) {
        pthread_t th;
        if (pthread_create(&th, NULL, script_run, copy) == 0) {
            pthread_detach(th);
        } else {
            free(copy);
        }
    }
}

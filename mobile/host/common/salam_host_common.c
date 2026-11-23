#include "salam_host_common.h"

#include <pthread.h>
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

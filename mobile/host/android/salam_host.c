#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <errno.h>
#include <jni.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define QCAP 512
#define TCAP 512
#define NV 8

enum {
    EV_RESIZE = 1,
    EV_POINTER = 2,
    EV_KEY = 3,
    EV_TEXT = 4,
    EV_INSETS = 5,
    EV_LIFECYCLE = 6,
    EV_SCHEME = 8,
    EV_PREEDIT = 9
};

typedef struct {
    double v[NV];
    char text[TCAP];
} Ev;

extern int salam_app_main(int argc, char **argv);

static Ev q[QCAP];
static int qh;
static int qt;
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
static int woken;
static ANativeWindow *win;
static int surf_w;
static int surf_h;
static double density = 1.0;
static char cur_text[TCAP];
static pthread_t th;
static int started;
static JavaVM *vm;
static jclass host_cls;
static jmethodID exit_mid;

jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    (void)reserved;
    vm = jvm;
    return JNI_VERSION_1_6;
}

static void push(const Ev *e)
{
    pthread_mutex_lock(&mu);
    int n = (qt + 1) % QCAP;
    if (n == qh) {
        pthread_mutex_unlock(&mu);
        return;
    }
    if (e->v[0] == EV_POINTER && e->v[1] == 2 && qh != qt) {
        Ev *last = &q[(qt + QCAP - 1) % QCAP];
        if (last->v[0] == EV_POINTER && last->v[1] == 2 && last->v[2] == e->v[2]) {
            *last = *e;
            pthread_cond_signal(&cv);
            pthread_mutex_unlock(&mu);
            return;
        }
    }
    q[qt] = *e;
    qt = n;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mu);
}

static void simple(double kind, double a, double b, double c, double d, double f)
{
    Ev e;
    memset(&e, 0, sizeof e);
    e.v[0] = kind;
    e.v[1] = a;
    e.v[2] = b;
    e.v[3] = c;
    e.v[4] = d;
    e.v[5] = f;
    push(&e);
}

int salam_host_poll(double *out)
{
    int kind = 0;
    pthread_mutex_lock(&mu);
    if (qh != qt) {
        memcpy(out, &q[qh].v[1], sizeof(double) * (NV - 1));
        memcpy(cur_text, q[qh].text, TCAP);
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
    pthread_mutex_lock(&mu);
    int r = win != NULL && surf_w > 0 && surf_h > 0;
    pthread_mutex_unlock(&mu);
    return r;
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
    return density;
}

int salam_host_blit(const void *pixels, int bw, int bh, int x, int y, int w, int h)
{
    int ok = 0;
    pthread_mutex_lock(&mu);
    if (win != NULL && pixels != NULL) {
        ARect r;
        ANativeWindow_Buffer b;
        r.left = x < 0 ? 0 : x;
        r.top = y < 0 ? 0 : y;
        r.right = x + w > bw ? bw : x + w;
        r.bottom = y + h > bh ? bh : y + h;
        if (ANativeWindow_lock(win, &b, &r) == 0) {
            int x0 = r.left < 0 ? 0 : r.left;
            int y0 = r.top < 0 ? 0 : r.top;
            int x1 = r.right > bw ? bw : r.right;
            int y1 = r.bottom > bh ? bh : r.bottom;
            if (x1 > b.width) {
                x1 = b.width;
            }
            if (y1 > b.height) {
                y1 = b.height;
            }
            for (int row = y0; row < y1; row++) {
                const uint32_t *s = (const uint32_t *)pixels + (size_t)row * bw + x0;
                uint32_t *d = (uint32_t *)b.bits + (size_t)row * b.stride + x0;
                for (int col = x0; col < x1; col++) {
                    uint32_t p = *s++;
                    *d++ = (p & 0x0000FF00u) | ((p >> 16) & 0xFFu) | ((p & 0xFFu) << 16) | 0xFF000000u;
                }
            }
            ANativeWindow_unlockAndPost(win);
            ok = 1;
        }
    }
    pthread_mutex_unlock(&mu);
    return ok;
}

void salam_host_exit(void)
{
    JNIEnv *env = NULL;
    int attached = 0;
    if (vm == NULL || host_cls == NULL || exit_mid == NULL) {
        return;
    }
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
            return;
        }
        attached = 1;
    }
    (*env)->CallStaticVoidMethod(env, host_cls, exit_mid);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
}

static void *run(void *arg)
{
    char *argv[2] = {"app", NULL};
    (void)arg;
    salam_app_main(1, argv);
    salam_host_exit();
    return NULL;
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeStart(JNIEnv *env, jclass cls, jstring files, jstring cache)
{
    if (started) {
        return;
    }
    const char *f = (*env)->GetStringUTFChars(env, files, NULL);
    const char *c = (*env)->GetStringUTFChars(env, cache, NULL);
    if (f != NULL) {
        setenv("HOME", f, 1);
    }
    if (c != NULL) {
        setenv("TMPDIR", c, 1);
    }
    (*env)->ReleaseStringUTFChars(env, files, f);
    (*env)->ReleaseStringUTFChars(env, cache, c);
    host_cls = (jclass)(*env)->NewGlobalRef(env, cls);
    exit_mid = (*env)->GetStaticMethodID(env, cls, "onExit", "()V");
    started = 1;
    pthread_create(&th, NULL, run, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeSurface(JNIEnv *env, jclass cls, jobject surface, jint w, jint h, jfloat dens)
{
    (void)cls;
    pthread_mutex_lock(&mu);
    if (win != NULL) {
        ANativeWindow_release(win);
        win = NULL;
    }
    if (surface != NULL) {
        win = ANativeWindow_fromSurface(env, surface);
        if (win != NULL) {
            ANativeWindow_setBuffersGeometry(win, w, h, 2);
        }
    }
    surf_w = surface != NULL ? w : 0;
    surf_h = surface != NULL ? h : 0;
    density = dens > 0 ? dens : 1.0;
    pthread_mutex_unlock(&mu);
    if (surface != NULL) {
        simple(EV_RESIZE, w, h, dens, 0, 0);
    }
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativePointer(JNIEnv *env, jclass cls, jint action, jint id, jfloat x, jfloat y)
{
    (void)env;
    (void)cls;
    simple(EV_POINTER, action, id, x, y, 0);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeKey(JNIEnv *env, jclass cls, jint down, jint code, jint meta)
{
    (void)env;
    (void)cls;
    simple(EV_KEY, down, code, meta, 0, 0);
}

static void text_event(JNIEnv *env, jstring s, double kind, double cursor)
{
    Ev e;
    memset(&e, 0, sizeof e);
    e.v[0] = kind;
    e.v[1] = cursor;
    if (s != NULL) {
        const char *u = (*env)->GetStringUTFChars(env, s, NULL);
        if (u != NULL) {
            strncpy(e.text, u, TCAP - 1);
            (*env)->ReleaseStringUTFChars(env, s, u);
        }
    }
    push(&e);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeText(JNIEnv *env, jclass cls, jstring s)
{
    (void)cls;
    text_event(env, s, EV_TEXT, 0);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeCommit(JNIEnv *env, jclass cls, jstring s)
{
    (void)cls;
    text_event(env, s, EV_TEXT, 1);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativePreedit(JNIEnv *env, jclass cls, jstring s, jint cursor)
{
    (void)cls;
    text_event(env, s, EV_PREEDIT, cursor);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeInsets(JNIEnv *env, jclass cls, jfloat t, jfloat r, jfloat b, jfloat l, jfloat kb)
{
    Ev e;
    (void)env;
    (void)cls;
    memset(&e, 0, sizeof e);
    e.v[0] = EV_INSETS;
    e.v[1] = t;
    e.v[2] = r;
    e.v[3] = b;
    e.v[4] = l;
    e.v[5] = kb;
    push(&e);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeLifecycle(JNIEnv *env, jclass cls, jint kind)
{
    (void)env;
    (void)cls;
    simple(EV_LIFECYCLE, kind, 0, 0, 0, 0);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeScheme(JNIEnv *env, jclass cls, jint dark)
{
    (void)env;
    (void)cls;
    simple(EV_SCHEME, dark, 0, 0, 0, 0);
}

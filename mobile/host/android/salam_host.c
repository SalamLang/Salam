#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <signal.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/salam_host_common.h"

#define TCAP SALAM_HOST_TCAP

static char *utf8_from_jstring(JNIEnv *env, jstring s)
{
    if (s == NULL) {
        char *empty = malloc(1);
        if (empty) {
            empty[0] = 0;
        }
        return empty;
    }
    jsize n = (*env)->GetStringLength(env, s);
    const jchar *u = (*env)->GetStringChars(env, s, NULL);
    char *out = malloc((size_t)n * 3 + 1);
    size_t o = 0;
    if (out == NULL) {
        (*env)->ReleaseStringChars(env, s, u);
        return NULL;
    }
    for (jsize i = 0; i < n; i++) {
        uint32_t c = u[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < n && u[i + 1] >= 0xDC00 &&
            u[i + 1] <= 0xDFFF) {
            c = 0x10000 + ((c - 0xD800) << 10) + (u[i + 1] - 0xDC00);
            i++;
        } else if (c >= 0xD800 && c <= 0xDFFF) {
            c = 0xFFFD;
        }
        if (c < 0x80) {
            out[o++] = (char)c;
        } else if (c < 0x800) {
            out[o++] = (char)(0xC0 | (c >> 6));
            out[o++] = (char)(0x80 | (c & 0x3F));
        } else if (c < 0x10000) {
            out[o++] = (char)(0xE0 | (c >> 12));
            out[o++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (c & 0x3F));
        } else {
            out[o++] = (char)(0xF0 | (c >> 18));
            out[o++] = (char)(0x80 | ((c >> 12) & 0x3F));
            out[o++] = (char)(0x80 | ((c >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (c & 0x3F));
        }
    }
    out[o] = 0;
    (*env)->ReleaseStringChars(env, s, u);
    return out;
}

static jstring jstring_from_utf8(JNIEnv *env, const char *s)
{
    if (s == NULL) {
        return (*env)->NewString(env, NULL, 0);
    }
    size_t n = strlen(s);
    jchar *u = malloc((n + 1) * sizeof(jchar));
    size_t o = 0;
    if (u == NULL) {
        return NULL;
    }
    const unsigned char *p = (const unsigned char *)s;
    size_t i = 0;
    while (i < n) {
        uint32_t c = p[i];
        size_t need = 0;
        if (c < 0x80) {
            need = 0;
        } else if ((c & 0xE0) == 0xC0) {
            c &= 0x1F;
            need = 1;
        } else if ((c & 0xF0) == 0xE0) {
            c &= 0x0F;
            need = 2;
        } else if ((c & 0xF8) == 0xF0) {
            c &= 0x07;
            need = 3;
        } else {
            u[o++] = 0xFFFD;
            i++;
            continue;
        }
        int bad = 0;
        for (size_t k = 1; k <= need; k++) {
            if (i + k >= n || (p[i + k] & 0xC0) != 0x80) {
                bad = 1;
                break;
            }
            c = (c << 6) | (p[i + k] & 0x3F);
        }
        if (bad) {
            u[o++] = 0xFFFD;
            i++;
            continue;
        }
        i += need + 1;
        if (c >= 0x10000) {
            c -= 0x10000;
            u[o++] = (jchar)(0xD800 + (c >> 10));
            u[o++] = (jchar)(0xDC00 + (c & 0x3FF));
        } else {
            u[o++] = (jchar)c;
        }
    }
    jstring r = (*env)->NewString(env, u, (jsize)o);
    free(u);
    return r;
}


extern int salam_app_main(int argc, char **argv);

#if __ANDROID_API__ < 28
int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
    return (int)syscall(__NR_rt_sigtimedwait, set, info, timeout, 8);
}
#endif

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static ANativeWindow *win;
static pthread_t th;
static int started;
static JavaVM *vm;
static jclass host_cls;
static jmethodID exit_mid;
static jmethodID ime_mid;
static jmethodID open_mid;
static jmethodID clip_set_mid;
static jmethodID clip_get_mid;
static char clip_buf[8192];
static int ime_state;

jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    (void)reserved;
    vm = jvm;
    return JNI_VERSION_1_6;
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

void salam_host_ime(int on)
{
    JNIEnv *env = NULL;
    int attached = 0;
    if (on == ime_state || vm == NULL || host_cls == NULL || ime_mid == NULL) {
        return;
    }
    ime_state = on;
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
            return;
        }
        attached = 1;
    }
    (*env)->CallStaticVoidMethod(env, host_cls, ime_mid, on);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
}

static JNIEnv *host_env(int *attached)
{
    JNIEnv *env = NULL;
    *attached = 0;
    if (vm == NULL || host_cls == NULL) {
        return NULL;
    }
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
            return NULL;
        }
        *attached = 1;
    }
    return env;
}

static int call_bool_string(jmethodID mid, const char *s)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || mid == NULL) {
        return 0;
    }
    jstring js = jstring_from_utf8(env, s);
    if (js != NULL) {
        r = (*env)->CallStaticBooleanMethod(env, host_cls, mid, js) ? 1 : 0;
        (*env)->DeleteLocalRef(env, js);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_open_url(const char *url)
{
    return call_bool_string(open_mid, url);
}

int salam_host_clip_set(const char *text)
{
    return call_bool_string(clip_set_mid, text);
}

const char *salam_host_clip_get(void)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    clip_buf[0] = 0;
    if (env == NULL || clip_get_mid == NULL) {
        return clip_buf;
    }
    jstring js = (jstring)(*env)->CallStaticObjectMethod(env, host_cls, clip_get_mid);
    if (js != NULL) {
        char *u = utf8_from_jstring(env, js);
        if (u != NULL) {
            strncpy(clip_buf, u, sizeof clip_buf - 1);
            clip_buf[sizeof clip_buf - 1] = 0;
            free(u);
        }
        (*env)->DeleteLocalRef(env, js);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return clip_buf;
}

static int log_pipe[2];

static void *log_reader(void *arg)
{
    char buf[1024];
    size_t n = 0;
    char c;
    (void)arg;
    while (read(log_pipe[0], &c, 1) == 1) {
        if (c == '\n' || n == sizeof buf - 1) {
            buf[n] = 0;
            __android_log_write(ANDROID_LOG_INFO, "salam", buf);
            n = 0;
        } else {
            buf[n++] = c;
        }
    }
    return NULL;
}

static void flush_logs(void)
{
    fflush(NULL);
    usleep(200000);
}

static void redirect_output(void)
{
    atexit(flush_logs);
    pthread_t t;
    if (pipe(log_pipe) != 0) {
        return;
    }
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    dup2(log_pipe[1], 1);
    dup2(log_pipe[1], 2);
    pthread_create(&t, NULL, log_reader, NULL);
    pthread_detach(t);
}

static void *run(void *arg)
{
    char *argv[2] = {"app", NULL};
    (void)arg;
    salam_app_main(1, argv);
    fflush(NULL);
    usleep(150000);
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
    ime_mid = (*env)->GetStaticMethodID(env, cls, "showIme", "(Z)V");
    open_mid = (*env)->GetStaticMethodID(env, cls, "openUrl", "(Ljava/lang/String;)Z");
    clip_set_mid = (*env)->GetStaticMethodID(env, cls, "clipSet", "(Ljava/lang/String;)Z");
    clip_get_mid = (*env)->GetStaticMethodID(env, cls, "clipGet", "()Ljava/lang/String;");
    started = 1;
    redirect_output();
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
    pthread_mutex_unlock(&mu);
    salam_host_set_surface(surface != NULL ? w : 0, surface != NULL ? h : 0, dens);
    if (surface != NULL) {
        salam_host_post(SALAM_EV_RESIZE, w, h, dens, 0, 0, NULL);
    }
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativePointer(JNIEnv *env, jclass cls, jint action, jint id, jfloat x, jfloat y)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_POINTER, action, id, x, y, 0, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeKey(JNIEnv *env, jclass cls, jint down, jint code, jint meta)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_KEY, down, code, meta, 0, 0, NULL);
}

static void text_event(JNIEnv *env, jstring s, double kind, double cursor)
{
    char *u = utf8_from_jstring(env, s);
    char buf[TCAP];
    size_t n = 0;
    memset(buf, 0, sizeof buf);
    if (u != NULL) {
        n = strlen(u);
        if (n >= TCAP) {
            n = TCAP - 1;
            while (n > 0 && (((unsigned char)u[n]) & 0xC0) == 0x80) {
                n--;
            }
        }
        memcpy(buf, u, n);
        free(u);
    }
    salam_host_post(kind, cursor, 0, 0, 0, 0, buf);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeText(JNIEnv *env, jclass cls, jstring s)
{
    (void)cls;
    text_event(env, s, SALAM_EV_TEXT, 0);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeCommit(JNIEnv *env, jclass cls, jstring s)
{
    (void)cls;
    text_event(env, s, SALAM_EV_TEXT, 1);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativePreedit(JNIEnv *env, jclass cls, jstring s, jint cursor)
{
    (void)cls;
    text_event(env, s, SALAM_EV_PREEDIT, cursor);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeInsets(JNIEnv *env, jclass cls, jfloat t, jfloat r, jfloat b, jfloat l, jfloat kb)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_INSETS, t, r, b, l, kb, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeLifecycle(JNIEnv *env, jclass cls, jint kind)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_LIFECYCLE, kind, 0, 0, 0, 0, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeScheme(JNIEnv *env, jclass cls, jint dark)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_SCHEME, dark, 0, 0, 0, 0, NULL);
}

#include <android/log.h>
#if defined(__aarch64__)
#include <arm_neon.h>
#endif
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <signal.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <unwind.h>
#include <sys/syscall.h>
#include <sys/system_properties.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/salam_host_common.h"

#define TCAP SALAM_HOST_TCAP

static void crash_hook(void);

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

static JNIEnv *host_env(int *attached);

static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static ANativeWindow *win;
static pthread_t th;
static int started;
static JavaVM *vm;
static jclass host_cls;
static jmethodID exit_mid;
static jmethodID ime_mid;
static jmethodID open_mid;
static jmethodID bars_mid;
static jmethodID page_bg_mid;
static uint32_t page_bg_last;
static int bars_top = -1;
static int bars_bottom = -1;
static jmethodID clip_set_mid;
static jmethodID clip_get_mid;
static jmethodID share_mid;
static jmethodID vibrate_mid;
static jmethodID haptic_mid;
static jmethodID path_mid;
static jmethodID screen_mid;
static jmethodID brightness_mid;
static jmethodID settings_mid;
static char launch_url[2048];
static char pending_url[2048];
static jmethodID info_mid;
static jmethodID network_mid;
static jmethodID secure_set_mid;
static jmethodID secure_get_mid;
static jmethodID secure_delete_mid;
static jmethodID perm_state_mid;
static jmethodID notify_mid;
static jmethodID loc_mid;
static jmethodID photo_mid;
static int photo_state;
static char photo_path[1024];
static pthread_mutex_t loc_mu = PTHREAD_MUTEX_INITIALIZER;
static int loc_state;
static double loc_val[3];
static jmethodID cancel_mid;
static jmethodID perm_request_mid;
static char clip_buf[8192];
static int ime_state;
static int ime_cfg;
static jmethodID ime_cfg_mid;

jint JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    (void)reserved;
    vm = jvm;
    return JNI_VERSION_1_6;
}

static void swizzle_row(const uint32_t *s, uint32_t *d, int n)
{
    int i = 0;
#if defined(__aarch64__)
    static const uint8_t idx[16] = {2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15};
    const uint8x16_t tbl = vld1q_u8(idx);
    const uint32x4_t alpha = vdupq_n_u32(0xFF000000u);
    for (; i + 4 <= n; i += 4) {
        uint8x16_t v = vqtbl1q_u8(vld1q_u8((const uint8_t *)(s + i)), tbl);
        vst1q_u32(d + i, vorrq_u32(vreinterpretq_u32_u8(v), alpha));
    }
#endif
    for (; i < n; i++) {
        uint32_t p = s[i];
        d[i] = (p & 0x0000FF00u) | ((p >> 16) & 0xFFu) | ((p & 0xFFu) << 16) | 0xFF000000u;
    }
}

static uint32_t swizzle_px(uint32_t p)
{
    return (p & 0x0000FF00u) | ((p >> 16) & 0xFFu) | ((p & 0xFFu) << 16) | 0xFF000000u;
}

static int pixel_is_light(uint32_t p)
{
    unsigned lum = ((p & 0xFFu) + 2u * ((p >> 8) & 0xFFu) + ((p >> 16) & 0xFFu)) / 4u;
    return lum > 140u;
}

static void report_bars(int top, int bottom)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    if (env == NULL || bars_mid == NULL) {
        return;
    }
    (*env)->CallStaticVoidMethod(env, host_cls, bars_mid, top, bottom);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
}

static void report_page_bg(uint32_t argb)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    if (env == NULL || page_bg_mid == NULL) {
        return;
    }
    (*env)->CallStaticVoidMethod(env, host_cls, page_bg_mid, (jint)argb);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
}

int salam_host_blit(const void *pixels, int bw, int bh, int x, int y, int w, int h)
{
    int ok = 0;
    int top = -1;
    int bottom = -1;
    int first_frame = 0;
    uint32_t page = 0;
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
                swizzle_row((const uint32_t *)pixels + (size_t)row * bw + x0,
                            (uint32_t *)b.bits + (size_t)row * b.stride + x0, x1 - x0);
            }
            ANativeWindow_unlockAndPost(win);
            ok = 1;
            first_frame = 1;
            page = swizzle_px(((const uint32_t *)pixels)[0]);
            if (y0 == 0 && x1 > x0) {
                top = pixel_is_light(((const uint32_t *)pixels)[bw / 2]);
            }
            if (y1 == bh && x1 > x0) {
                bottom = pixel_is_light(((const uint32_t *)pixels)[(size_t)(bh - 1) * bw + bw / 2]);
            }
        }
    }
    pthread_mutex_unlock(&mu);
    if (first_frame) {
        static int hooked;
        if (!hooked) {
            hooked = 1;
            crash_hook();
        }
        salam_host_script_frame();
        if (page != page_bg_last) {
            page_bg_last = page;
            report_page_bg(page);
        }
    }
    if ((top >= 0 && top != bars_top) || (bottom >= 0 && bottom != bars_bottom)) {
        if (top >= 0) {
            bars_top = top;
        }
        if (bottom >= 0) {
            bars_bottom = bottom;
        }
        report_bars(bars_top < 0 ? 0 : bars_top, bars_bottom < 0 ? 0 : bars_bottom);
    }
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

void salam_host_ime_config(int config)
{
    JNIEnv *env = NULL;
    int attached = 0;
    if (config == ime_cfg) {
        return;
    }
    ime_cfg = config;
    if (vm == NULL || host_cls == NULL || ime_cfg_mid == NULL) {
        return;
    }
    if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
        if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
            return;
        }
        attached = 1;
    }
    (*env)->CallStaticVoidMethod(env, host_cls, ime_cfg_mid, config);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
}

void salam_host_ime_rect(int x, int y, int w, int h)
{
    (void)x;
    (void)y;
    (void)w;
    (void)h;
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

static int call_bool_2s(jmethodID mid, const char *a, const char *b)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || mid == NULL) {
        return 0;
    }
    jstring ja = jstring_from_utf8(env, a);
    jstring jb = jstring_from_utf8(env, b);
    if (ja != NULL && jb != NULL) {
        r = (*env)->CallStaticBooleanMethod(env, host_cls, mid, ja, jb) ? 1 : 0;
    }
    if (ja != NULL) {
        (*env)->DeleteLocalRef(env, ja);
    }
    if (jb != NULL) {
        (*env)->DeleteLocalRef(env, jb);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

static int call_bool_int(jmethodID mid, int v)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || mid == NULL) {
        return 0;
    }
    r = (*env)->CallStaticBooleanMethod(env, host_cls, mid, (jint)v) ? 1 : 0;
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_photo_request(int camera)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || photo_mid == NULL) {
        return 0;
    }
    pthread_mutex_lock(&loc_mu);
    photo_state = 1;
    pthread_mutex_unlock(&loc_mu);
    r = (*env)->CallStaticBooleanMethod(env, host_cls, photo_mid, (jint)camera) ? 1 : 0;
    if (r == 0) {
        pthread_mutex_lock(&loc_mu);
        photo_state = 3;
        pthread_mutex_unlock(&loc_mu);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_photo_state(void)
{
    int s;
    pthread_mutex_lock(&loc_mu);
    s = photo_state;
    pthread_mutex_unlock(&loc_mu);
    return s;
}

const char *salam_host_photo_path(void)
{
    static char copy[1024];
    pthread_mutex_lock(&loc_mu);
    memcpy(copy, photo_path, sizeof copy);
    pthread_mutex_unlock(&loc_mu);
    return copy;
}

int salam_host_location_request(void)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || loc_mid == NULL) {
        return 0;
    }
    pthread_mutex_lock(&loc_mu);
    loc_state = 1;
    pthread_mutex_unlock(&loc_mu);
    r = (*env)->CallStaticBooleanMethod(env, host_cls, loc_mid) ? 1 : 0;
    if (r == 0) {
        pthread_mutex_lock(&loc_mu);
        loc_state = 3;
        pthread_mutex_unlock(&loc_mu);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_location_get(double *out)
{
    int s;
    pthread_mutex_lock(&loc_mu);
    s = loc_state;
    out[0] = loc_val[0];
    out[1] = loc_val[1];
    out[2] = loc_val[2];
    pthread_mutex_unlock(&loc_mu);
    return s;
}

int salam_host_notify(int id, const char *title, const char *body, int delay_ms)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || notify_mid == NULL) {
        return 0;
    }
    jstring jt = jstring_from_utf8(env, title);
    jstring jb = jstring_from_utf8(env, body);
    if (jt != NULL && jb != NULL) {
        r = (*env)->CallStaticBooleanMethod(env, host_cls, notify_mid, (jint)id, jt, jb, (jint)delay_ms) ? 1 : 0;
    }
    if (jt != NULL) {
        (*env)->DeleteLocalRef(env, jt);
    }
    if (jb != NULL) {
        (*env)->DeleteLocalRef(env, jb);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_notify_cancel(int id)
{
    return call_bool_int(cancel_mid, id);
}

int salam_host_perm_state(const char *name)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || perm_state_mid == NULL) {
        return 0;
    }
    jstring js = jstring_from_utf8(env, name);
    if (js != NULL) {
        r = (int)(*env)->CallStaticIntMethod(env, host_cls, perm_state_mid, js);
        (*env)->DeleteLocalRef(env, js);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_perm_request(const char *name)
{
    return call_bool_string(perm_request_mid, name);
}

int salam_host_vibrate(int ms)
{
    return call_bool_int(vibrate_mid, ms);
}

int salam_host_haptic(int kind)
{
    return call_bool_int(haptic_mid, kind);
}

const char *salam_host_path(int kind)
{
    static char copy[1024];
    int attached;
    JNIEnv *env = host_env(&attached);
    copy[0] = 0;
    if (env == NULL || path_mid == NULL) {
        return copy;
    }
    jstring js = (jstring)(*env)->CallStaticObjectMethod(env, host_cls, path_mid, (jint)kind);
    if (js != NULL) {
        char *u = utf8_from_jstring(env, js);
        if (u != NULL) {
            strncpy(copy, u, sizeof copy - 1);
            copy[sizeof copy - 1] = 0;
            free(u);
        }
        (*env)->DeleteLocalRef(env, js);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return copy;
}

int salam_host_secure_set(const char *key, const char *value)
{
    return call_bool_2s(secure_set_mid, key, value);
}

int salam_host_secure_delete(const char *key)
{
    return call_bool_string(secure_delete_mid, key);
}

const char *salam_host_secure_get(const char *key)
{
    static char copy[16384];
    int attached;
    JNIEnv *env = host_env(&attached);
    copy[0] = 0;
    if (env == NULL || secure_get_mid == NULL) {
        return copy;
    }
    jstring jk = jstring_from_utf8(env, key);
    if (jk != NULL) {
        jstring js = (jstring)(*env)->CallStaticObjectMethod(env, host_cls, secure_get_mid, jk);
        if (js != NULL) {
            char *u = utf8_from_jstring(env, js);
            if (u != NULL) {
                strncpy(copy, u, sizeof copy - 1);
                copy[sizeof copy - 1] = 0;
                free(u);
            }
            (*env)->DeleteLocalRef(env, js);
        }
        (*env)->DeleteLocalRef(env, jk);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return copy;
}

const char *salam_host_info(const char *key)
{
    static char copy[2048];
    int attached;
    JNIEnv *env = host_env(&attached);
    copy[0] = 0;
    if (env == NULL || info_mid == NULL) {
        return copy;
    }
    jstring jk = jstring_from_utf8(env, key);
    if (jk != NULL) {
        jstring js = (jstring)(*env)->CallStaticObjectMethod(env, host_cls, info_mid, jk);
        if (js != NULL) {
            char *u = utf8_from_jstring(env, js);
            if (u != NULL) {
                strncpy(copy, u, sizeof copy - 1);
                copy[sizeof copy - 1] = 0;
                free(u);
            }
            (*env)->DeleteLocalRef(env, js);
        }
        (*env)->DeleteLocalRef(env, jk);
    }
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return copy;
}

int salam_host_network(void)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || network_mid == NULL) {
        return 0;
    }
    r = (int)(*env)->CallStaticIntMethod(env, host_cls, network_mid);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

const char *salam_host_launch_url(void)
{
    static char copy[2048];
    pthread_mutex_lock(&loc_mu);
    memcpy(copy, launch_url, sizeof copy);
    pthread_mutex_unlock(&loc_mu);
    return copy;
}

const char *salam_host_take_url(void)
{
    static char copy[2048];
    pthread_mutex_lock(&loc_mu);
    memcpy(copy, pending_url, sizeof copy);
    pending_url[0] = 0;
    pthread_mutex_unlock(&loc_mu);
    return copy;
}

int salam_host_open_settings(void)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || settings_mid == NULL) {
        return 0;
    }
    r = (*env)->CallStaticBooleanMethod(env, host_cls, settings_mid) ? 1 : 0;
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_screen(int what, int value)
{
    int attached;
    JNIEnv *env = host_env(&attached);
    int r = 0;
    if (env == NULL || screen_mid == NULL) {
        return 0;
    }
    r = (int)(*env)->CallStaticIntMethod(env, host_cls, screen_mid, (jint)what, (jint)value);
    if (attached) {
        (*vm)->DetachCurrentThread(vm);
    }
    return r;
}

int salam_host_share(const char *text, const char *url)
{
    return call_bool_2s(share_mid, text, url);
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
static char log_buf[1024];
static size_t log_len;
static char panic_msg[256];

static void *log_reader(void *arg)
{
    char c;
    (void)arg;
    while (read(log_pipe[0], &c, 1) == 1) {
        if (c == '\n' || log_len == sizeof log_buf - 1) {
            log_buf[log_len] = 0;
            log_len = 0;
            if (strncmp(log_buf, "salam panic:", 12) == 0) {
                strncpy(panic_msg, log_buf, sizeof panic_msg - 1);
            }
            __android_log_write(ANDROID_LOG_INFO, "salam", log_buf);
        } else {
            log_buf[log_len++] = c;
        }
    }
    return NULL;
}

static char crash_path[1100];
static int crash_fd = -1;
static volatile sig_atomic_t crashing;
static const int crash_sigs[] = {SIGABRT, SIGSEGV, SIGBUS, SIGFPE, SIGILL, SIGTRAP};
static struct sigaction crash_old[6];
static char crash_stack[65536];

static void crash_put(const char *s)
{
    __android_log_write(ANDROID_LOG_FATAL, "salam", s);
    if (crash_fd >= 0) {
        if (write(crash_fd, s, strlen(s)) < 0) {
            return;
        }
        if (write(crash_fd, "\n", 1) < 0) {
            return;
        }
    }
}

static _Unwind_Reason_Code crash_frame(struct _Unwind_Context *ctx, void *arg)
{
    int *n = (int *)arg;
    uintptr_t pc = _Unwind_GetIP(ctx);
    Dl_info di;
    char line[400];
    if (pc == 0 || *n >= 48) {
        return _URC_END_OF_STACK;
    }
    if (dladdr((void *)pc, &di) != 0 && di.dli_fname != NULL) {
        uintptr_t base = (uintptr_t)di.dli_fbase;
        const char *slash = strrchr(di.dli_fname, '/');
        const char *lib = slash != NULL ? slash + 1 : di.dli_fname;
        if (di.dli_sname != NULL) {
            snprintf(line, sizeof line, "  #%02d pc %08lx %s (%s+%lu)", *n, (unsigned long)(pc - base), lib,
                     di.dli_sname, (unsigned long)(pc - (uintptr_t)di.dli_saddr));
        } else {
            snprintf(line, sizeof line, "  #%02d pc %08lx %s", *n, (unsigned long)(pc - base), lib);
        }
    } else {
        snprintf(line, sizeof line, "  #%02d pc %p", *n, (void *)pc);
    }
    crash_put(line);
    (*n)++;
    return _URC_NO_REASON;
}

static const char *crash_name(int sig)
{
    switch (sig) {
    case SIGABRT: return "SIGABRT";
    case SIGSEGV: return "SIGSEGV";
    case SIGBUS: return "SIGBUS";
    case SIGFPE: return "SIGFPE";
    case SIGILL: return "SIGILL";
    case SIGTRAP: return "SIGTRAP";
    }
    return "signal";
}

static void crash_handler(int sig, siginfo_t *info, void *uc)
{
    char line[160];
    char text[4096];
    ssize_t got;
    int n = 0;
    (void)uc;
    if (crashing) {
        _exit(128 + sig);
    }
    crashing = 1;
    if (crash_path[0] != 0) {
        crash_fd = open(crash_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    }
    snprintf(line, sizeof line, "fatal signal %d (%s), fault addr %p", sig, crash_name(sig), info != NULL ? info->si_addr : NULL);
    crash_put(line);
    if (panic_msg[0] != 0) {
        crash_put(panic_msg);
    }
    fflush(NULL);
    got = 0;
    if (log_pipe[0] > 0) {
        fcntl(log_pipe[0], F_SETFL, O_NONBLOCK);
        got = read(log_pipe[0], text, sizeof text - 1);
    }
    if (log_len > 0) {
        log_buf[log_len] = 0;
        crash_put(log_buf);
    }
    if (got > 0) {
        text[got] = 0;
        crash_put(text);
    }
    crash_put("backtrace:");
    _Unwind_Backtrace(crash_frame, &n);
    if (crash_fd >= 0) {
        close(crash_fd);
    }
    for (size_t i = 0; i < sizeof crash_sigs / sizeof crash_sigs[0]; i++) {
        if (crash_sigs[i] == sig) {
            sigaction(sig, &crash_old[i], NULL);
        }
    }
    if (sig != SIGSEGV && sig != SIGBUS && sig != SIGFPE && sig != SIGILL) {
        raise(sig);
    }
}

static void crash_hook(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_sigaction = crash_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    for (size_t i = 0; i < sizeof crash_sigs / sizeof crash_sigs[0]; i++) {
        struct sigaction cur;
        sigaction(crash_sigs[i], NULL, &cur);
        if (cur.sa_sigaction != crash_handler) {
            crash_old[i] = cur;
            sigaction(crash_sigs[i], &sa, NULL);
        }
    }
}

static void crash_install(const char *files)
{
    stack_t st;
    if (files != NULL) {
        snprintf(crash_path, sizeof crash_path, "%s/crash.txt", files);
    }
    st.ss_sp = crash_stack;
    st.ss_size = sizeof crash_stack;
    st.ss_flags = 0;
    sigaltstack(&st, NULL);
    crash_hook();
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

static void crash_alt_stack(void)
{
    stack_t st;
    st.ss_sp = malloc(65536);
    st.ss_size = 65536;
    st.ss_flags = 0;
    if (st.ss_sp != NULL) {
        sigaltstack(&st, NULL);
    }
}

static void *run(void *arg)
{
    char *argv[2] = {"app", NULL};
    (void)arg;
    crash_alt_stack();
    salam_app_main(1, argv);
    fflush(NULL);
    usleep(150000);
    salam_host_exit();
    return NULL;
}

static int script_given;

static void script_fallback(const char *files)
{
    char buf[PROP_VALUE_MAX];
    char path[1024];
    char text[4096];
    if (script_given) {
        return;
    }
    if (__system_property_get("debug.salam.script", buf) > 0) {
        salam_host_script_set(buf);
        return;
    }
    if (files == NULL) {
        return;
    }
    snprintf(path, sizeof path, "%s/salam-script", files);
    FILE *f = fopen(path, "r");
    if (f != NULL) {
        size_t n = fread(text, 1, sizeof text - 1, f);
        fclose(f);
        while (n > 0 && (text[n - 1] == '\n' || text[n - 1] == '\r')) {
            n--;
        }
        text[n] = 0;
        salam_host_script_set(text);
    }
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeScript(JNIEnv *env, jclass cls, jstring s)
{
    (void)cls;
    char *u = utf8_from_jstring(env, s);
    if (u != NULL && u[0] != 0) {
        salam_host_script_set(u);
        script_given = 1;
    }
    free(u);
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
    script_fallback(f);
    crash_install(f);
    if (c != NULL) {
        setenv("TMPDIR", c, 1);
    }
    (*env)->ReleaseStringUTFChars(env, files, f);
    (*env)->ReleaseStringUTFChars(env, cache, c);
    host_cls = (jclass)(*env)->NewGlobalRef(env, cls);
    exit_mid = (*env)->GetStaticMethodID(env, cls, "onExit", "()V");
    ime_mid = (*env)->GetStaticMethodID(env, cls, "showIme", "(Z)V");
    ime_cfg_mid = (*env)->GetStaticMethodID(env, cls, "setImeConfig", "(I)V");
    bars_mid = (*env)->GetStaticMethodID(env, cls, "barsLight", "(II)V");
    page_bg_mid = (*env)->GetStaticMethodID(env, cls, "pageBackground", "(I)V");
    open_mid = (*env)->GetStaticMethodID(env, cls, "openUrl", "(Ljava/lang/String;)Z");
    clip_set_mid = (*env)->GetStaticMethodID(env, cls, "clipSet", "(Ljava/lang/String;)Z");
    clip_get_mid = (*env)->GetStaticMethodID(env, cls, "clipGet", "()Ljava/lang/String;");
    vibrate_mid = (*env)->GetStaticMethodID(env, cls, "vibrate", "(I)Z");
    haptic_mid = (*env)->GetStaticMethodID(env, cls, "haptic", "(I)Z");
    path_mid = (*env)->GetStaticMethodID(env, cls, "path", "(I)Ljava/lang/String;");
    screen_mid = (*env)->GetStaticMethodID(env, cls, "screen", "(II)I");
    settings_mid = (*env)->GetStaticMethodID(env, cls, "openSettings", "()Z");
    info_mid = (*env)->GetStaticMethodID(env, cls, "info", "(Ljava/lang/String;)Ljava/lang/String;");
    network_mid = (*env)->GetStaticMethodID(env, cls, "network", "()I");
    secure_set_mid = (*env)->GetStaticMethodID(env, cls, "secureSet", "(Ljava/lang/String;Ljava/lang/String;)Z");
    secure_get_mid = (*env)->GetStaticMethodID(env, cls, "secureGet", "(Ljava/lang/String;)Ljava/lang/String;");
    secure_delete_mid = (*env)->GetStaticMethodID(env, cls, "secureDelete", "(Ljava/lang/String;)Z");
    perm_state_mid = (*env)->GetStaticMethodID(env, cls, "permState", "(Ljava/lang/String;)I");
    perm_request_mid = (*env)->GetStaticMethodID(env, cls, "permRequest", "(Ljava/lang/String;)Z");
    notify_mid = (*env)->GetStaticMethodID(env, cls, "notify", "(ILjava/lang/String;Ljava/lang/String;I)Z");
    cancel_mid = (*env)->GetStaticMethodID(env, cls, "cancelNotification", "(I)Z");
    loc_mid = (*env)->GetStaticMethodID(env, cls, "locationRequest", "()Z");
    photo_mid = (*env)->GetStaticMethodID(env, cls, "photoRequest", "(I)Z");
    share_mid = (*env)->GetStaticMethodID(env, cls, "share", "(Ljava/lang/String;Ljava/lang/String;)Z");
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

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeLocation(JNIEnv *env, jclass cls, jint ok, jdouble lat, jdouble lon, jdouble acc)
{
    (void)env;
    (void)cls;
    pthread_mutex_lock(&loc_mu);
    loc_state = ok ? 2 : 3;
    loc_val[0] = lat;
    loc_val[1] = lon;
    loc_val[2] = acc;
    pthread_mutex_unlock(&loc_mu);
    salam_host_post(SALAM_EV_DEVICE, 2, 0, 0, 0, 0, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativePhoto(JNIEnv *env, jclass cls, jstring path)
{
    (void)cls;
    char *u = utf8_from_jstring(env, path);
    pthread_mutex_lock(&loc_mu);
    photo_path[0] = 0;
    if (u != NULL && u[0] != 0) {
        strncpy(photo_path, u, sizeof photo_path - 1);
        photo_path[sizeof photo_path - 1] = 0;
        photo_state = 2;
    } else {
        photo_state = 3;
    }
    pthread_mutex_unlock(&loc_mu);
    free(u);
    salam_host_post(SALAM_EV_DEVICE, 3, 0, 0, 0, 0, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeUrl(JNIEnv *env, jclass cls, jint kind, jstring url)
{
    (void)cls;
    char *u = utf8_from_jstring(env, url);
    if (u == NULL) {
        return;
    }
    pthread_mutex_lock(&loc_mu);
    if (kind == 0) {
        strncpy(launch_url, u, sizeof launch_url - 1);
        launch_url[sizeof launch_url - 1] = 0;
    } else {
        strncpy(pending_url, u, sizeof pending_url - 1);
        pending_url[sizeof pending_url - 1] = 0;
    }
    pthread_mutex_unlock(&loc_mu);
    free(u);
    if (kind != 0) {
        salam_host_post(SALAM_EV_DEVICE, 5, 0, 0, 0, 0, NULL);
    }
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeDevice(JNIEnv *env, jclass cls, jint kind)
{
    (void)env;
    (void)cls;
    salam_host_post(SALAM_EV_DEVICE, kind, 0, 0, 0, 0, NULL);
}

JNIEXPORT void JNICALL Java_ir_salamlang_host_Host_nativeScheme(JNIEnv *env, jclass cls, jint dark)
{
    (void)env;
    (void)cls;
    bars_top = -1;
    bars_bottom = -1;
    salam_host_post(SALAM_EV_SCHEME, dark, 0, 0, 0, 0, NULL);
}

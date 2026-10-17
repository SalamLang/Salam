#include "salam_engine.h"

#include <jni.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

static char *utf8_from_jstring(JNIEnv *env, jstring s) {
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
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < n && u[i + 1] >= 0xDC00 && u[i + 1] <= 0xDFFF) {
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

static jstring jstring_from_utf8(JNIEnv *env, const char *s) {
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

static jstring take(JNIEnv *env, char *s) {
    jstring r = jstring_from_utf8(env, s);
    salam_engine_free(s);
    return r;
}

JNIEXPORT jint JNICALL Java_ir_salamlang_app_Engine_nativeBoot(JNIEnv *env, jclass cls, jstring std_root) {
    (void)cls;
    char *root = utf8_from_jstring(env, std_root);
    int rc = salam_engine_boot(root);
    free(root);
    return rc;
}

JNIEXPORT jint JNICALL Java_ir_salamlang_app_Engine_nativeCli(JNIEnv *env, jclass cls, jstring args) {
    (void)cls;
    char *a = utf8_from_jstring(env, args);
    int rc = salam_engine_cli(a);
    fflush(stdout);
    fflush(stderr);
    free(a);
    return rc;
}

JNIEXPORT void JNICALL Java_ir_salamlang_app_Engine_nativeStop(JNIEnv *env, jclass cls) {
    (void)env;
    (void)cls;
    salam_engine_stop();
}

JNIEXPORT void JNICALL Java_ir_salamlang_app_Engine_nativeClearStop(JNIEnv *env, jclass cls) {
    (void)env;
    (void)cls;
    salam_engine_clear_stop();
}

JNIEXPORT jstring JNICALL Java_ir_salamlang_app_Engine_nativeTokens(JNIEnv *env, jclass cls, jstring source, jstring lang) {
    (void)cls;
    char *s = utf8_from_jstring(env, source);
    char *l = utf8_from_jstring(env, lang);
    jstring r = take(env, salam_engine_tokens(s, l));
    free(s);
    free(l);
    return r;
}

JNIEXPORT jstring JNICALL Java_ir_salamlang_app_Engine_nativeCheck(JNIEnv *env, jclass cls, jstring source, jstring path, jstring lang) {
    (void)cls;
    char *s = utf8_from_jstring(env, source);
    char *p = utf8_from_jstring(env, path);
    char *l = utf8_from_jstring(env, lang);
    jstring r = take(env, salam_engine_check(s, p, l));
    free(s);
    free(p);
    free(l);
    return r;
}

JNIEXPORT jstring JNICALL Java_ir_salamlang_app_Engine_nativeDetect(JNIEnv *env, jclass cls, jstring source) {
    (void)cls;
    char *s = utf8_from_jstring(env, source);
    jstring r = take(env, salam_engine_detect(s));
    free(s);
    return r;
}

JNIEXPORT jstring JNICALL Java_ir_salamlang_app_Engine_nativeVersion(JNIEnv *env, jclass cls) {
    (void)cls;
    return take(env, salam_engine_version());
}

JNIEXPORT jintArray JNICALL Java_ir_salamlang_app_Engine_nativeRedirect(JNIEnv *env, jclass cls) {
    (void)cls;
    int out_pipe[2], err_pipe[2], in_pipe[2];
    if (pipe(out_pipe) != 0 || pipe(err_pipe) != 0 || pipe(in_pipe) != 0) {
        return NULL;
    }
    fflush(stdout);
    fflush(stderr);
    dup2(out_pipe[1], 1);
    dup2(err_pipe[1], 2);
    dup2(in_pipe[0], 0);
    close(out_pipe[1]);
    close(err_pipe[1]);
    close(in_pipe[0]);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    jint fds[3] = {out_pipe[0], err_pipe[0], in_pipe[1]};
    jintArray r = (*env)->NewIntArray(env, 3);
    if (r != NULL) {
        (*env)->SetIntArrayRegion(env, r, 0, 3, fds);
    }
    return r;
}

JNIEXPORT jint JNICALL Java_ir_salamlang_app_Engine_nativeChdir(JNIEnv *env, jclass cls, jstring dir) {
    (void)cls;
    char *d = utf8_from_jstring(env, dir);
    int rc = d ? chdir(d) : -1;
    free(d);
    return rc;
}

JNIEXPORT void JNICALL Java_ir_salamlang_app_Engine_nativeDrainStdin(JNIEnv *env, jclass cls) {
    (void)env;
    (void)cls;
    int flags = fcntl(0, F_GETFL);
    if (flags < 0) {
        return;
    }
    fcntl(0, F_SETFL, flags | O_NONBLOCK);
    char buf[512];
    while (read(0, buf, sizeof buf) > 0) {
    }
    fcntl(0, F_SETFL, flags);
}

JNIEXPORT void JNICALL Java_ir_salamlang_app_Engine_nativeMark(JNIEnv *env, jclass cls, jstring text) {
    (void)cls;
    char *t = utf8_from_jstring(env, text);
    if (t == NULL) {
        return;
    }
    fflush(stdout);
    size_t n = strlen(t);
    size_t off = 0;
    while (off < n) {
        ssize_t w = write(1, t + off, n - off);
        if (w <= 0) {
            break;
        }
        off += (size_t)w;
    }
    free(t);
}

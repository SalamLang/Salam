/* Salam Programming Language (2024-2026) - std/opencv TEST-ONLY mock shim
 *
 * This is NOT OpenCV. It is a small, deterministic, dependency-free
 * stand-in for native/opencv_shim.cpp's ABI (opencv_shim.hpp), written in
 * plain C so it builds anywhere with no OpenCV install - its only purpose
 * is to let std/opencv's *Salam-side* code (linking, memory ownership,
 * struct plumbing, pixel math at the FFI boundary) be exercised and
 * regression-tested without requiring real OpenCV on every dev machine or
 * CI runner. It implements real, correct logic for the "core" operations
 * (io via a tiny PPM/PGM codec, color conversion, resize/crop/rotate/flip,
 * box blur, threshold, histogram, drawing including ellipse/polylines/
 * fill_poly, morphology's erode/dilate/open/close, arithmetic blend/
 * bitwise/abs_diff, brute-force template matching, and trackbar
 * bookkeeping) and honestly fails (returns null/0 + a LastError message)
 * for anything that would require actually reimplementing a nontrivial
 * OpenCV algorithm (Gaussian/median blur, Canny, adaptive threshold,
 * arbitrary-angle rotation, perspective warp, contours, ORB/SIFT,
 * cascades, DNN, video, and any real GUI window) - those are only
 * meaningful against the real shim in ../opencv_shim.cpp.
 *
 * This mirrors the precedent set by tests/en/db/mysql_mock.c (a hand-
 * written C stand-in for libmysqlclient, built by the test runner without
 * a real MySQL server). Keep this file's function signatures in sync with
 * opencv_shim.hpp by hand if that ABI changes - there is no shared header
 * because extern "C" blocks (used there for C++) aren't valid C syntax.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int32_t width;
    int32_t height;
    int32_t channels; /* 1 = gray, 3 = BGR (matches the real shim's internal-BGR
                         convention) */
    unsigned char *data;
} MockMat;

static char g_last_error[512];

static void set_error(const char *msg)
{
    snprintf(g_last_error, sizeof(g_last_error), "%s", msg ? msg : "");
}

static void unsupported(const char *fn)
{
    char buf[512];
    snprintf(buf, sizeof(buf),
             "%s: not implemented in the mock shim (test-only stand-in) - build against "
             "native/opencv_shim.cpp + real OpenCV for this",
             fn);
    set_error(buf);
}

static MockMat *alloc_mat(int32_t w, int32_t h, int32_t ch)
{
    MockMat *m = (MockMat *)malloc(sizeof(MockMat));
    m->width = w;
    m->height = h;
    m->channels = ch;
    m->data = (unsigned char *)calloc((size_t)w * (size_t)h * (size_t)ch, 1);
    return m;
}

static int in_bounds(MockMat *m, int32_t x, int32_t y)
{
    return m && x >= 0 && y >= 0 && x < m->width && y < m->height;
}

static unsigned char clampb(int v)
{
    return v < 0 ? 0 : (v > 255 ? 255 : (unsigned char)v);
}

/* --------------------------------------------------------------- diagnostics */

const char *scv_last_error(void)
{
    return g_last_error;
}
const char *scv_version(void)
{
    return "mock-1.0 (not OpenCV - see opencv_mock.c)";
}

/* ----------------------------------------------------------------- lifecycle */

void *scv_new_blank(int32_t width, int32_t height, int32_t channels, int32_t r, int32_t g,
                    int32_t b)
{
    set_error("");
    if (width <= 0 || height <= 0 || (channels != 1 && channels != 3)) {
        set_error("scv_new_blank: invalid width/height/channels");
        return NULL;
    }
    MockMat *m = alloc_mat(width, height, channels);
    int32_t n = width * height;
    if (channels == 1) {
        unsigned char gray = clampb((r + g + b) / 3);
        memset(m->data, gray, (size_t)n);
    } else {
        for (int32_t i = 0; i < n; i++) {
            m->data[i * 3 + 0] = clampb(b);
            m->data[i * 3 + 1] = clampb(g);
            m->data[i * 3 + 2] = clampb(r);
        }
    }
    return m;
}

void *scv_from_buffer(const uint8_t *data, int32_t width, int32_t height,
                      int32_t channels)
{
    set_error("");
    if (!data || width <= 0 || height <= 0 || (channels != 1 && channels != 3)) {
        set_error("scv_from_buffer: invalid arguments");
        return NULL;
    }
    MockMat *m = alloc_mat(width, height, channels);
    int32_t n = width * height;
    if (channels == 1) {
        memcpy(m->data, data, (size_t)n);
    } else {
        /* input is RGB-interleaved; store BGR internally */
        for (int32_t i = 0; i < n; i++) {
            m->data[i * 3 + 0] = data[i * 3 + 2];
            m->data[i * 3 + 1] = data[i * 3 + 1];
            m->data[i * 3 + 2] = data[i * 3 + 0];
        }
    }
    return m;
}

void *scv_clone(void *mat)
{
    if (!mat) return NULL;
    MockMat *src = (MockMat *)mat;
    MockMat *out = alloc_mat(src->width, src->height, src->channels);
    memcpy(out->data, src->data, (size_t)src->width * src->height * src->channels);
    return out;
}

void scv_free(void *mat)
{
    if (!mat) return;
    MockMat *m = (MockMat *)mat;
    free(m->data);
    free(m);
}

int32_t scv_write(const char *path, void *mat)
{
    set_error("");
    if (!mat || !path) return 0;
    MockMat *m = (MockMat *)mat;
    FILE *f = fopen(path, "wb");
    if (!f) {
        set_error("scv_write: could not open file for writing");
        return 0;
    }
    if (m->channels == 1) {
        fprintf(f, "P5\n%d %d\n255\n", m->width, m->height);
        fwrite(m->data, 1, (size_t)m->width * m->height, f);
    } else {
        fprintf(f, "P6\n%d %d\n255\n", m->width, m->height);
        int32_t n = m->width * m->height;
        for (int32_t i = 0; i < n; i++) {
            unsigned char rgb[3] = {m->data[i * 3 + 2], m->data[i * 3 + 1],
                                    m->data[i * 3 + 0]};
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 1;
}

void *scv_read(const char *path)
{
    set_error("");
    if (!path) return NULL;
    FILE *f = fopen(path, "rb");
    if (!f) {
        set_error("scv_read: could not open file");
        return NULL;
    }
    char magic[3] = {0};
    if (fscanf(f, "%2s", magic) != 1) {
        fclose(f);
        set_error("scv_read: not a PPM/PGM file (mock shim only reads P5/P6)");
        return NULL;
    }
    int32_t w, h, maxval;
    if (fscanf(f, "%d %d %d", &w, &h, &maxval) != 3 || w <= 0 || h <= 0) {
        fclose(f);
        set_error("scv_read: malformed PPM/PGM header");
        return NULL;
    }
    fgetc(f); /* single whitespace byte before binary data */
    int channels = (strcmp(magic, "P6") == 0) ? 3 : (strcmp(magic, "P5") == 0) ? 1 : 0;
    if (channels == 0) {
        fclose(f);
        set_error("scv_read: unsupported format (mock shim only reads P5/P6)");
        return NULL;
    }
    MockMat *m = alloc_mat(w, h, channels);
    size_t got;
    if (channels == 1) {
        got = fread(m->data, 1, (size_t)w * h, f);
        (void)got;
    } else {
        int32_t n = w * h;
        for (int32_t i = 0; i < n; i++) {
            unsigned char rgb[3];
            got = fread(rgb, 1, 3, f);
            (void)got;
            m->data[i * 3 + 0] = rgb[2];
            m->data[i * 3 + 1] = rgb[1];
            m->data[i * 3 + 2] = rgb[0];
        }
    }
    fclose(f);
    return m;
}

/* -------------------------------------------------------------- introspection */

int32_t scv_width(void *mat)
{
    return mat ? ((MockMat *)mat)->width : 0;
}
int32_t scv_height(void *mat)
{
    return mat ? ((MockMat *)mat)->height : 0;
}
int32_t scv_channels(void *mat)
{
    return mat ? ((MockMat *)mat)->channels : 0;
}
int32_t scv_empty(void *mat)
{
    return (!mat || ((MockMat *)mat)->width <= 0 || ((MockMat *)mat)->height <= 0) ? 1
                                                                                   : 0;
}
uint8_t *scv_data(void *mat)
{
    return mat ? ((MockMat *)mat)->data : NULL;
}
int32_t scv_step(void *mat)
{
    if (!mat) return 0;
    MockMat *m = (MockMat *)mat;
    return m->width * m->channels;
}
int32_t scv_mat_total(void *mat)
{
    if (!mat) return 0;
    MockMat *m = (MockMat *)mat;
    return m->width * m->height;
}

int32_t scv_get_pixel(void *mat, int32_t x, int32_t y, int32_t *out_rgb)
{
    MockMat *m = (MockMat *)mat;
    if (!in_bounds(m, x, y) || !out_rgb) return 0;
    int32_t off = (y * m->width + x) * m->channels;
    if (m->channels == 1) {
        out_rgb[0] = out_rgb[1] = out_rgb[2] = m->data[off];
    } else {
        out_rgb[0] = m->data[off + 2];
        out_rgb[1] = m->data[off + 1];
        out_rgb[2] = m->data[off + 0];
    }
    return 1;
}

int32_t scv_set_pixel(void *mat, int32_t x, int32_t y, int32_t r, int32_t g, int32_t b)
{
    MockMat *m = (MockMat *)mat;
    if (!in_bounds(m, x, y)) return 0;
    int32_t off = (y * m->width + x) * m->channels;
    if (m->channels == 1) {
        m->data[off] = clampb((r + g + b) / 3);
    } else {
        m->data[off + 0] = clampb(b);
        m->data[off + 1] = clampb(g);
        m->data[off + 2] = clampb(r);
    }
    return 1;
}

/* ---------------------------------------------------------------- color space */

static MockMat *to_gray(MockMat *m)
{
    if (m->channels == 1) return (MockMat *)scv_clone(m);
    MockMat *out = alloc_mat(m->width, m->height, 1);
    int32_t n = m->width * m->height;
    for (int32_t i = 0; i < n; i++) {
        unsigned char bb = m->data[i * 3 + 0], gg = m->data[i * 3 + 1],
                      rr = m->data[i * 3 + 2];
        out->data[i] = clampb((rr * 299 + gg * 587 + bb * 114) / 1000);
    }
    return out;
}

static MockMat *gray_to_color(MockMat *m)
{
    MockMat *out = alloc_mat(m->width, m->height, 3);
    int32_t n = m->width * m->height;
    for (int32_t i = 0; i < n; i++) {
        out->data[i * 3 + 0] = out->data[i * 3 + 1] = out->data[i * 3 + 2] = m->data[i];
    }
    return out;
}

static void rgb_to_hsv_px(unsigned char r, unsigned char g, unsigned char b,
                          unsigned char *h, unsigned char *s, unsigned char *v)
{
    unsigned char maxc = r > g ? (r > b ? r : b) : (g > b ? g : b);
    unsigned char minc = r < g ? (r < b ? r : b) : (g < b ? g : b);
    int delta = maxc - minc;
    *v = maxc;
    *s = maxc == 0 ? 0 : (unsigned char)((delta * 255) / maxc);
    double hue;
    if (delta == 0) {
        hue = 0;
    } else if (maxc == r) {
        /* The division is hoisted into its own local on purpose - see the
         * comment in hsv_to_rgb_px below. */
        double sextant = (double)(g - b) / delta;
        hue = 60.0 * fmod(sextant, 6.0);
    } else if (maxc == g) {
        hue = 60.0 * (((double)(b - r) / delta) + 2.0);
    } else {
        hue = 60.0 * (((double)(r - g) / delta) + 4.0);
    }
    if (hue < 0) hue += 360.0;
    *h = clampb((int)(hue / 2.0)); /* OpenCV's 0-179 8-bit hue range */
}

static void hsv_to_rgb_px(unsigned char h, unsigned char s, unsigned char v,
                          unsigned char *r, unsigned char *g, unsigned char *b)
{
    double hh = h * 2.0, ss = s / 255.0, vv = v / 255.0;
    double c = vv * ss;
    /*
     * No libc float call in this expression, by hand instead of fmod/fabs.
     * A tcc 0.9.27 win64 build miscompiles a call whose first *two*
     * arguments are both floating point: the first argument is overwritten
     * with the second's value, so `fmod(hh / 60.0, 2.0)` evaluated as
     * `fmod(2.0, 2.0)` == 0. That made x always 0, and every hue in
     * [60,120) came back as (v*(1-s), v, v*(1-s)) instead of (x+m, c+m, m)
     * - the red channel of an HSV round trip lost its value on Windows
     * only, since the test runner builds this shim with whichever of
     * tcc/gcc/cc/clang it finds first. Hoisting the argument into a local
     * fixed it once and then stopped being enough on a later bundled tcc,
     * so the calls are gone rather than shaped around: `hh` is
     * `unsigned char * 2.0`, hence finite and >= 0, which is all the
     * reduction below needs.
     */
    double sextant = hh / 60.0;
    double k = sextant;
    double d;
    double x;
    while (k >= 2.0)
        k -= 2.0;
    d = k - 1.0;
    if (d < 0.0) d = -d;
    x = c * (1.0 - d);
    double m = vv - c;
    double rp, gp, bp;
    if (hh < 60) {
        rp = c;
        gp = x;
        bp = 0;
    } else if (hh < 120) {
        rp = x;
        gp = c;
        bp = 0;
    } else if (hh < 180) {
        rp = 0;
        gp = c;
        bp = x;
    } else if (hh < 240) {
        rp = 0;
        gp = x;
        bp = c;
    } else if (hh < 300) {
        rp = x;
        gp = 0;
        bp = c;
    } else {
        rp = c;
        gp = 0;
        bp = x;
    }
    *r = clampb((int)((rp + m) * 255));
    *g = clampb((int)((gp + m) * 255));
    *b = clampb((int)((bp + m) * 255));
}

void *scv_cvt_color(void *mat, int32_t code)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m) return NULL;
    switch (code) {
    case 0: /* BGR2GRAY */
        return to_gray(m);
    case 1: /* GRAY2BGR */
    case 3: /* GRAY2RGB (achromatic - identical bytes) */
        return m->channels == 1 ? gray_to_color(m) : scv_clone(m);
    case 4:   /* BGR2RGB */
    case 5: { /* RGB2BGR - both are the same byte-swap */
        if (m->channels != 3) return scv_clone(m);
        MockMat *out = alloc_mat(m->width, m->height, 3);
        int32_t n = m->width * m->height;
        for (int32_t i = 0; i < n; i++) {
            out->data[i * 3 + 0] = m->data[i * 3 + 2];
            out->data[i * 3 + 1] = m->data[i * 3 + 1];
            out->data[i * 3 + 2] = m->data[i * 3 + 0];
        }
        return out;
    }
    case 6: { /* BGR2HSV */
        if (m->channels != 3) {
            set_error("scv_cvt_color: BGR2HSV needs a 3-channel Mat");
            return NULL;
        }
        MockMat *out = alloc_mat(m->width, m->height, 3);
        int32_t n = m->width * m->height;
        for (int32_t i = 0; i < n; i++) {
            unsigned char bb = m->data[i * 3 + 0], gg = m->data[i * 3 + 1],
                          rr = m->data[i * 3 + 2];
            rgb_to_hsv_px(rr, gg, bb, &out->data[i * 3 + 0], &out->data[i * 3 + 1],
                          &out->data[i * 3 + 2]);
        }
        return out;
    }
    case 7: { /* HSV2BGR */
        if (m->channels != 3) {
            set_error("scv_cvt_color: HSV2BGR needs a 3-channel Mat");
            return NULL;
        }
        MockMat *out = alloc_mat(m->width, m->height, 3);
        int32_t n = m->width * m->height;
        for (int32_t i = 0; i < n; i++) {
            unsigned char r, g, b;
            hsv_to_rgb_px(m->data[i * 3 + 0], m->data[i * 3 + 1], m->data[i * 3 + 2], &r,
                          &g, &b);
            out->data[i * 3 + 0] = b;
            out->data[i * 3 + 1] = g;
            out->data[i * 3 + 2] = r;
        }
        return out;
    }
    default:
        set_error("scv_cvt_color: color code not implemented in mock shim (HSV RGB-input "
                  "variants 8/9 - use BGR2HSV/HSV2BGR instead)");
        return NULL;
    }
}

/* -------------------------------------------------------------------- geometry */

void *scv_resize(void *mat, int32_t width, int32_t height, int32_t interpolation)
{
    (void)interpolation; /* mock always uses nearest-neighbor */
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || width <= 0 || height <= 0) return NULL;
    MockMat *out = alloc_mat(width, height, m->channels);
    for (int32_t y = 0; y < height; y++) {
        int32_t sy = (int32_t)((int64_t)y * m->height / height);
        if (sy >= m->height) sy = m->height - 1;
        for (int32_t x = 0; x < width; x++) {
            int32_t sx = (int32_t)((int64_t)x * m->width / width);
            if (sx >= m->width) sx = m->width - 1;
            int32_t so = (sy * m->width + sx) * m->channels;
            int32_t doo = (y * width + x) * m->channels;
            memcpy(out->data + doo, m->data + so, (size_t)m->channels);
        }
    }
    return out;
}

void *scv_crop(void *mat, int32_t x, int32_t y, int32_t w, int32_t h)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || w <= 0 || h <= 0 || x < 0 || y < 0 || x + w > m->width ||
        y + h > m->height) {
        set_error("scv_crop: rectangle outside the image");
        return NULL;
    }
    MockMat *out = alloc_mat(w, h, m->channels);
    for (int32_t row = 0; row < h; row++) {
        int32_t so = ((y + row) * m->width + x) * m->channels;
        int32_t doo = row * w * m->channels;
        memcpy(out->data + doo, m->data + so, (size_t)w * m->channels);
    }
    return out;
}

void *scv_rotate90(void *mat, int32_t code)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m) return NULL;
    if (code == 1) { /* 180 */
        MockMat *out = alloc_mat(m->width, m->height, m->channels);
        int32_t n = m->width * m->height;
        for (int32_t i = 0; i < n; i++)
            memcpy(out->data + (n - 1 - i) * m->channels, m->data + i * m->channels,
                   (size_t)m->channels);
        return out;
    }
    /* 90 CW (code 0) or CCW (code 2): dimensions swap */
    MockMat *out = alloc_mat(m->height, m->width, m->channels);
    for (int32_t y = 0; y < m->height; y++) {
        for (int32_t x = 0; x < m->width; x++) {
            int32_t nx, ny;
            if (code == 2) { /* CCW */
                nx = y;
                ny = m->width - 1 - x;
            } else { /* CW */
                nx = m->height - 1 - y;
                ny = x;
            }
            memcpy(out->data + (ny * out->width + nx) * m->channels,
                   m->data + (y * m->width + x) * m->channels, (size_t)m->channels);
        }
    }
    return out;
}

void *scv_flip(void *mat, int32_t mode)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m) return NULL;
    MockMat *out = alloc_mat(m->width, m->height, m->channels);
    for (int32_t y = 0; y < m->height; y++) {
        int32_t sy =
            (mode == 1) ? y
                        : (m->height - 1 - y); /* 1 = horizontal only -> rows unchanged */
        for (int32_t x = 0; x < m->width; x++) {
            int32_t sx =
                (mode == 0)
                    ? x
                    : (m->width - 1 - x); /* 0 = vertical only -> cols unchanged */
            memcpy(out->data + (y * m->width + x) * m->channels,
                   m->data + (sy * m->width + sx) * m->channels, (size_t)m->channels);
        }
    }
    return out;
}

/* rotate_angle: arbitrary-angle rotation isn't worth reimplementing for a
 * test mock; real geometry is exercised via resize/crop/rotate90/flip
 * above, all of which are exact and image-independent. */
void *scv_rotate_angle(void *mat, double angle_deg, double scale)
{
    (void)mat;
    (void)angle_deg;
    (void)scale;
    unsupported("scv_rotate_angle");
    return NULL;
}

/* --------------------------------------------------------------------- filters */

void *scv_blur(void *mat, int32_t ksize)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || ksize <= 0) return NULL;
    int32_t half = ksize / 2;
    MockMat *out = alloc_mat(m->width, m->height, m->channels);
    for (int32_t y = 0; y < m->height; y++) {
        for (int32_t x = 0; x < m->width; x++) {
            for (int32_t c = 0; c < m->channels; c++) {
                int32_t sum = 0, cnt = 0;
                for (int32_t dy = -half; dy <= half; dy++) {
                    int32_t sy = y + dy;
                    if (sy < 0 || sy >= m->height) continue;
                    for (int32_t dx = -half; dx <= half; dx++) {
                        int32_t sx = x + dx;
                        if (sx < 0 || sx >= m->width) continue;
                        sum += m->data[(sy * m->width + sx) * m->channels + c];
                        cnt++;
                    }
                }
                out->data[(y * m->width + x) * m->channels + c] =
                    clampb(sum / (cnt > 0 ? cnt : 1));
            }
        }
    }
    return out;
}

void *scv_gaussian_blur(void *mat, int32_t ksize, double sigma)
{
    (void)mat;
    (void)ksize;
    (void)sigma;
    unsupported("scv_gaussian_blur");
    return NULL;
}

void *scv_median_blur(void *mat, int32_t ksize)
{
    (void)mat;
    (void)ksize;
    unsupported("scv_median_blur");
    return NULL;
}

void *scv_canny(void *mat, double t1, double t2)
{
    (void)mat;
    (void)t1;
    (void)t2;
    unsupported("scv_canny");
    return NULL;
}

void *scv_threshold(void *mat, double thresh, double maxval, int32_t kind)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m) return NULL;
    if (kind == 5) { /* OTSU */
        unsupported("scv_threshold (OTSU)");
        return NULL;
    }
    MockMat *gray = to_gray(m);
    MockMat *out = alloc_mat(gray->width, gray->height, 1);
    int32_t n = gray->width * gray->height;
    for (int32_t i = 0; i < n; i++) {
        int v = gray->data[i];
        int out_v;
        switch (kind) {
        case 1:
            out_v = v > thresh ? 0 : (int)maxval;
            break; /* BINARY_INV */
        case 2:
            out_v = v > thresh ? (int)thresh : v;
            break; /* TRUNC */
        case 3:
            out_v = v > thresh ? v : 0;
            break; /* TOZERO */
        case 4:
            out_v = v > thresh ? 0 : v;
            break; /* TOZERO_INV */
        default:
            out_v = v > thresh ? (int)maxval : 0;
            break; /* BINARY */
        }
        out->data[i] = clampb(out_v);
    }
    scv_free(gray);
    return out;
}

void *scv_adaptive_threshold(void *mat, double maxval, int32_t method, int32_t kind,
                             int32_t block_size, double c)
{
    (void)mat;
    (void)maxval;
    (void)method;
    (void)kind;
    (void)block_size;
    (void)c;
    unsupported("scv_adaptive_threshold");
    return NULL;
}

int32_t scv_hist_gray(void *mat, int32_t *out_counts)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || !out_counts) return 0;
    memset(out_counts, 0, sizeof(int32_t) * 256);
    MockMat *gray = to_gray(m);
    int32_t n = gray->width * gray->height;
    for (int32_t i = 0; i < n; i++)
        out_counts[gray->data[i]]++;
    scv_free(gray);
    return 1;
}

/* ---------------------------------------------------------------------- drawing */

int32_t scv_line(void *mat, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r,
                 int32_t g, int32_t b, int32_t thickness)
{
    (void)thickness; /* mock always draws a 1px line */
    MockMat *m = (MockMat *)mat;
    if (!m) return 0;
    int32_t dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int32_t dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int32_t err = dx + dy;
    int32_t x = x1, y = y1;
    while (1) {
        scv_set_pixel(m, x, y, r, g, b);
        if (x == x2 && y == y2) break;
        int32_t e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y += sy;
        }
    }
    return 1;
}

int32_t scv_rectangle(void *mat, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r,
                      int32_t g, int32_t b, int32_t thickness)
{
    MockMat *m = (MockMat *)mat;
    if (!m) return 0;
    if (thickness < 0) { /* filled */
        for (int32_t yy = y; yy < y + h; yy++)
            for (int32_t xx = x; xx < x + w; xx++)
                scv_set_pixel(m, xx, yy, r, g, b);
        return 1;
    }
    scv_line(m, x, y, x + w - 1, y, r, g, b, 1);
    scv_line(m, x, y + h - 1, x + w - 1, y + h - 1, r, g, b, 1);
    scv_line(m, x, y, x, y + h - 1, r, g, b, 1);
    scv_line(m, x + w - 1, y, x + w - 1, y + h - 1, r, g, b, 1);
    return 1;
}

int32_t scv_circle(void *mat, int32_t cx, int32_t cy, int32_t radius, int32_t r,
                   int32_t g, int32_t b, int32_t thickness)
{
    MockMat *m = (MockMat *)mat;
    if (!m) return 0;
    for (int32_t y = cy - radius; y <= cy + radius; y++) {
        for (int32_t x = cx - radius; x <= cx + radius; x++) {
            double dist = sqrt((double)(x - cx) * (x - cx) + (double)(y - cy) * (y - cy));
            if (thickness < 0) {
                if (dist <= radius) scv_set_pixel(m, x, y, r, g, b);
            } else if (fabs(dist - radius) < 0.75) {
                scv_set_pixel(m, x, y, r, g, b);
            }
        }
    }
    return 1;
}

int32_t scv_ellipse(void *mat, int32_t cx, int32_t cy, int32_t axis_w, int32_t axis_h,
                    double angle, int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    /* Approximated as an axis-aligned ellipse via a coarse point walk;
     * arbitrary rotation isn't worth reimplementing for a test mock (see
     * scv_rotate_angle's own note) - real angle support comes from the
     * OpenCV-backed shim. */
    (void)angle;
    MockMat *m = (MockMat *)mat;
    if (!m || axis_w <= 0 || axis_h <= 0) return 0;
    for (int32_t y = cy - axis_h; y <= cy + axis_h; y++) {
        for (int32_t x = cx - axis_w; x <= cx + axis_w; x++) {
            double dx = (double)(x - cx) / axis_w;
            double dy = (double)(y - cy) / axis_h;
            double v = dx * dx + dy * dy;
            if (thickness < 0) {
                if (v <= 1.0) scv_set_pixel(m, x, y, r, g, b);
            } else if (fabs(v - 1.0) < 0.08) {
                scv_set_pixel(m, x, y, r, g, b);
            }
        }
    }
    return 1;
}

int32_t scv_polylines(void *mat, const int32_t *pts_xy, int32_t count, int32_t closed,
                      int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    MockMat *m = (MockMat *)mat;
    if (!m || !pts_xy || count < 2) return 0;
    for (int32_t i = 0; i < count - 1; i++)
        scv_line(m, pts_xy[i * 2], pts_xy[i * 2 + 1], pts_xy[(i + 1) * 2],
                 pts_xy[(i + 1) * 2 + 1], r, g, b, thickness);
    if (closed)
        scv_line(m, pts_xy[(count - 1) * 2], pts_xy[(count - 1) * 2 + 1], pts_xy[0],
                 pts_xy[1], r, g, b, thickness);
    return 1;
}

int32_t scv_fill_poly(void *mat, const int32_t *pts_xy, int32_t count, int32_t r,
                      int32_t g, int32_t b)
{
    /* Simple scanline-free approximation: fill the bounding box, clipped
     * to a coarse point-in-polygon test - adequate for a test mock. */
    MockMat *m = (MockMat *)mat;
    if (!m || !pts_xy || count < 3) return 0;
    int32_t min_x = pts_xy[0], max_x = pts_xy[0], min_y = pts_xy[1], max_y = pts_xy[1];
    for (int32_t i = 1; i < count; i++) {
        if (pts_xy[i * 2] < min_x) min_x = pts_xy[i * 2];
        if (pts_xy[i * 2] > max_x) max_x = pts_xy[i * 2];
        if (pts_xy[i * 2 + 1] < min_y) min_y = pts_xy[i * 2 + 1];
        if (pts_xy[i * 2 + 1] > max_y) max_y = pts_xy[i * 2 + 1];
    }
    for (int32_t y = min_y; y <= max_y; y++) {
        for (int32_t x = min_x; x <= max_x; x++) {
            int32_t inside = 0;
            for (int32_t i = 0, j = count - 1; i < count; j = i++) {
                int32_t xi = pts_xy[i * 2], yi = pts_xy[i * 2 + 1];
                int32_t xj = pts_xy[j * 2], yj = pts_xy[j * 2 + 1];
                if (((yi > y) != (yj > y)) &&
                    (x < (xj - xi) * (y - yi) / (double)(yj - yi) + xi))
                    inside = !inside;
            }
            if (inside) scv_set_pixel(m, x, y, r, g, b);
        }
    }
    return 1;
}

int32_t scv_put_text(void *mat, const char *text, int32_t x, int32_t y, double font_scale,
                     int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    (void)mat;
    (void)text;
    (void)x;
    (void)y;
    (void)font_scale;
    (void)r;
    (void)g;
    (void)b;
    (void)thickness;
    unsupported("scv_put_text");
    return 0;
}

/* ------------------------------------------------------------------ morphology */

void *scv_erode(void *mat, int32_t ksize, int32_t iterations)
{
    (void)iterations; /* mock applies a single pass regardless of iteration count */
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || ksize <= 0) return NULL;
    int32_t half = ksize / 2;
    MockMat *out = alloc_mat(m->width, m->height, m->channels);
    for (int32_t y = 0; y < m->height; y++) {
        for (int32_t x = 0; x < m->width; x++) {
            for (int32_t c = 0; c < m->channels; c++) {
                int min_v = 255;
                for (int32_t dy = -half; dy <= half; dy++) {
                    int32_t sy = y + dy;
                    if (sy < 0 || sy >= m->height) continue;
                    for (int32_t dx = -half; dx <= half; dx++) {
                        int32_t sx = x + dx;
                        if (sx < 0 || sx >= m->width) continue;
                        int v = m->data[(sy * m->width + sx) * m->channels + c];
                        if (v < min_v) min_v = v;
                    }
                }
                out->data[(y * m->width + x) * m->channels + c] = clampb(min_v);
            }
        }
    }
    return out;
}

void *scv_dilate(void *mat, int32_t ksize, int32_t iterations)
{
    (void)iterations;
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m || ksize <= 0) return NULL;
    int32_t half = ksize / 2;
    MockMat *out = alloc_mat(m->width, m->height, m->channels);
    for (int32_t y = 0; y < m->height; y++) {
        for (int32_t x = 0; x < m->width; x++) {
            for (int32_t c = 0; c < m->channels; c++) {
                int max_v = 0;
                for (int32_t dy = -half; dy <= half; dy++) {
                    int32_t sy = y + dy;
                    if (sy < 0 || sy >= m->height) continue;
                    for (int32_t dx = -half; dx <= half; dx++) {
                        int32_t sx = x + dx;
                        if (sx < 0 || sx >= m->width) continue;
                        int v = m->data[(sy * m->width + sx) * m->channels + c];
                        if (v > max_v) max_v = v;
                    }
                }
                out->data[(y * m->width + x) * m->channels + c] = clampb(max_v);
            }
        }
    }
    return out;
}

void *scv_morphology(void *mat, int32_t op, int32_t ksize)
{
    /* OPEN = erode then dilate; CLOSE = dilate then erode; the rest
     * (GRADIENT/TOPHAT/BLACKHAT) need both a base image and one of those
     * results, which is more than this mock's scope - the real shim
     * covers them all faithfully via cv::morphologyEx. */
    if (op == 1) { /* MORPH_CLOSE */
        void *d = scv_dilate(mat, ksize, 1);
        if (!d) return NULL;
        void *e = scv_erode(d, ksize, 1);
        scv_free(d);
        return e;
    }
    if (op == 0) { /* MORPH_OPEN */
        void *e = scv_erode(mat, ksize, 1);
        if (!e) return NULL;
        void *d = scv_dilate(e, ksize, 1);
        scv_free(e);
        return d;
    }
    (void)ksize;
    unsupported("scv_morphology (GRADIENT/TOPHAT/BLACKHAT)");
    return NULL;
}

/* --------------------------------------------------------------- arithmetic */

static int32_t same_shape(MockMat *a, MockMat *b)
{
    return a && b && a->width == b->width && a->height == b->height &&
           a->channels == b->channels;
}

void *scv_add_weighted(void *mat1, double alpha, void *mat2, double beta, double gamma)
{
    set_error("");
    MockMat *m1 = (MockMat *)mat1, *m2 = (MockMat *)mat2;
    if (!same_shape(m1, m2)) {
        set_error("scv_add_weighted: mismatched Mat shapes");
        return NULL;
    }
    MockMat *out = alloc_mat(m1->width, m1->height, m1->channels);
    int32_t n = m1->width * m1->height * m1->channels;
    for (int32_t i = 0; i < n; i++)
        out->data[i] = clampb((int)(m1->data[i] * alpha + m2->data[i] * beta + gamma));
    return out;
}

void *scv_bitwise(void *mat1, void *mat2, int32_t op)
{
    set_error("");
    MockMat *m1 = (MockMat *)mat1, *m2 = (MockMat *)mat2;
    if (!same_shape(m1, m2)) {
        set_error("scv_bitwise: mismatched Mat shapes");
        return NULL;
    }
    MockMat *out = alloc_mat(m1->width, m1->height, m1->channels);
    int32_t n = m1->width * m1->height * m1->channels;
    for (int32_t i = 0; i < n; i++) {
        switch (op) {
        case 1:
            out->data[i] = m1->data[i] | m2->data[i];
            break;
        case 2:
            out->data[i] = m1->data[i] ^ m2->data[i];
            break;
        default:
            out->data[i] = m1->data[i] & m2->data[i];
            break;
        }
    }
    return out;
}

void *scv_bitwise_not(void *mat)
{
    set_error("");
    MockMat *m = (MockMat *)mat;
    if (!m) return NULL;
    MockMat *out = alloc_mat(m->width, m->height, m->channels);
    int32_t n = m->width * m->height * m->channels;
    for (int32_t i = 0; i < n; i++)
        out->data[i] = (unsigned char)(255 - m->data[i]);
    return out;
}

void *scv_abs_diff(void *mat1, void *mat2)
{
    set_error("");
    MockMat *m1 = (MockMat *)mat1, *m2 = (MockMat *)mat2;
    if (!same_shape(m1, m2)) {
        set_error("scv_abs_diff: mismatched Mat shapes");
        return NULL;
    }
    MockMat *out = alloc_mat(m1->width, m1->height, m1->channels);
    int32_t n = m1->width * m1->height * m1->channels;
    for (int32_t i = 0; i < n; i++) {
        int d = (int)m1->data[i] - (int)m2->data[i];
        out->data[i] = (unsigned char)(d < 0 ? -d : d);
    }
    return out;
}

/* --------------------------------------------------- perspective / template */

void *scv_warp_perspective_quad(void *mat, const double *src_xy, const double *dst_xy,
                                int32_t out_width, int32_t out_height)
{
    /* A true perspective warp (3x3 homography solve) is out of scope for
     * this mock; real usage is verified against the OpenCV-backed shim.
     * This still exercises the Salam-side call/marshaling path. */
    (void)src_xy;
    (void)dst_xy;
    (void)mat;
    unsupported("scv_warp_perspective_quad");
    (void)out_width;
    (void)out_height;
    return NULL;
}

int32_t scv_match_template(void *mat, void *tmpl, int32_t *out_x, int32_t *out_y,
                           double *out_score)
{
    /* Exact (non-normalized) brute-force SSD match - correct for a small
     * test mock, just not calibrated to OpenCV's TM_CCOEFF_NORMED score
     * range (out_score here is a placeholder 1.0 on any match). */
    set_error("");
    MockMat *m = (MockMat *)mat;
    MockMat *t = (MockMat *)tmpl;
    if (!m || !t || !out_x || !out_y || !out_score) return 0;
    if (t->width > m->width || t->height > m->height || t->channels != m->channels) {
        set_error("scv_match_template: template is larger than the search image");
        return 0;
    }
    int64_t best = -1;
    int32_t best_x = 0, best_y = 0;
    for (int32_t y = 0; y <= m->height - t->height; y++) {
        for (int32_t x = 0; x <= m->width - t->width; x++) {
            int64_t sum = 0;
            for (int32_t ty = 0; ty < t->height; ty++) {
                for (int32_t tx = 0; tx < t->width; tx++) {
                    for (int32_t c = 0; c < t->channels; c++) {
                        int a =
                            m->data[((y + ty) * m->width + (x + tx)) * m->channels + c];
                        int b = t->data[(ty * t->width + tx) * t->channels + c];
                        int64_t d = a - b;
                        sum += d * d;
                    }
                }
            }
            if (best < 0 || sum < best) {
                best = sum;
                best_x = x;
                best_y = y;
            }
        }
    }
    *out_x = best_x;
    *out_y = best_y;
    *out_score = 1.0;
    return 1;
}

/* ------------------------------------------------------------------- contours */

void *scv_find_contours(void *mat)
{
    (void)mat;
    unsupported("scv_find_contours");
    return NULL;
}
int32_t scv_contour_count(void *cs)
{
    (void)cs;
    return 0;
}
int32_t scv_contour_point_count(void *cs, int32_t idx)
{
    (void)cs;
    (void)idx;
    return 0;
}
int32_t scv_contour_point_get(void *cs, int32_t idx, int32_t pt_idx, int32_t *out_xy)
{
    (void)cs;
    (void)idx;
    (void)pt_idx;
    (void)out_xy;
    return 0;
}
double scv_contour_area(void *cs, int32_t idx)
{
    (void)cs;
    (void)idx;
    return 0.0;
}
int32_t scv_contour_bounding_rect(void *cs, int32_t idx, int32_t *out_xywh)
{
    (void)cs;
    (void)idx;
    (void)out_xywh;
    return 0;
}
int32_t scv_draw_contour(void *mat, void *cs, int32_t idx, int32_t r, int32_t g,
                         int32_t b, int32_t thickness)
{
    (void)mat;
    (void)cs;
    (void)idx;
    (void)r;
    (void)g;
    (void)b;
    (void)thickness;
    return 0;
}
void scv_free_contours(void *cs)
{
    (void)cs;
}

/* -------------------------------------------------------------------- features */

int32_t scv_good_features(void *mat, int32_t max_corners, double quality, double min_dist,
                          int32_t *out_x, int32_t *out_y)
{
    (void)mat;
    (void)max_corners;
    (void)quality;
    (void)min_dist;
    (void)out_x;
    (void)out_y;
    unsupported("scv_good_features");
    return 0;
}

void *scv_orb_detect(void *mat, int32_t max_features)
{
    (void)mat;
    (void)max_features;
    unsupported("scv_orb_detect");
    return NULL;
}

void *scv_sift_detect(void *mat, int32_t max_features)
{
    (void)mat;
    (void)max_features;
    unsupported("scv_sift_detect");
    return NULL;
}

int32_t scv_keypoint_count(void *kp)
{
    (void)kp;
    return 0;
}

int32_t scv_keypoint_get(void *kp, int32_t idx, double *out_xy)
{
    (void)kp;
    (void)idx;
    (void)out_xy;
    return 0;
}

void *scv_orb_descriptors(void *kp)
{
    (void)kp;
    unsupported("scv_orb_descriptors");
    return NULL;
}

void scv_free_keypoints(void *kp)
{
    (void)kp;
}

void *scv_bf_match(void *desc1, void *desc2, int32_t max_matches, int32_t norm)
{
    (void)desc1;
    (void)desc2;
    (void)max_matches;
    (void)norm;
    unsupported("scv_bf_match");
    return NULL;
}

int32_t scv_match_count(void *m)
{
    (void)m;
    return 0;
}

int32_t scv_match_get(void *m, int32_t idx, int32_t *out_query, int32_t *out_train,
                      double *out_dist)
{
    (void)m;
    (void)idx;
    (void)out_query;
    (void)out_train;
    (void)out_dist;
    return 0;
}

void scv_free_matches(void *m)
{
    (void)m;
}

/* -------------------------------------------------------------- object detection */

void *scv_cascade_load(const char *path)
{
    (void)path;
    unsupported("scv_cascade_load");
    return NULL;
}

void scv_cascade_free(void *c)
{
    (void)c;
}

int32_t scv_cascade_detect(void *c, void *mat, double scale_factor, int32_t min_neighbors,
                           int32_t max_results, int32_t *out_x, int32_t *out_y,
                           int32_t *out_w, int32_t *out_h)
{
    (void)c;
    (void)mat;
    (void)scale_factor;
    (void)min_neighbors;
    (void)max_results;
    (void)out_x;
    (void)out_y;
    (void)out_w;
    (void)out_h;
    unsupported("scv_cascade_detect");
    return 0;
}

/* ------------------------------------------------------------------------ dnn */

void *scv_dnn_load(const char *model_path, const char *config_path)
{
    (void)model_path;
    (void)config_path;
    unsupported("scv_dnn_load");
    return NULL;
}
void scv_dnn_free(void *net)
{
    (void)net;
}
int32_t scv_dnn_set_input(void *net, void *mat, double scale, double mean_r,
                          double mean_g, double mean_b, int32_t swap_rb, int32_t width,
                          int32_t height)
{
    (void)net;
    (void)mat;
    (void)scale;
    (void)mean_r;
    (void)mean_g;
    (void)mean_b;
    (void)swap_rb;
    (void)width;
    (void)height;
    return 0;
}
void *scv_dnn_forward(void *net)
{
    (void)net;
    return NULL;
}
void *scv_dnn_forward_named(void *net, const char *layer_name)
{
    (void)net;
    (void)layer_name;
    return NULL;
}

/* ---------------------------------------------------------------------- video */

void *scv_video_open_file(const char *path)
{
    (void)path;
    unsupported("scv_video_open_file");
    return NULL;
}
void *scv_video_open_camera(int32_t index)
{
    (void)index;
    unsupported("scv_video_open_camera");
    return NULL;
}
void *scv_video_read(void *v)
{
    (void)v;
    return NULL;
}
void scv_video_release(void *v)
{
    (void)v;
}
int32_t scv_video_ok(void *v)
{
    (void)v;
    return 0;
}
double scv_video_fps(void *v)
{
    (void)v;
    return 0.0;
}
int32_t scv_video_frame_count(void *v)
{
    (void)v;
    return 0;
}
void *scv_writer_open(const char *path, const char *fourcc, double fps, int32_t width,
                      int32_t height)
{
    (void)path;
    (void)fourcc;
    (void)fps;
    (void)width;
    (void)height;
    unsupported("scv_writer_open");
    return NULL;
}
int32_t scv_writer_write(void *w, void *frame)
{
    (void)w;
    (void)frame;
    return 0;
}
void scv_writer_release(void *w)
{
    (void)w;
}

/* ------------------------------------------------------------------------ gui */

void scv_imshow(const char *window, void *mat)
{
    (void)window;
    (void)mat;
    unsupported("scv_imshow");
}
int32_t scv_wait_key(int32_t delay_ms)
{
    (void)delay_ms;
    return -1;
}
void scv_destroy_window(const char *window)
{
    (void)window;
}
void scv_destroy_all_windows(void)
{
}

/* Trackbars are pure bookkeeping (no real window to attach to without a
 * GUI backend), so the mock tracks positions itself - enough to exercise
 * CreateTrackbar/GetTrackbarPos/SetTrackbarPos's Salam-side plumbing. */
#define SCV_MAX_TRACKBARS 32
static char g_trackbar_keys[SCV_MAX_TRACKBARS][256];
static int32_t g_trackbar_vals[SCV_MAX_TRACKBARS];
static int32_t g_trackbar_count = 0;

static int32_t trackbar_find(const char *name, const char *window)
{
    char key[256];
    snprintf(key, sizeof(key), "%s::%s", window, name);
    for (int32_t i = 0; i < g_trackbar_count; i++)
        if (strcmp(g_trackbar_keys[i], key) == 0) return i;
    return -1;
}

int32_t scv_create_trackbar(const char *name, const char *window, int32_t initial,
                            int32_t max)
{
    (void)max;
    if (!name || !window || g_trackbar_count >= SCV_MAX_TRACKBARS) return 0;
    int32_t i = trackbar_find(name, window);
    if (i < 0) {
        i = g_trackbar_count++;
        snprintf(g_trackbar_keys[i], sizeof(g_trackbar_keys[i]), "%s::%s", window, name);
    }
    g_trackbar_vals[i] = initial;
    return 1;
}

int32_t scv_get_trackbar_pos(const char *name, const char *window)
{
    int32_t i = trackbar_find(name, window);
    return i < 0 ? 0 : g_trackbar_vals[i];
}

void scv_set_trackbar_pos(const char *name, const char *window, int32_t pos)
{
    int32_t i = trackbar_find(name, window);
    if (i >= 0) g_trackbar_vals[i] = pos;
}

// Salam Programming Language (2024-2026) - std/opencv native shim
//
// Implements the ABI declared in opencv_shim.hpp against real OpenCV 4.x.
// See that header for the ownership/channel-order/error-handling contract
// this file must uphold, and BUILD.md for compiling it.
//
// Every exported function catches every exception at its own boundary (no
// C++ exception may ever unwind across the extern "C" ABI into Salam) and
// reports failure through its return value, recording a human-readable
// message via set_error() that Salam's LastError() can retrieve.

#include "opencv_shim.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/dnn.hpp>

#include <algorithm>
#include <cstring>
#include <map>
#include <string>
#include <vector>

namespace
{

thread_local std::string g_last_error;

void set_error(const std::string &msg)
{
    g_last_error = msg;
}

void clear_error()
{
    g_last_error.clear();
}

cv::Mat *as_mat(cv_mat_t *m)
{
    return static_cast<cv::Mat *>(m);
}

cv_mat_t *wrap(cv::Mat m)
{
    return new cv::Mat(std::move(m));
}

// Bundles ORB keypoints with their descriptors, matching how
// cv::ORB::detectAndCompute produces (and every consumer needs) them
// together. Owned as one opaque handle on the Salam side (Keypoints).
struct KpBundle {
    std::vector<cv::KeyPoint> kps;
    cv::Mat descriptors;
};

KpBundle *as_kp(cv_keypoints_t *k)
{
    return static_cast<KpBundle *>(k);
}

std::vector<cv::DMatch> *as_matches(cv_matches_t *m)
{
    return static_cast<std::vector<cv::DMatch> *>(m);
}

struct ContoursBundle {
    std::vector<std::vector<cv::Point>> contours;
};

ContoursBundle *as_contours(cv_contours_t *c)
{
    return static_cast<ContoursBundle *>(c);
}

cv::dnn::Net *as_net(cv_net_t *n)
{
    return static_cast<cv::dnn::Net *>(n);
}

// Returns a single-channel view of `m`, converting BGR->GRAY only if
// needed - used by algorithms (corners, ORB, cascades, hist) that operate
// on intensity alone. Never mutates `m`.
cv::Mat to_gray_view(const cv::Mat &m)
{
    if (m.channels() == 1) return m;
    cv::Mat gray;
    cv::cvtColor(m, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

// Maps this package's own COLOR_* constants (opencv.salam) to OpenCV's.
// Kept as an explicit table, not a passthrough, so an OpenCV upgrade that
// renumbers cv::ColorConversionCodes can never silently change behavior
// here - only this switch needs to move with it.
int map_color_code(int32_t code, bool *ok)
{
    *ok = true;
    switch (code) {
    case 0:
        return cv::COLOR_BGR2GRAY;
    case 1:
        return cv::COLOR_GRAY2BGR;
    case 2:
        return cv::COLOR_RGB2GRAY;
    case 3:
        return cv::COLOR_GRAY2RGB;
    case 4:
        return cv::COLOR_BGR2RGB;
    case 5:
        return cv::COLOR_RGB2BGR;
    case 6:
        return cv::COLOR_BGR2HSV;
    case 7:
        return cv::COLOR_HSV2BGR;
    case 8:
        return cv::COLOR_RGB2HSV;
    case 9:
        return cv::COLOR_HSV2RGB;
    default:
        *ok = false;
        return 0;
    }
}

int map_interpolation(int32_t code)
{
    switch (code) {
    case 0:
        return cv::INTER_NEAREST;
    case 2:
        return cv::INTER_CUBIC;
    case 3:
        return cv::INTER_AREA;
    case 4:
        return cv::INTER_LANCZOS4;
    default:
        return cv::INTER_LINEAR; // 1, and any unrecognized value
    }
}

int map_rotate90(int32_t code)
{
    switch (code) {
    case 1:
        return cv::ROTATE_180;
    case 2:
        return cv::ROTATE_90_COUNTERCLOCKWISE;
    default:
        return cv::ROTATE_90_CLOCKWISE; // 0
    }
}

int map_flip(int32_t mode)
{
    switch (mode) {
    case 1:
        return 1; // FLIP_HORIZONTAL
    case 2:
        return -1; // FLIP_BOTH
    default:
        return 0; // FLIP_VERTICAL
    }
}

int map_thresh(int32_t kind, bool *otsu)
{
    *otsu = (kind == 5);
    switch (kind) {
    case 1:
        return cv::THRESH_BINARY_INV;
    case 2:
        return cv::THRESH_TRUNC;
    case 3:
        return cv::THRESH_TOZERO;
    case 4:
        return cv::THRESH_TOZERO_INV;
    default:
        return cv::THRESH_BINARY; // 0, and OTSU (5) rides on BINARY
    }
}

// BGR fill/draw scalar for a given RGB triple and the target Mat's channel
// count (1 = gray, averaged; 3 = BGR, swapped from the RGB the caller gave
// us - see opencv_shim.hpp's channel-order contract).
cv::Scalar rgb_scalar(int32_t r, int32_t g, int32_t b, int channels)
{
    if (channels == 1) return cv::Scalar((r + g + b) / 3);
    return cv::Scalar(b, g, r);
}

int map_morph_op(int32_t op)
{
    switch (op) {
    case 1:
        return cv::MORPH_CLOSE;
    case 2:
        return cv::MORPH_GRADIENT;
    case 3:
        return cv::MORPH_TOPHAT;
    case 4:
        return cv::MORPH_BLACKHAT;
    default:
        return cv::MORPH_OPEN; // 0
    }
}

int map_norm(int32_t norm)
{
    return norm == 1 ? cv::NORM_L2 : cv::NORM_HAMMING;
}

} // namespace

// ---------------------------------------------------------------- diagnostics

const char *scv_last_error()
{
    return g_last_error.c_str();
}

const char *scv_version()
{
    static const std::string v = CV_VERSION;
    return v.c_str();
}

// ------------------------------------------------------------------ lifecycle

cv_mat_t *scv_read(const char *path)
{
    clear_error();
    try {
        cv::Mat m = cv::imread(path, cv::IMREAD_COLOR);
        if (m.empty()) {
            set_error(std::string("failed to read image: ") + (path ? path : "(null)"));
            return nullptr;
        }
        return wrap(std::move(m));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_write(const char *path, cv_mat_t *mat)
{
    clear_error();
    if (!mat) {
        set_error("scv_write: null mat");
        return 0;
    }
    try {
        return cv::imwrite(path, *as_mat(mat)) ? 1 : 0;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

void scv_free(cv_mat_t *mat)
{
    delete as_mat(mat);
}

cv_mat_t *scv_clone(cv_mat_t *mat)
{
    if (!mat) return nullptr;
    return wrap(as_mat(mat)->clone());
}

cv_mat_t *scv_new_blank(int32_t width, int32_t height, int32_t channels, int32_t r,
                        int32_t g, int32_t b)
{
    clear_error();
    if (width <= 0 || height <= 0 || (channels != 1 && channels != 3)) {
        set_error("scv_new_blank: invalid width/height/channels");
        return nullptr;
    }
    try {
        int type = channels == 1 ? CV_8UC1 : CV_8UC3;
        cv::Mat m(height, width, type, rgb_scalar(r, g, b, channels));
        return wrap(std::move(m));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_from_buffer(const uint8_t *data, int32_t width, int32_t height,
                          int32_t channels)
{
    clear_error();
    if (!data || width <= 0 || height <= 0 || (channels != 1 && channels != 3)) {
        set_error("scv_from_buffer: invalid arguments");
        return nullptr;
    }
    try {
        if (channels == 1) {
            cv::Mat view(height, width, CV_8UC1, const_cast<uint8_t *>(data));
            return wrap(view.clone());
        }
        // `data` is RGB-interleaved (this package's boundary convention);
        // store BGR internally like every other 3-channel Mat here.
        cv::Mat rgb_view(height, width, CV_8UC3, const_cast<uint8_t *>(data));
        cv::Mat bgr;
        cv::cvtColor(rgb_view, bgr, cv::COLOR_RGB2BGR);
        return wrap(std::move(bgr));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// --------------------------------------------------------------- introspection

int32_t scv_width(cv_mat_t *mat)
{
    return mat ? as_mat(mat)->cols : 0;
}
int32_t scv_height(cv_mat_t *mat)
{
    return mat ? as_mat(mat)->rows : 0;
}
int32_t scv_channels(cv_mat_t *mat)
{
    return mat ? as_mat(mat)->channels() : 0;
}
int32_t scv_empty(cv_mat_t *mat)
{
    return (!mat || as_mat(mat)->empty()) ? 1 : 0;
}
uint8_t *scv_data(cv_mat_t *mat)
{
    return mat ? as_mat(mat)->data : nullptr;
}
int32_t scv_step(cv_mat_t *mat)
{
    return mat ? static_cast<int32_t>(as_mat(mat)->step[0]) : 0;
}
int32_t scv_mat_total(cv_mat_t *mat)
{
    return mat ? static_cast<int32_t>(as_mat(mat)->total()) : 0;
}

int32_t scv_get_pixel(cv_mat_t *mat, int32_t x, int32_t y, int32_t *out_rgb)
{
    if (!mat || !out_rgb) return 0;
    cv::Mat *m = as_mat(mat);
    if (x < 0 || y < 0 || x >= m->cols || y >= m->rows) return 0;
    if (m->channels() == 1) {
        uint8_t v = m->at<uint8_t>(y, x);
        out_rgb[0] = v;
        out_rgb[1] = v;
        out_rgb[2] = v;
    } else {
        cv::Vec3b px = m->at<cv::Vec3b>(y, x); // BGR
        out_rgb[0] = px[2];
        out_rgb[1] = px[1];
        out_rgb[2] = px[0];
    }
    return 1;
}

int32_t scv_set_pixel(cv_mat_t *mat, int32_t x, int32_t y, int32_t r, int32_t g,
                      int32_t b)
{
    if (!mat) return 0;
    cv::Mat *m = as_mat(mat);
    if (x < 0 || y < 0 || x >= m->cols || y >= m->rows) return 0;
    if (m->channels() == 1) {
        m->at<uint8_t>(y, x) = static_cast<uint8_t>((r + g + b) / 3);
    } else {
        m->at<cv::Vec3b>(y, x) = cv::Vec3b(
            static_cast<uint8_t>(b), static_cast<uint8_t>(g), static_cast<uint8_t>(r));
    }
    return 1;
}

// ----------------------------------------------------------------- color space

cv_mat_t *scv_cvt_color(cv_mat_t *mat, int32_t code)
{
    clear_error();
    if (!mat) return nullptr;
    bool ok = false;
    int cvcode = map_color_code(code, &ok);
    if (!ok) {
        set_error("scv_cvt_color: unknown color code");
        return nullptr;
    }
    try {
        cv::Mat out;
        cv::cvtColor(*as_mat(mat), out, cvcode);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// -------------------------------------------------------------------- geometry

cv_mat_t *scv_resize(cv_mat_t *mat, int32_t width, int32_t height, int32_t interpolation)
{
    clear_error();
    if (!mat || width <= 0 || height <= 0) return nullptr;
    try {
        cv::Mat out;
        cv::resize(*as_mat(mat), out, cv::Size(width, height), 0, 0,
                   map_interpolation(interpolation));
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_crop(cv_mat_t *mat, int32_t x, int32_t y, int32_t w, int32_t h)
{
    clear_error();
    if (!mat || w <= 0 || h <= 0) return nullptr;
    cv::Mat *m = as_mat(mat);
    if (x < 0 || y < 0 || x + w > m->cols || y + h > m->rows) {
        set_error("scv_crop: rectangle outside the image");
        return nullptr;
    }
    try {
        return wrap((*m)(cv::Rect(x, y, w, h)).clone());
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_rotate90(cv_mat_t *mat, int32_t code)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat out;
        cv::rotate(*as_mat(mat), out, map_rotate90(code));
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_rotate_angle(cv_mat_t *mat, double angle_deg, double scale)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat *m = as_mat(mat);
        cv::Point2f center(m->cols / 2.0f, m->rows / 2.0f);
        cv::Mat rot =
            cv::getRotationMatrix2D(center, angle_deg, scale <= 0.0 ? 1.0 : scale);
        cv::Mat out;
        cv::warpAffine(*m, out, rot, m->size());
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_flip(cv_mat_t *mat, int32_t mode)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat out;
        cv::flip(*as_mat(mat), out, map_flip(mode));
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// -------------------------------------------------------------------- filters

cv_mat_t *scv_blur(cv_mat_t *mat, int32_t ksize)
{
    clear_error();
    if (!mat || ksize <= 0) return nullptr;
    try {
        cv::Mat out;
        cv::blur(*as_mat(mat), out, cv::Size(ksize, ksize));
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_gaussian_blur(cv_mat_t *mat, int32_t ksize, double sigma)
{
    clear_error();
    if (!mat || ksize <= 0) return nullptr;
    int k = ksize % 2 == 0 ? ksize + 1 : ksize;
    try {
        cv::Mat out;
        cv::GaussianBlur(*as_mat(mat), out, cv::Size(k, k), sigma);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_median_blur(cv_mat_t *mat, int32_t ksize)
{
    clear_error();
    if (!mat || ksize <= 1) return nullptr;
    int k = ksize % 2 == 0 ? ksize + 1 : ksize;
    try {
        cv::Mat out;
        cv::medianBlur(*as_mat(mat), out, k);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_canny(cv_mat_t *mat, double t1, double t2)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        cv::Mat out;
        cv::Canny(gray, out, t1, t2);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_threshold(cv_mat_t *mat, double thresh, double maxval, int32_t kind)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        bool otsu = false;
        int flag = map_thresh(kind, &otsu);
        if (otsu) flag |= cv::THRESH_OTSU;
        cv::Mat gray = to_gray_view(*as_mat(mat));
        cv::Mat out;
        cv::threshold(gray, out, thresh, maxval, flag);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_adaptive_threshold(cv_mat_t *mat, double maxval, int32_t method,
                                 int32_t kind, int32_t block_size, double c)
{
    clear_error();
    if (!mat) return nullptr;
    int bs = block_size % 2 == 0 ? block_size + 1 : block_size;
    if (bs < 3) bs = 3;
    try {
        int cv_method =
            method == 1 ? cv::ADAPTIVE_THRESH_GAUSSIAN_C : cv::ADAPTIVE_THRESH_MEAN_C;
        int cv_kind = kind == 1 ? cv::THRESH_BINARY_INV : cv::THRESH_BINARY;
        cv::Mat gray = to_gray_view(*as_mat(mat));
        cv::Mat out;
        cv::adaptiveThreshold(gray, out, maxval, cv_method, cv_kind, bs, c);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_hist_gray(cv_mat_t *mat, int32_t *out_counts)
{
    clear_error();
    if (!mat || !out_counts) return 0;
    std::memset(out_counts, 0, sizeof(int32_t) * 256);
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        for (int y = 0; y < gray.rows; y++) {
            const uint8_t *row = gray.ptr<uint8_t>(y);
            for (int x = 0; x < gray.cols; x++)
                out_counts[row[x]]++;
        }
        return 1;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

// --------------------------------------------------------------------- drawing

int32_t scv_line(cv_mat_t *mat, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r,
                 int32_t g, int32_t b, int32_t thickness)
{
    if (!mat) return 0;
    cv::Mat *m = as_mat(mat);
    cv::line(*m, cv::Point(x1, y1), cv::Point(x2, y2), rgb_scalar(r, g, b, m->channels()),
             thickness);
    return 1;
}

int32_t scv_rectangle(cv_mat_t *mat, int32_t x, int32_t y, int32_t w, int32_t h,
                      int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    if (!mat) return 0;
    cv::Mat *m = as_mat(mat);
    cv::rectangle(*m, cv::Rect(x, y, w, h), rgb_scalar(r, g, b, m->channels()),
                  thickness);
    return 1;
}

int32_t scv_circle(cv_mat_t *mat, int32_t cx, int32_t cy, int32_t radius, int32_t r,
                   int32_t g, int32_t b, int32_t thickness)
{
    if (!mat) return 0;
    cv::Mat *m = as_mat(mat);
    cv::circle(*m, cv::Point(cx, cy), radius, rgb_scalar(r, g, b, m->channels()),
               thickness);
    return 1;
}

int32_t scv_put_text(cv_mat_t *mat, const char *text, int32_t x, int32_t y,
                     double font_scale, int32_t r, int32_t g, int32_t b,
                     int32_t thickness)
{
    if (!mat || !text) return 0;
    cv::Mat *m = as_mat(mat);
    cv::putText(*m, text, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, font_scale,
                rgb_scalar(r, g, b, m->channels()), thickness);
    return 1;
}

int32_t scv_ellipse(cv_mat_t *mat, int32_t cx, int32_t cy, int32_t axis_w, int32_t axis_h,
                    double angle, int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    if (!mat) return 0;
    cv::Mat *m = as_mat(mat);
    cv::ellipse(*m, cv::Point(cx, cy), cv::Size(axis_w, axis_h), angle, 0, 360,
                rgb_scalar(r, g, b, m->channels()), thickness);
    return 1;
}

namespace
{
std::vector<cv::Point> pts_from_xy(const int32_t *pts_xy, int32_t count)
{
    std::vector<cv::Point> pts;
    pts.reserve(count);
    for (int32_t i = 0; i < count; i++)
        pts.emplace_back(pts_xy[i * 2], pts_xy[i * 2 + 1]);
    return pts;
}
} // namespace

int32_t scv_polylines(cv_mat_t *mat, const int32_t *pts_xy, int32_t count, int32_t closed,
                      int32_t r, int32_t g, int32_t b, int32_t thickness)
{
    if (!mat || !pts_xy || count < 2) return 0;
    cv::Mat *m = as_mat(mat);
    std::vector<std::vector<cv::Point>> polys{pts_from_xy(pts_xy, count)};
    cv::polylines(*m, polys, closed != 0, rgb_scalar(r, g, b, m->channels()), thickness);
    return 1;
}

int32_t scv_fill_poly(cv_mat_t *mat, const int32_t *pts_xy, int32_t count, int32_t r,
                      int32_t g, int32_t b)
{
    if (!mat || !pts_xy || count < 3) return 0;
    cv::Mat *m = as_mat(mat);
    std::vector<std::vector<cv::Point>> polys{pts_from_xy(pts_xy, count)};
    cv::fillPoly(*m, polys, rgb_scalar(r, g, b, m->channels()));
    return 1;
}

// ------------------------------------------------------------------ morphology

cv_mat_t *scv_erode(cv_mat_t *mat, int32_t ksize, int32_t iterations)
{
    clear_error();
    if (!mat || ksize <= 0) return nullptr;
    try {
        cv::Mat kernel =
            cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));
        cv::Mat out;
        cv::erode(*as_mat(mat), out, kernel, cv::Point(-1, -1),
                  iterations > 0 ? iterations : 1);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_dilate(cv_mat_t *mat, int32_t ksize, int32_t iterations)
{
    clear_error();
    if (!mat || ksize <= 0) return nullptr;
    try {
        cv::Mat kernel =
            cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));
        cv::Mat out;
        cv::dilate(*as_mat(mat), out, kernel, cv::Point(-1, -1),
                   iterations > 0 ? iterations : 1);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_morphology(cv_mat_t *mat, int32_t op, int32_t ksize)
{
    clear_error();
    if (!mat || ksize <= 0) return nullptr;
    try {
        cv::Mat kernel =
            cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize, ksize));
        cv::Mat out;
        cv::morphologyEx(*as_mat(mat), out, map_morph_op(op), kernel);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// --------------------------------------------------------------- arithmetic

cv_mat_t *scv_add_weighted(cv_mat_t *mat1, double alpha, cv_mat_t *mat2, double beta,
                           double gamma)
{
    clear_error();
    if (!mat1 || !mat2) return nullptr;
    try {
        cv::Mat out;
        cv::addWeighted(*as_mat(mat1), alpha, *as_mat(mat2), beta, gamma, out);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_bitwise(cv_mat_t *mat1, cv_mat_t *mat2, int32_t op)
{
    clear_error();
    if (!mat1 || !mat2) return nullptr;
    try {
        cv::Mat out;
        switch (op) {
        case 1:
            cv::bitwise_or(*as_mat(mat1), *as_mat(mat2), out);
            break;
        case 2:
            cv::bitwise_xor(*as_mat(mat1), *as_mat(mat2), out);
            break;
        default:
            cv::bitwise_and(*as_mat(mat1), *as_mat(mat2), out);
            break;
        }
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_bitwise_not(cv_mat_t *mat)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat out;
        cv::bitwise_not(*as_mat(mat), out);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_abs_diff(cv_mat_t *mat1, cv_mat_t *mat2)
{
    clear_error();
    if (!mat1 || !mat2) return nullptr;
    try {
        cv::Mat out;
        cv::absdiff(*as_mat(mat1), *as_mat(mat2), out);
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// --------------------------------------------------- perspective / template

cv_mat_t *scv_warp_perspective_quad(cv_mat_t *mat, const double *src_xy,
                                    const double *dst_xy, int32_t out_width,
                                    int32_t out_height)
{
    clear_error();
    if (!mat || !src_xy || !dst_xy || out_width <= 0 || out_height <= 0) return nullptr;
    try {
        std::vector<cv::Point2f> src(4), dst(4);
        for (int i = 0; i < 4; i++) {
            src[i] = cv::Point2f(static_cast<float>(src_xy[i * 2]),
                                 static_cast<float>(src_xy[i * 2 + 1]));
            dst[i] = cv::Point2f(static_cast<float>(dst_xy[i * 2]),
                                 static_cast<float>(dst_xy[i * 2 + 1]));
        }
        cv::Mat M = cv::getPerspectiveTransform(src, dst);
        cv::Mat out;
        cv::warpPerspective(*as_mat(mat), out, M, cv::Size(out_width, out_height));
        return wrap(std::move(out));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_match_template(cv_mat_t *mat, cv_mat_t *tmpl, int32_t *out_x, int32_t *out_y,
                           double *out_score)
{
    clear_error();
    if (!mat || !tmpl || !out_x || !out_y || !out_score) return 0;
    cv::Mat *m = as_mat(mat);
    cv::Mat *t = as_mat(tmpl);
    if (t->cols > m->cols || t->rows > m->rows) {
        set_error("scv_match_template: template is larger than the search image");
        return 0;
    }
    try {
        cv::Mat result;
        cv::matchTemplate(*m, *t, result, cv::TM_CCOEFF_NORMED);
        double min_val, max_val;
        cv::Point min_loc, max_loc;
        cv::minMaxLoc(result, &min_val, &max_val, &min_loc, &max_loc);
        *out_x = max_loc.x;
        *out_y = max_loc.y;
        *out_score = max_val;
        return 1;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

// ------------------------------------------------------------------- contours

cv_contours_t *scv_find_contours(cv_mat_t *mat)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        auto *cb = new ContoursBundle();
        cv::findContours(gray, cb->contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        return cb;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_contour_count(cv_contours_t *cs)
{
    return cs ? static_cast<int32_t>(as_contours(cs)->contours.size()) : 0;
}

int32_t scv_contour_point_count(cv_contours_t *cs, int32_t idx)
{
    if (!cs) return 0;
    auto &cts = as_contours(cs)->contours;
    if (idx < 0 || idx >= static_cast<int32_t>(cts.size())) return 0;
    return static_cast<int32_t>(cts[idx].size());
}

int32_t scv_contour_point_get(cv_contours_t *cs, int32_t idx, int32_t pt_idx,
                              int32_t *out_xy)
{
    if (!cs || !out_xy) return 0;
    auto &cts = as_contours(cs)->contours;
    if (idx < 0 || idx >= static_cast<int32_t>(cts.size())) return 0;
    if (pt_idx < 0 || pt_idx >= static_cast<int32_t>(cts[idx].size())) return 0;
    out_xy[0] = cts[idx][pt_idx].x;
    out_xy[1] = cts[idx][pt_idx].y;
    return 1;
}

double scv_contour_area(cv_contours_t *cs, int32_t idx)
{
    if (!cs) return 0.0;
    auto &cts = as_contours(cs)->contours;
    if (idx < 0 || idx >= static_cast<int32_t>(cts.size())) return 0.0;
    return cv::contourArea(cts[idx]);
}

int32_t scv_contour_bounding_rect(cv_contours_t *cs, int32_t idx, int32_t *out_xywh)
{
    if (!cs || !out_xywh) return 0;
    auto &cts = as_contours(cs)->contours;
    if (idx < 0 || idx >= static_cast<int32_t>(cts.size())) return 0;
    cv::Rect r = cv::boundingRect(cts[idx]);
    out_xywh[0] = r.x;
    out_xywh[1] = r.y;
    out_xywh[2] = r.width;
    out_xywh[3] = r.height;
    return 1;
}

int32_t scv_draw_contour(cv_mat_t *mat, cv_contours_t *cs, int32_t idx, int32_t r,
                         int32_t g, int32_t b, int32_t thickness)
{
    if (!mat || !cs) return 0;
    cv::Mat *m = as_mat(mat);
    cv::drawContours(*m, as_contours(cs)->contours, idx,
                     rgb_scalar(r, g, b, m->channels()), thickness);
    return 1;
}

void scv_free_contours(cv_contours_t *cs)
{
    delete as_contours(cs);
}

// -------------------------------------------------------------------- features

int32_t scv_good_features(cv_mat_t *mat, int32_t max_corners, double quality,
                          double min_dist, int32_t *out_x, int32_t *out_y)
{
    clear_error();
    if (!mat || max_corners <= 0 || !out_x || !out_y) return 0;
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        std::vector<cv::Point2f> corners;
        cv::goodFeaturesToTrack(gray, corners, max_corners, quality, min_dist);
        int32_t n = static_cast<int32_t>(std::min<size_t>(corners.size(), max_corners));
        for (int32_t i = 0; i < n; i++) {
            out_x[i] = static_cast<int32_t>(corners[i].x);
            out_y[i] = static_cast<int32_t>(corners[i].y);
        }
        return n;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

cv_keypoints_t *scv_orb_detect(cv_mat_t *mat, int32_t max_features)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        KpBundle *b = new KpBundle();
        cv::Ptr<cv::ORB> orb = cv::ORB::create(max_features > 0 ? max_features : 500);
        orb->detectAndCompute(gray, cv::noArray(), b->kps, b->descriptors);
        return b;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// SIFT descriptors are float32 (not ORB's binary ones) - match them with
// NORM_L2, not the default NORM_HAMMING (see MatchDescriptors' doc
// comment in features.salam).
cv_keypoints_t *scv_sift_detect(cv_mat_t *mat, int32_t max_features)
{
    clear_error();
    if (!mat) return nullptr;
    try {
        cv::Mat gray = to_gray_view(*as_mat(mat));
        KpBundle *b = new KpBundle();
        cv::Ptr<cv::SIFT> sift = cv::SIFT::create(max_features > 0 ? max_features : 0);
        sift->detectAndCompute(gray, cv::noArray(), b->kps, b->descriptors);
        return b;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_keypoint_count(cv_keypoints_t *kp)
{
    return kp ? static_cast<int32_t>(as_kp(kp)->kps.size()) : 0;
}

int32_t scv_keypoint_get(cv_keypoints_t *kp, int32_t idx, double *out_xy)
{
    if (!kp || !out_xy) return 0;
    KpBundle *b = as_kp(kp);
    if (idx < 0 || idx >= static_cast<int32_t>(b->kps.size())) return 0;
    const cv::KeyPoint &k = b->kps[idx];
    out_xy[0] = k.pt.x;
    out_xy[1] = k.pt.y;
    out_xy[2] = k.size;
    out_xy[3] = k.angle;
    out_xy[4] = k.response;
    return 1;
}

cv_mat_t *scv_orb_descriptors(cv_keypoints_t *kp)
{
    if (!kp) return nullptr;
    // Cloned so the returned Mat is an independently owned handle - the
    // caller frees it with its own scv_free, separately from scv_free_keypoints.
    return wrap(as_kp(kp)->descriptors.clone());
}

void scv_free_keypoints(cv_keypoints_t *kp)
{
    delete as_kp(kp);
}

cv_matches_t *scv_bf_match(cv_mat_t *desc1, cv_mat_t *desc2, int32_t max_matches,
                           int32_t norm)
{
    clear_error();
    if (!desc1 || !desc2) return nullptr;
    try {
        auto *matches = new std::vector<cv::DMatch>();
        cv::BFMatcher matcher(map_norm(norm));
        matcher.match(*as_mat(desc1), *as_mat(desc2), *matches);
        std::sort(matches->begin(), matches->end(),
                  [](const cv::DMatch &a, const cv::DMatch &b) {
                      return a.distance < b.distance;
                  });
        if (max_matches > 0 && static_cast<int32_t>(matches->size()) > max_matches)
            matches->resize(max_matches);
        return matches;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_match_count(cv_matches_t *m)
{
    return m ? static_cast<int32_t>(as_matches(m)->size()) : 0;
}

int32_t scv_match_get(cv_matches_t *m, int32_t idx, int32_t *out_query,
                      int32_t *out_train, double *out_dist)
{
    if (!m || !out_query || !out_train || !out_dist) return 0;
    auto *matches = as_matches(m);
    if (idx < 0 || idx >= static_cast<int32_t>(matches->size())) return 0;
    const cv::DMatch &d = (*matches)[idx];
    *out_query = d.queryIdx;
    *out_train = d.trainIdx;
    *out_dist = static_cast<double>(d.distance);
    return 1;
}

void scv_free_matches(cv_matches_t *m)
{
    delete as_matches(m);
}

// -------------------------------------------------------------- object detection

cv_cascade_t *scv_cascade_load(const char *path)
{
    clear_error();
    if (!path) return nullptr;
    try {
        auto *c = new cv::CascadeClassifier();
        if (!c->load(path)) {
            delete c;
            set_error(std::string("failed to load cascade: ") + path);
            return nullptr;
        }
        return c;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

void scv_cascade_free(cv_cascade_t *c)
{
    delete static_cast<cv::CascadeClassifier *>(c);
}

int32_t scv_cascade_detect(cv_cascade_t *c, cv_mat_t *mat, double scale_factor,
                           int32_t min_neighbors, int32_t max_results, int32_t *out_x,
                           int32_t *out_y, int32_t *out_w, int32_t *out_h)
{
    clear_error();
    if (!c || !mat || max_results <= 0) return 0;
    try {
        std::vector<cv::Rect> results;
        static_cast<cv::CascadeClassifier *>(c)->detectMultiScale(
            *as_mat(mat), results, scale_factor > 1.0 ? scale_factor : 1.1,
            min_neighbors);
        int32_t n = static_cast<int32_t>(std::min<size_t>(results.size(), max_results));
        for (int32_t i = 0; i < n; i++) {
            out_x[i] = results[i].x;
            out_y[i] = results[i].y;
            out_w[i] = results[i].width;
            out_h[i] = results[i].height;
        }
        return n;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

// ------------------------------------------------------------------------ dnn

cv_net_t *scv_dnn_load(const char *model_path, const char *config_path)
{
    clear_error();
    if (!model_path) return nullptr;
    try {
        std::string cfg =
            (config_path && std::strlen(config_path) > 0) ? config_path : std::string();
        auto *net = new cv::dnn::Net();
        *net = cv::dnn::readNet(model_path, cfg);
        if (net->empty()) {
            delete net;
            set_error(std::string("failed to load network: ") + model_path);
            return nullptr;
        }
        return net;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

void scv_dnn_free(cv_net_t *net)
{
    delete as_net(net);
}

int32_t scv_dnn_set_input(cv_net_t *net, cv_mat_t *mat, double scale, double mean_r,
                          double mean_g, double mean_b, int32_t swap_rb, int32_t width,
                          int32_t height)
{
    clear_error();
    if (!net || !mat || width <= 0 || height <= 0) return 0;
    try {
        // The source Mat is stored BGR internally (see the channel-order
        // contract in opencv_shim.hpp); the mean Scalar must match that
        // same pre-swap order, so it's built as (B, G, R) here regardless
        // of swap_rb - swap_rb only controls whether blobFromImage itself
        // additionally swaps the image's R/B before the network sees it.
        cv::Mat blob = cv::dnn::blobFromImage(
            *as_mat(mat), scale, cv::Size(width, height),
            cv::Scalar(mean_b, mean_g, mean_r), swap_rb != 0, false, CV_32F);
        as_net(net)->setInput(blob);
        return 1;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

cv_mat_t *scv_dnn_forward(cv_net_t *net)
{
    clear_error();
    if (!net) return nullptr;
    try {
        cv::Mat out = as_net(net)->forward();
        return wrap(out.clone());
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_dnn_forward_named(cv_net_t *net, const char *layer_name)
{
    clear_error();
    if (!net || !layer_name) return nullptr;
    try {
        cv::Mat out = as_net(net)->forward(layer_name);
        return wrap(out.clone());
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

// ---------------------------------------------------------------------- video

cv_video_t *scv_video_open_file(const char *path)
{
    clear_error();
    if (!path) return nullptr;
    try {
        return new cv::VideoCapture(path);
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_video_t *scv_video_open_camera(int32_t index)
{
    clear_error();
    try {
        return new cv::VideoCapture(index);
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

cv_mat_t *scv_video_read(cv_video_t *v)
{
    if (!v) return nullptr;
    try {
        cv::Mat frame;
        bool got = static_cast<cv::VideoCapture *>(v)->read(frame);
        if (!got || frame.empty()) return nullptr;
        return wrap(std::move(frame));
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

void scv_video_release(cv_video_t *v)
{
    if (!v) return;
    auto *cap = static_cast<cv::VideoCapture *>(v);
    cap->release();
    delete cap;
}

int32_t scv_video_ok(cv_video_t *v)
{
    return (v && static_cast<cv::VideoCapture *>(v)->isOpened()) ? 1 : 0;
}

double scv_video_fps(cv_video_t *v)
{
    return v ? static_cast<cv::VideoCapture *>(v)->get(cv::CAP_PROP_FPS) : 0.0;
}

int32_t scv_video_frame_count(cv_video_t *v)
{
    return v ? static_cast<int32_t>(
                   static_cast<cv::VideoCapture *>(v)->get(cv::CAP_PROP_FRAME_COUNT))
             : 0;
}

cv_writer_t *scv_writer_open(const char *path, const char *fourcc, double fps,
                             int32_t width, int32_t height)
{
    clear_error();
    if (!path || !fourcc || std::strlen(fourcc) < 4 || width <= 0 || height <= 0) {
        set_error("scv_writer_open: invalid arguments");
        return nullptr;
    }
    try {
        int code = cv::VideoWriter::fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
        auto *w = new cv::VideoWriter(path, code, fps, cv::Size(width, height));
        if (!w->isOpened()) {
            delete w;
            set_error(std::string("failed to open video writer: ") + path);
            return nullptr;
        }
        return w;
    } catch (const std::exception &e) {
        set_error(e.what());
        return nullptr;
    }
}

int32_t scv_writer_write(cv_writer_t *w, cv_mat_t *frame)
{
    if (!w || !frame) return 0;
    try {
        static_cast<cv::VideoWriter *>(w)->write(*as_mat(frame));
        return 1;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

void scv_writer_release(cv_writer_t *w)
{
    if (!w) return;
    auto *writer = static_cast<cv::VideoWriter *>(w);
    writer->release();
    delete writer;
}

// ------------------------------------------------------------------------ gui

void scv_imshow(const char *window, cv_mat_t *mat)
{
    if (!window || !mat) return;
    try {
        cv::imshow(window, *as_mat(mat));
    } catch (const std::exception &e) {
        set_error(e.what());
    }
}

int32_t scv_wait_key(int32_t delay_ms)
{
    try {
        return cv::waitKey(delay_ms);
    } catch (const std::exception &e) {
        set_error(e.what());
        return -1;
    }
}

void scv_destroy_window(const char *window)
{
    if (!window) return;
    try {
        cv::destroyWindow(window);
    } catch (const std::exception &) {
    }
}

void scv_destroy_all_windows()
{
    try {
        cv::destroyAllWindows();
    } catch (const std::exception &) {
    }
}

namespace
{
// cv::createTrackbar needs a stable int* to own for the trackbar's
// lifetime (it does not take ownership itself) - kept alive here for the
// process lifetime, keyed by "window::name". highgui itself is not
// thread-safe, so a plain (non-atomic) map is consistent with how every
// other GUI call in this file is already used (single UI thread).
std::map<std::string, int> &trackbar_store()
{
    static std::map<std::string, int> store;
    return store;
}
} // namespace

int32_t scv_create_trackbar(const char *name, const char *window, int32_t initial,
                            int32_t max)
{
    if (!name || !window) return 0;
    try {
        std::string key = std::string(window) + "::" + name;
        int &value = trackbar_store()[key];
        value = initial;
        cv::createTrackbar(name, window, &value, max);
        return 1;
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

int32_t scv_get_trackbar_pos(const char *name, const char *window)
{
    if (!name || !window) return 0;
    try {
        return cv::getTrackbarPos(name, window);
    } catch (const std::exception &e) {
        set_error(e.what());
        return 0;
    }
}

void scv_set_trackbar_pos(const char *name, const char *window, int32_t pos)
{
    if (!name || !window) return;
    try {
        cv::setTrackbarPos(name, window, pos);
    } catch (const std::exception &e) {
        set_error(e.what());
    }
}

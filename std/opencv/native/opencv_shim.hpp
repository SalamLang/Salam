// Salam Programming Language (2024-2026) - std/opencv native shim
//
// A flat, versioned extern "C" ABI bridging Salam's FFI (which can only
// bind plain C symbols - see SKILL.md section 8) to OpenCV's C++ API
// (which has had no stable C ABI since OpenCV 4). This header is the single
// source of truth for that ABI; std/opencv/opencv.salam's `extern:` block
// must match it function-for-function. See BUILD.md for how to compile
// this into a library and link it from Salam.
//
// ABI conventions (mirrored on the Salam side, opencv.salam's header
// comment):
//   - Every `cv_mat_t` is a heap-allocated cv::Mat*, opaque to Salam.
//     Ownership: whichever function returned it owns it until the caller
//     passes it to scv_free (or another *_free/*_release). Never freed
//     twice, never used after freeing - same discipline as malloc/free.
//   - Every 3-channel Mat is stored BGR internally (OpenCV's own default);
//     any function that takes/returns pixel *colors* (not raw buffers)
//     converts at this boundary, so Salam-side colors are always RGB.
//   - No exceptions cross the ABI: every OpenCV exception is caught inside
//     the shim, recorded via set_last_error(), and converted to a null
//     handle / 0 / -1 return, per function.
//   - Integers are exactly 32-bit (`int32_t`), matching Salam's `int`/`i32`
//     alias, regardless of host `int` width.
#pragma once

#include <cstdint>

#if defined(_WIN32)
#  define SCV_API extern "C" __declspec(dllexport)
#else
#  define SCV_API extern "C" __attribute__((visibility("default")))
#endif

typedef void cv_mat_t;
typedef void cv_keypoints_t;
typedef void cv_matches_t;
typedef void cv_cascade_t;
typedef void cv_video_t;
typedef void cv_writer_t;
typedef void cv_contours_t;
typedef void cv_net_t;

extern "C" {

// -- diagnostics ---------------------------------------------------------
SCV_API const char *scv_last_error();
SCV_API const char *scv_version();

// -- lifecycle / io --------------------------------------------------------
SCV_API cv_mat_t *scv_read(const char *path);
SCV_API int32_t scv_write(const char *path, cv_mat_t *mat);
SCV_API void scv_free(cv_mat_t *mat);
SCV_API cv_mat_t *scv_clone(cv_mat_t *mat);
SCV_API cv_mat_t *scv_new_blank(int32_t width, int32_t height, int32_t channels,
                                int32_t r, int32_t g, int32_t b);
SCV_API cv_mat_t *scv_from_buffer(const uint8_t *data, int32_t width, int32_t height,
                                  int32_t channels);

// -- introspection ---------------------------------------------------------
SCV_API int32_t scv_width(cv_mat_t *mat);
SCV_API int32_t scv_height(cv_mat_t *mat);
SCV_API int32_t scv_channels(cv_mat_t *mat);
SCV_API int32_t scv_empty(cv_mat_t *mat);
SCV_API uint8_t *scv_data(cv_mat_t *mat);
SCV_API int32_t scv_step(cv_mat_t *mat);
SCV_API int32_t scv_mat_total(cv_mat_t *mat);
SCV_API int32_t scv_get_pixel(cv_mat_t *mat, int32_t x, int32_t y, int32_t *out_rgb);
SCV_API int32_t scv_set_pixel(cv_mat_t *mat, int32_t x, int32_t y, int32_t r, int32_t g,
                              int32_t b);

// -- color space -----------------------------------------------------------
SCV_API cv_mat_t *scv_cvt_color(cv_mat_t *mat, int32_t code);

// -- geometry ----------------------------------------------------------
SCV_API cv_mat_t *scv_resize(cv_mat_t *mat, int32_t width, int32_t height,
                             int32_t interpolation);
SCV_API cv_mat_t *scv_crop(cv_mat_t *mat, int32_t x, int32_t y, int32_t w, int32_t h);
SCV_API cv_mat_t *scv_rotate90(cv_mat_t *mat, int32_t code);
SCV_API cv_mat_t *scv_rotate_angle(cv_mat_t *mat, double angle_deg, double scale);
SCV_API cv_mat_t *scv_flip(cv_mat_t *mat, int32_t mode);

// -- filters -----------------------------------------------------------
SCV_API cv_mat_t *scv_blur(cv_mat_t *mat, int32_t ksize);
SCV_API cv_mat_t *scv_gaussian_blur(cv_mat_t *mat, int32_t ksize, double sigma);
SCV_API cv_mat_t *scv_median_blur(cv_mat_t *mat, int32_t ksize);
SCV_API cv_mat_t *scv_canny(cv_mat_t *mat, double t1, double t2);
SCV_API cv_mat_t *scv_threshold(cv_mat_t *mat, double thresh, double maxval,
                                int32_t kind);
SCV_API cv_mat_t *scv_adaptive_threshold(cv_mat_t *mat, double maxval, int32_t method,
                                         int32_t kind, int32_t block_size, double c);
SCV_API int32_t scv_hist_gray(cv_mat_t *mat, int32_t *out_counts /* [256] */);

// -- drawing (mutate the Mat in place) ------------------------------------
SCV_API int32_t scv_line(cv_mat_t *mat, int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                         int32_t r, int32_t g, int32_t b, int32_t thickness);
SCV_API int32_t scv_rectangle(cv_mat_t *mat, int32_t x, int32_t y, int32_t w, int32_t h,
                              int32_t r, int32_t g, int32_t b, int32_t thickness);
SCV_API int32_t scv_circle(cv_mat_t *mat, int32_t cx, int32_t cy, int32_t radius,
                           int32_t r, int32_t g, int32_t b, int32_t thickness);
SCV_API int32_t scv_put_text(cv_mat_t *mat, const char *text, int32_t x, int32_t y,
                             double font_scale, int32_t r, int32_t g, int32_t b,
                             int32_t thickness);
SCV_API int32_t scv_ellipse(cv_mat_t *mat, int32_t cx, int32_t cy, int32_t axis_w,
                            int32_t axis_h, double angle, int32_t r, int32_t g, int32_t b,
                            int32_t thickness);
SCV_API int32_t scv_polylines(cv_mat_t *mat, const int32_t *pts_xy, int32_t count,
                              int32_t closed, int32_t r, int32_t g, int32_t b,
                              int32_t thickness);
SCV_API int32_t scv_fill_poly(cv_mat_t *mat, const int32_t *pts_xy, int32_t count,
                              int32_t r, int32_t g, int32_t b);

// -- morphology ------------------------------------------------------------
SCV_API cv_mat_t *scv_erode(cv_mat_t *mat, int32_t ksize, int32_t iterations);
SCV_API cv_mat_t *scv_dilate(cv_mat_t *mat, int32_t ksize, int32_t iterations);
SCV_API cv_mat_t *scv_morphology(cv_mat_t *mat, int32_t op, int32_t ksize);

// -- arithmetic / blending ---------------------------------------------------
SCV_API cv_mat_t *scv_add_weighted(cv_mat_t *mat1, double alpha, cv_mat_t *mat2,
                                   double beta, double gamma);
SCV_API cv_mat_t *scv_bitwise(cv_mat_t *mat1, cv_mat_t *mat2, int32_t op);
SCV_API cv_mat_t *scv_bitwise_not(cv_mat_t *mat);
SCV_API cv_mat_t *scv_abs_diff(cv_mat_t *mat1, cv_mat_t *mat2);

// -- perspective / template matching ------------------------------------------
SCV_API cv_mat_t *scv_warp_perspective_quad(cv_mat_t *mat, const double *src_xy /* [8] */,
                                            const double *dst_xy /* [8] */,
                                            int32_t out_width, int32_t out_height);
SCV_API int32_t scv_match_template(cv_mat_t *mat, cv_mat_t *tmpl, int32_t *out_x,
                                   int32_t *out_y, double *out_score);

// -- contours ----------------------------------------------------------------
SCV_API cv_contours_t *scv_find_contours(cv_mat_t *mat);
SCV_API int32_t scv_contour_count(cv_contours_t *cs);
SCV_API int32_t scv_contour_point_count(cv_contours_t *cs, int32_t idx);
SCV_API int32_t scv_contour_point_get(cv_contours_t *cs, int32_t idx, int32_t pt_idx,
                                      int32_t *out_xy);
SCV_API double scv_contour_area(cv_contours_t *cs, int32_t idx);
SCV_API int32_t scv_contour_bounding_rect(cv_contours_t *cs, int32_t idx,
                                          int32_t *out_xywh);
SCV_API int32_t scv_draw_contour(cv_mat_t *mat, cv_contours_t *cs, int32_t idx, int32_t r,
                                 int32_t g, int32_t b, int32_t thickness);
SCV_API void scv_free_contours(cv_contours_t *cs);

// -- features ------------------------------------------------------------
SCV_API int32_t scv_good_features(cv_mat_t *mat, int32_t max_corners, double quality,
                                  double min_dist, int32_t *out_x, int32_t *out_y);
SCV_API cv_keypoints_t *scv_orb_detect(cv_mat_t *mat, int32_t max_features);
SCV_API cv_keypoints_t *scv_sift_detect(cv_mat_t *mat, int32_t max_features);
SCV_API int32_t scv_keypoint_count(cv_keypoints_t *kp);
SCV_API int32_t scv_keypoint_get(cv_keypoints_t *kp, int32_t idx,
                                 double *out_xy /* [5] */);
SCV_API cv_mat_t *scv_orb_descriptors(cv_keypoints_t *kp);
SCV_API void scv_free_keypoints(cv_keypoints_t *kp);
SCV_API cv_matches_t *scv_bf_match(cv_mat_t *desc1, cv_mat_t *desc2, int32_t max_matches,
                                   int32_t norm);
SCV_API int32_t scv_match_count(cv_matches_t *m);
SCV_API int32_t scv_match_get(cv_matches_t *m, int32_t idx, int32_t *out_query,
                              int32_t *out_train, double *out_dist);
SCV_API void scv_free_matches(cv_matches_t *m);

// -- dnn -------------------------------------------------------------------
SCV_API cv_net_t *scv_dnn_load(const char *model_path, const char *config_path);
SCV_API void scv_dnn_free(cv_net_t *net);
SCV_API int32_t scv_dnn_set_input(cv_net_t *net, cv_mat_t *mat, double scale,
                                  double mean_r, double mean_g, double mean_b,
                                  int32_t swap_rb, int32_t width, int32_t height);
SCV_API cv_mat_t *scv_dnn_forward(cv_net_t *net);
SCV_API cv_mat_t *scv_dnn_forward_named(cv_net_t *net, const char *layer_name);

// -- object detection ----------------------------------------------------
SCV_API cv_cascade_t *scv_cascade_load(const char *path);
SCV_API void scv_cascade_free(cv_cascade_t *c);
SCV_API int32_t scv_cascade_detect(cv_cascade_t *c, cv_mat_t *mat, double scale_factor,
                                   int32_t min_neighbors, int32_t max_results,
                                   int32_t *out_x, int32_t *out_y, int32_t *out_w,
                                   int32_t *out_h);

// -- video -----------------------------------------------------------------
SCV_API cv_video_t *scv_video_open_file(const char *path);
SCV_API cv_video_t *scv_video_open_camera(int32_t index);
SCV_API cv_mat_t *scv_video_read(cv_video_t *v);
SCV_API void scv_video_release(cv_video_t *v);
SCV_API int32_t scv_video_ok(cv_video_t *v);
SCV_API double scv_video_fps(cv_video_t *v);
SCV_API int32_t scv_video_frame_count(cv_video_t *v);
SCV_API cv_writer_t *scv_writer_open(const char *path, const char *fourcc, double fps,
                                     int32_t width, int32_t height);
SCV_API int32_t scv_writer_write(cv_writer_t *w, cv_mat_t *frame);
SCV_API void scv_writer_release(cv_writer_t *w);

// -- gui (highgui, optional module) -----------------------------------------
SCV_API void scv_imshow(const char *window, cv_mat_t *mat);
SCV_API int32_t scv_wait_key(int32_t delay_ms);
SCV_API void scv_destroy_window(const char *window);
SCV_API void scv_destroy_all_windows();
SCV_API int32_t scv_create_trackbar(const char *name, const char *window, int32_t initial,
                                    int32_t max);
SCV_API int32_t scv_get_trackbar_pos(const char *name, const char *window);
SCV_API void scv_set_trackbar_pos(const char *name, const char *window, int32_t pos);

} // extern "C"

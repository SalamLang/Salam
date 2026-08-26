/*
 * Salam Programming Language (2024-2026)
 *
 * Dependency-free stand-in for cef_shim.cc, linked instead of the real shim
 * when a program is built with -DSALAM_WEBVIEW_CEF -DSALAM_WEBVIEW_CEF_MOCK.
 *
 * It exists so the Salam half of the CEF backend - cef.salam - can be
 * compiled, linked and run in CI without the ~200 MB CEF binary
 * distribution. It opens no window and renders nothing; it only tracks state
 * so that the calls Salam makes can be observed and asserted on.
 * std/opencv/native/mock/opencv_mock.c serves the same purpose for OpenCV.
 *
 * Do not ship this. A program linked against it has no browser.
 */

#include "../cef_shim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOCK_TEXT_MAX 4096

struct mock_view {
  int width;
  int height;
  int fixed;
  int closed;
  int pumps_left;
  char title[256];
  char url[MOCK_TEXT_MAX];
};

static int g_initialized = 0;
static struct mock_view *g_current = NULL;

/* Number of salam_wv_cef_pump() calls before the mock window reports itself
   closed, so a test's Run() loop terminates. Overridable for tests that want
   a longer or shorter loop. */
static int mock_pump_budget(void) {
  const char *s = getenv("SALAM_CEF_MOCK_PUMPS");
  if (s && *s) {
    int n = atoi(s);
    if (n >= 0) {
      return n;
    }
  }
  return 3;
}

int salam_wv_cef_init(int debug) {
  (void)debug;
  g_initialized = 1;
  return 1;
}

void *salam_wv_cef_create(int width, int height, int debug) {
  struct mock_view *v;
  (void)debug;
  if (!g_initialized) {
    return NULL;
  }
  v = (struct mock_view *)calloc(1, sizeof(*v));
  if (!v) {
    return NULL;
  }
  v->width = width > 0 ? width : 800;
  v->height = height > 0 ? height : 600;
  v->pumps_left = mock_pump_budget();
  snprintf(v->url, sizeof(v->url), "about:blank");
  g_current = v;
  return v;
}

void salam_wv_cef_destroy(void *view) {
  struct mock_view *v = (struct mock_view *)view;
  if (!v) {
    return;
  }
  if (g_current == v) {
    g_current = NULL;
  }
  free(v);
}

void salam_wv_cef_pump(void) {
  if (g_current && g_current->pumps_left > 0) {
    g_current->pumps_left--;
    if (g_current->pumps_left == 0) {
      g_current->closed = 1;
    }
  }
}

int salam_wv_cef_is_closed(void *view) {
  struct mock_view *v = (struct mock_view *)view;
  return (!v || v->closed) ? 1 : 0;
}

void salam_wv_cef_set_title(void *view, const char *title) {
  struct mock_view *v = (struct mock_view *)view;
  if (!v || !title) {
    return;
  }
  snprintf(v->title, sizeof(v->title), "%s", title);
  printf("cef-mock: title=%s\n", v->title);
  fflush(stdout);
}

void salam_wv_cef_set_size(void *view, int width, int height, int fixed) {
  struct mock_view *v = (struct mock_view *)view;
  if (!v || width <= 0 || height <= 0) {
    return;
  }
  v->width = width;
  v->height = height;
  v->fixed = fixed;
  printf("cef-mock: size=%dx%d fixed=%d\n", width, height, fixed);
  fflush(stdout);
}

void salam_wv_cef_set_html(void *view, const char *html) {
  struct mock_view *v = (struct mock_view *)view;
  if (!v || !html) {
    return;
  }
  snprintf(v->url, sizeof(v->url), "data:text/html;len=%d",
           (int)strlen(html));
  printf("cef-mock: html=%d bytes\n", (int)strlen(html));
  fflush(stdout);
}

void salam_wv_cef_navigate(void *view, const char *url) {
  struct mock_view *v = (struct mock_view *)view;
  if (!v || !url) {
    return;
  }
  snprintf(v->url, sizeof(v->url), "%s", url);
  printf("cef-mock: navigate=%s\n", v->url);
  fflush(stdout);
}

void salam_wv_cef_eval(void *view, const char *js) {
  if (!view || !js) {
    return;
  }
  printf("cef-mock: eval=%s\n", js);
  fflush(stdout);
}

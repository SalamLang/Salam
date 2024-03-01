/*
 * Salam Programming Language (2024-2026)
 *
 * Flat C ABI between std/webview's CEF backend (cef.salam) and CEF itself.
 *
 * Salam generates C and links with a C compiler, while CEF's real API is C++.
 * Everything below is therefore plain C with no CEF types in the signatures -
 * cef_shim.cc implements it over CefRefPtr/CefString/CefClient, and
 * mock/cef_mock.c implements the same names with no CEF at all so the Salam
 * side can be tested without the SDK.
 *
 * Handles are opaque. Strings are UTF-8 and are copied by the callee; the
 * caller keeps ownership. Every call except salam_wv_cef_init and
 * salam_wv_cef_create tolerates a NULL view and does nothing.
 */
#ifndef SALAM_WEBVIEW_CEF_SHIM_H
#define SALAM_WEBVIEW_CEF_SHIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes CEF once per process. Returns 1 on success, 0 on failure.
   Safe to call repeatedly; later calls return the first result. */
int salam_wv_cef_init(int debug);

/* Creates a browser window. Returns an opaque view handle, or NULL. */
void *salam_wv_cef_create(int width, int height, int debug);

/* Closes the browser and releases the handle. Shuts CEF down once the last
   live view is destroyed. */
void salam_wv_cef_destroy(void *view);

/* One iteration of CEF's message loop (CefDoMessageLoopWork). */
void salam_wv_cef_pump(void);

/* 1 once the user closed the window, else 0. */
int salam_wv_cef_is_closed(void *view);

void salam_wv_cef_set_title(void *view, const char *title);
void salam_wv_cef_set_size(void *view, int width, int height, int fixed);

/* CEF has no LoadString; the shim encodes html as a data: URL. */
void salam_wv_cef_set_html(void *view, const char *html);

void salam_wv_cef_navigate(void *view, const char *url);
void salam_wv_cef_eval(void *view, const char *js);

#ifdef __cplusplus
}
#endif

#endif /* SALAM_WEBVIEW_CEF_SHIM_H */

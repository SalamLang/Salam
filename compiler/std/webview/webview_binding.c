/*
 * WebView Salam Binding
 * Provides FFI bridge between Salam and the webview library
 * Supports Windows (WebView2), Linux (WebKitGTK), macOS (WKWebView)
 */

#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#  define SALAM_OS_WINDOWS 1
#  include <windows.h>
#  include <wrl.h>
#  include <wil/com.h> /* codespell: ignore=wil */
#  include "WebView2.h"

typedef struct {
    ICoreWebView2Controller *controller;
    ICoreWebView2 *webview;
    HWND hwnd;
    int closed;
} SalamWebView;

#elif defined(__linux__)
#  define SALAM_OS_LINUX 1
#  include <gtk/gtk.h>
#  include <webkit2/webkit2.h>

typedef struct {
    GtkWidget *window;
    WebKitWebView *webview;
    int closed;
} SalamWebView;

#elif defined(__APPLE__)
#  define SALAM_OS_DARWIN 1
#  include <Cocoa/Cocoa.h>
#  include <WebKit/WebKit.h>

typedef struct {
    NSWindow *window;
    WKWebView *webview;
    int closed;
} SalamWebView;

#else
#  error "Unsupported platform"
#endif

void *webview_create(int debug, void *window)
{
    SalamWebView *wv = (SalamWebView *)malloc(sizeof(SalamWebView));
    if (wv == NULL) return NULL;

    wv->closed = 0;

#if defined(SALAM_OS_WINDOWS)
    wv->hwnd = NULL;
    wv->controller = NULL;
    wv->webview = NULL;
    /* WebView2 initialization deferred to when window is available */

#elif defined(SALAM_OS_LINUX)
    if (!gtk_init_check(0, NULL)) {
        free(wv);
        return NULL;
    }

    wv->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (wv->window == NULL) {
        free(wv);
        return NULL;
    }

    gtk_window_set_default_size(GTK_WINDOW(wv->window), 800, 600);

    wv->webview = WEBKIT_WEB_VIEW(webkit_web_view_new());
    if (wv->webview == NULL) {
        g_object_unref(wv->window);
        free(wv);
        return NULL;
    }

    gtk_container_add(GTK_CONTAINER(wv->window), GTK_WIDGET(wv->webview));
    gtk_widget_show_all(wv->window);

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSRect frame = NSMakeRect(100, 100, 800, 600);
        wv->window = [[NSWindow alloc]
            initWithContentRect:frame
                      styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                                NSWindowStyleMaskMiniaturizable |
                                NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                          defer:NO];

        WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
        wv->webview = [[WKWebView alloc] initWithFrame:frame configuration:config];

        [[wv->window contentView] addSubview:(NSView *)wv->webview];
        [wv->window makeKeyAndOrderFront:nil];
        [config release];
    }
#endif

    return (void *)wv;
}

void webview_destroy(void *w)
{
    if (w == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;
    wv->closed = 1;

#if defined(SALAM_OS_WINDOWS)
    if (wv->webview) wv->webview->Release();
    if (wv->controller) wv->controller->Release();
    if (wv->hwnd) DestroyWindow(wv->hwnd);

#elif defined(SALAM_OS_LINUX)
    if (wv->webview) g_object_unref(wv->webview);
    if (wv->window) g_object_unref(wv->window);

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        [(NSWindow *)wv->window close];
        [(WKWebView *)wv->webview release];
        [(NSWindow *)wv->window release];
    }
#endif

    free(wv);
}

void webview_run(void *w)
{
    if (w == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0 && !wv->closed) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#elif defined(SALAM_OS_LINUX)
    g_signal_connect(wv->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        [NSApp run];
    }
#endif
}

void webview_terminate(void *w)
{
    if (w == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;
    wv->closed = 1;

#if defined(SALAM_OS_WINDOWS)
    if (wv->hwnd) PostMessage(wv->hwnd, WM_QUIT, 0, 0);

#elif defined(SALAM_OS_LINUX)
    gtk_main_quit();

#elif defined(SALAM_OS_DARWIN)
    [NSApp terminate:nil];
#endif
}

void webview_set_title(void *w, const char *title)
{
    if (w == NULL || title == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    if (wv->hwnd) SetWindowTextA(wv->hwnd, title);

#elif defined(SALAM_OS_LINUX)
    if (wv->window) gtk_window_set_title(GTK_WINDOW(wv->window), title);

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSString *ns_title = [NSString stringWithUTF8String:title];
        [(NSWindow *)wv->window setTitle:ns_title];
    }
#endif
}

void webview_set_size(void *w, int width, int height, int hints)
{
    if (w == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    if (wv->hwnd) {
        SetWindowPos(wv->hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE);
    }

#elif defined(SALAM_OS_LINUX)
    if (wv->window) {
        gtk_window_set_default_size(GTK_WINDOW(wv->window), width, height);
    }

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSRect frame = [(NSWindow *)wv->window frame];
        frame.size.width = width;
        frame.size.height = height;
        [(NSWindow *)wv->window setFrame:frame display:YES];
    }
#endif
}

void webview_set_html(void *w, const char *html)
{
    if (w == NULL || html == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    if (wv->webview) {
        wchar_t *whtml = (wchar_t *)malloc((strlen(html) + 1) * sizeof(wchar_t));
        if (whtml) {
            MultiByteToWideChar(CP_UTF8, 0, html, -1, whtml, strlen(html) + 1);
            wv->webview->NavigateToString(whtml);
            free(whtml);
        }
    }

#elif defined(SALAM_OS_LINUX)
    if (wv->webview) {
        webkit_web_view_load_html(wv->webview, html, NULL);
    }

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSString *ns_html = [NSString stringWithUTF8String:html];
        [(WKWebView *)wv->webview loadHTMLString:ns_html baseURL:nil];
    }
#endif
}

void webview_navigate(void *w, const char *url)
{
    if (w == NULL || url == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    if (wv->webview) {
        wchar_t *wurl = (wchar_t *)malloc((strlen(url) + 1) * sizeof(wchar_t));
        if (wurl) {
            MultiByteToWideChar(CP_UTF8, 0, url, -1, wurl, strlen(url) + 1);
            wv->webview->Navigate(wurl);
            free(wurl);
        }
    }

#elif defined(SALAM_OS_LINUX)
    if (wv->webview) {
        webkit_web_view_load_uri(wv->webview, url);
    }

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSString *ns_url = [NSString stringWithUTF8String:url];
        NSURL *url_obj = [NSURL URLWithString:ns_url];
        NSURLRequest *request = [NSURLRequest requestWithURL:url_obj];
        [(WKWebView *)wv->webview loadRequest:request];
    }
#endif
}

void webview_init(void *w, const char *js)
{
    if (w == NULL || js == NULL) return;

    SalamWebView *wv = (SalamWebView *)w;

#if defined(SALAM_OS_WINDOWS)
    if (wv->webview) {
        wchar_t *wjs = (wchar_t *)malloc((strlen(js) + 1) * sizeof(wchar_t));
        if (wjs) {
            MultiByteToWideChar(CP_UTF8, 0, js, -1, wjs, strlen(js) + 1);
            wv->webview->ExecuteScript(wjs, NULL);
            free(wjs);
        }
    }

#elif defined(SALAM_OS_LINUX)
    if (wv->webview) {
        webkit_web_view_run_javascript(wv->webview, js, NULL, NULL, NULL);
    }

#elif defined(SALAM_OS_DARWIN)
    @autoreleasepool {
        NSString *ns_js = [NSString stringWithUTF8String:js];
        [(WKWebView *)wv->webview evaluateJavaScript:ns_js completionHandler:nil];
    }
#endif
}

void webview_eval(void *w, const char *js)
{
    webview_init(w, js);
}

void webview_bind(void *w, const char *name, void *fn, void *arg)
{
    /* Binding implementation would go here - deferred for basic version */
}

void webview_unbind(void *w, const char *name)
{
    /* Unbind implementation would go here - deferred for basic version */
}

void webview_return(void *w, const char *seq, int status, const char *result)
{
    /* Return implementation would go here - deferred for basic version */
}

/*
 * A stand-in for WebView2Loader.dll and the COM objects behind it, for testing
 * std/webview on Windows without the WebView2 runtime. Its methods are stdcall,
 * as COM methods are on 32-bit Windows, and it checks that the handlers it calls
 * back pop their own arguments there.
 *
 * tools/bash/native-windows-webview-check.sh builds and runs it.
 */

#define COBJMACROS
#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <windows.h>

#define VT_SLOTS 64

typedef struct {
    void **vtbl;
} Obj;

static int g_exec;
static int g_bad_stack;
static int g_bounds;

static void __stdcall bad_slot(void)
{
    printf("FAIL unexpected vtable slot\n");
    fflush(stdout);
    ExitProcess(3);
}

/* Bytes the callee left on the stack: 0 for stdcall, the argument size for cdecl. */
#if defined(__i386__)
static HRESULT call_com(void *fn, int nargs, void *a, void *b, void *c)
{
    HRESULT r;
    unsigned left;
    __asm__ volatile("movl %%esp, %%esi\n\t"
                     "cmpl $1, %[n]\n\t"
                     "je 1f\n\t"
                     "pushl %%edi\n\t"
                     "pushl %%edx\n\t"
                     "1:\n\t"
                     "pushl %%ebx\n\t"
                     "call *%%eax\n\t"
                     "movl %%esi, %%ecx\n\t"
                     "subl %%esp, %%ecx\n\t"
                     "movl %%esi, %%esp\n\t"
                     : "=a"(r), "=c"(left), "+d"(b), "+D"(c)
                     : "a"(fn), "b"(a), [n] "c"(nargs)
                     : "esi", "memory", "cc");
    if (left != 0) {
        printf("FAIL handler left %u bytes on the stack\n", left);
        g_bad_stack++;
    }
    return r;
}
#else
static HRESULT call_com(void *fn, int nargs, void *a, void *b, void *c)
{
    if (nargs == 1) return ((ULONG (*)(void *))fn)(a);
    return ((HRESULT (*)(void *, void *, void *))fn)(a, b, c);
}
#endif

static void complete(void *handler, void *result)
{
    void **vt = ((Obj *)handler)->vtbl;
    void *out = NULL;
    HRESULT qr = call_com(vt[0], 3, handler, NULL, &out);
    if (qr != S_OK || out != handler) printf("FAIL QueryInterface\n");
    call_com(vt[1], 1, handler, NULL, NULL);
    call_com(vt[2], 1, handler, NULL, NULL);
    if (call_com(vt[3], 3, handler, (void *)(ULONG_PTR)S_OK, result) != S_OK)
        printf("FAIL Invoke\n");
}

static void show(const char *what, const wchar_t *text)
{
    char buf[256];
    WideCharToMultiByte(CP_UTF8, 0, text, -1, buf, sizeof buf, NULL, NULL);
    printf("%s %s\n", what, buf);
}

static Obj g_webview;
static Obj g_controller;
static Obj g_env;
static Obj g_settings;
static Obj g_args;
static Obj g_request;
static Obj g_response;
static void *g_webview_vt[VT_SLOTS];
static void *g_controller_vt[VT_SLOTS];
static void *g_env_vt[VT_SLOTS];
static void *g_settings_vt[VT_SLOTS];
static void *g_args_vt[VT_SLOTS];
static void *g_request_vt[VT_SLOTS];
static void *g_response_vt[VT_SLOTS];
static void *g_nav_start;
static void *g_nav_done;
static void *g_message;
static void *g_new_window;
static void *g_resource;
static void *g_crash;
static const wchar_t *g_source = L"about:blank";
static const wchar_t *g_arg_text = L"";
static int g_cancelled;

static wchar_t *task_copy(const wchar_t *text)
{
    size_t n = (wcslen(text) + 1) * sizeof(wchar_t);
    wchar_t *out = CoTaskMemAlloc(n);
    memcpy(out, text, n);
    return out;
}

static HRESULT __stdcall ok_int(void *self, int value)
{
    (void)self;
    (void)value;
    return S_OK;
}

static HRESULT __stdcall ok_ref(void *self)
{
    (void)self;
    return 1;
}

static HRESULT __stdcall qi_self(void *self, void *iid, void **out)
{
    (void)iid;
    *out = self;
    return S_OK;
}

static HRESULT __stdcall args_get_text(void *self, wchar_t **out)
{
    (void)self;
    *out = task_copy(g_arg_text);
    return S_OK;
}

static HRESULT __stdcall args_cancel(void *self, int value)
{
    (void)self;
    g_cancelled = value;
    return S_OK;
}

static HRESULT __stdcall args_handled(void *self, int value)
{
    (void)self;
    printf("new window handled %d\n", value);
    return S_OK;
}

static HRESULT __stdcall args_get_request(void *self, void **out)
{
    (void)self;
    *out = &g_request;
    return S_OK;
}

static HRESULT __stdcall args_put_response(void *self, void *response)
{
    (void)self;
    if (response != &g_response) printf("FAIL put_Response\n");
    return S_OK;
}

static HRESULT __stdcall args_get_kind(void *self, int *out)
{
    (void)self;
    *out = 1;
    return S_OK;
}

static HRESULT __stdcall request_get_method(void *self, wchar_t **out)
{
    (void)self;
    *out = task_copy(L"GET");
    return S_OK;
}

static void fire(void *handler, const wchar_t *text)
{
    if (!handler) return;
    g_arg_text = text;
    if (call_com(((Obj *)handler)->vtbl[3], 3, handler, &g_webview, &g_args) != S_OK)
        printf("FAIL event Invoke\n");
}

static HRESULT __stdcall wv_navigate(Obj *self, const wchar_t *url)
{
    if (self != &g_webview) printf("FAIL Navigate this\n");
    show("navigate", url);
    g_cancelled = 0;
    fire(g_nav_start, url);
    if (g_cancelled) {
        printf("navigation cancelled\n");
        return S_OK;
    }
    g_source = _wcsdup(url);
    if (wcsstr(url, L".localhost")) {
        g_args_vt[3] = (void *)args_get_request;
        g_args_vt[5] = (void *)args_put_response;
        fire(g_resource, url);
        g_args_vt[3] = (void *)args_get_text;
    }
    g_args_vt[5] = (void *)args_get_text;
    fire(g_message, L"{\"hello\":1}");
    fire(g_new_window, L"https://popup.example/");
    g_args_vt[3] = (void *)args_get_kind;
    fire(g_crash, L"");
    g_args_vt[3] = (void *)args_get_text;
    fire(g_nav_done, url);
    return S_OK;
}

static HRESULT __stdcall wv_navigate_to_string(Obj *self, const wchar_t *html)
{
    if (self != &g_webview) printf("FAIL NavigateToString this\n");
    show("html", html);
    return S_OK;
}

static HRESULT __stdcall wv_execute_script(Obj *self, const wchar_t *js, void *handler)
{
    if (self != &g_webview) printf("FAIL ExecuteScript this\n");
    if (g_exec++ == 0) show("exec", js);
    complete(handler, (void *)L"null");
    return S_OK;
}

static HRESULT __stdcall wv_add_script(Obj *self, const wchar_t *js, void *handler)
{
    if (self != &g_webview) printf("FAIL AddScript this\n");
    show("script", js);
    complete(handler, (void *)L"id");
    return S_OK;
}

static HRESULT __stdcall wv_get_settings(Obj *self, void **out)
{
    if (self != &g_webview) printf("FAIL get_Settings this\n");
    *out = &g_settings;
    return S_OK;
}

static HRESULT __stdcall wv_get_source(Obj *self, wchar_t **out)
{
    if (self != &g_webview) printf("FAIL get_Source this\n");
    *out = task_copy(g_source);
    return S_OK;
}

#define ADD_EVENT(name, slot)                                                            \
    static HRESULT __stdcall name(Obj *self, void *handler, void *token)                 \
    {                                                                                    \
        if (self != &g_webview || !handler || !token) printf("FAIL " #name "\n");        \
        slot = handler;                                                                  \
        return S_OK;                                                                     \
    }

ADD_EVENT(wv_add_nav_start, g_nav_start)
ADD_EVENT(wv_add_nav_done, g_nav_done)
ADD_EVENT(wv_add_message, g_message)
ADD_EVENT(wv_add_new_window, g_new_window)
ADD_EVENT(wv_add_resource, g_resource)
ADD_EVENT(wv_add_crash, g_crash)

static HRESULT __stdcall wv_add_filter(Obj *self, const wchar_t *uri, int kind)
{
    if (self != &g_webview || kind != 0) printf("FAIL filter\n");
    show("filter", uri);
    return S_OK;
}

static HRESULT __stdcall ctrl_put_bounds(Obj *self, RECT r)
{
    if (self != &g_controller) printf("FAIL put_Bounds this\n");
    if (r.left != 0 || r.top != 0) printf("FAIL put_Bounds origin\n");
    g_bounds++;
    return S_OK;
}

static HRESULT __stdcall ctrl_get_webview(Obj *self, void **out)
{
    if (self != &g_controller) printf("FAIL get_CoreWebView2 this\n");
    *out = &g_webview;
    return S_OK;
}

static HRESULT __stdcall ctrl_close(Obj *self)
{
    if (self != &g_controller) printf("FAIL Close this\n");
    printf("close exec=%d bad_stack=%d bounds=%d\n", g_exec, g_bad_stack, g_bounds > 0);
    return S_OK;
}

static HRESULT __stdcall env_create_controller(Obj *self, HWND parent, void *handler)
{
    if (self != &g_env || parent == NULL)
        printf("FAIL CreateCoreWebView2Controller args\n");
    complete(handler, &g_controller);
    return S_OK;
}

static HRESULT __stdcall env_create_response(Obj *self, IStream *content, int status,
                                             const wchar_t *reason,
                                             const wchar_t *headers, void **out)
{
    char body[32] = {0};
    ULONG got = 0;
    if (self != &g_env) printf("FAIL CreateWebResourceResponse this\n");
    if (content) content->lpVtbl->Read(content, body, sizeof body - 1, &got);
    printf("response %d body=%s\n", status, body);
    show("reason", reason);
    show("headers", headers);
    *out = &g_response;
    return S_OK;
}

static void fill(void **vt)
{
    for (int i = 0; i < VT_SLOTS; i++)
        vt[i] = (void *)bad_slot;
    vt[0] = (void *)qi_self;
    vt[1] = (void *)ok_ref;
    vt[2] = (void *)ok_ref;
}

__declspec(dllexport) HRESULT __stdcall
CreateCoreWebView2EnvironmentWithOptions(const wchar_t *browser, const wchar_t *data,
                                         void *options, void *handler)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (browser != NULL || options != NULL) printf("FAIL environment args\n");
    if (data != NULL) show("data dir", data);
    fill(g_env_vt);
    fill(g_controller_vt);
    fill(g_webview_vt);
    fill(g_settings_vt);
    fill(g_args_vt);
    fill(g_request_vt);
    fill(g_response_vt);
    g_env.vtbl = g_env_vt;
    g_controller.vtbl = g_controller_vt;
    g_webview.vtbl = g_webview_vt;
    g_settings.vtbl = g_settings_vt;
    g_args.vtbl = g_args_vt;
    g_request.vtbl = g_request_vt;
    g_response.vtbl = g_response_vt;
    g_env_vt[3] = (void *)env_create_controller;
    g_env_vt[4] = (void *)env_create_response;
    g_controller_vt[6] = (void *)ctrl_put_bounds;
    g_controller_vt[12] = (void *)ok_int;
    g_controller_vt[24] = (void *)ctrl_close;
    g_controller_vt[25] = (void *)ctrl_get_webview;
    g_settings_vt[10] = (void *)ok_int;
    g_settings_vt[12] = (void *)ok_int;
    g_settings_vt[14] = (void *)ok_int;
    g_webview_vt[3] = (void *)wv_get_settings;
    g_webview_vt[4] = (void *)wv_get_source;
    g_webview_vt[5] = (void *)wv_navigate;
    g_webview_vt[6] = (void *)wv_navigate_to_string;
    g_webview_vt[7] = (void *)wv_add_nav_start;
    g_webview_vt[15] = (void *)wv_add_nav_done;
    g_webview_vt[25] = (void *)wv_add_crash;
    g_webview_vt[27] = (void *)wv_add_script;
    g_webview_vt[29] = (void *)wv_execute_script;
    g_webview_vt[34] = (void *)wv_add_message;
    g_webview_vt[44] = (void *)wv_add_new_window;
    g_webview_vt[55] = (void *)wv_add_resource;
    g_webview_vt[57] = (void *)wv_add_filter;
    g_args_vt[3] = (void *)args_get_text;
    g_args_vt[5] = (void *)args_get_text;
    g_args_vt[6] = (void *)args_handled;
    g_args_vt[7] = (void *)bad_slot;
    g_args_vt[8] = (void *)args_cancel;
    g_args_vt[5] = (void *)args_get_text;
    g_request_vt[3] = (void *)args_get_text;
    g_request_vt[5] = (void *)request_get_method;
    complete(handler, &g_env);
    return S_OK;
}

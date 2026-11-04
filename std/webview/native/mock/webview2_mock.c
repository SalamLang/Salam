/*
 * A stand-in for WebView2Loader.dll and the COM objects behind it, for testing
 * std/webview on Windows without the WebView2 runtime. Its methods are stdcall,
 * as COM methods are on 32-bit Windows, and it checks that the handlers it calls
 * back pop their own arguments there.
 *
 * tools/bash/native-windows-webview-check.sh builds and runs it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define VT_SLOTS 32

typedef struct {
    void **vtbl;
} Obj;

static int g_exec;
static int g_bad_stack;

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
static void *g_webview_vt[VT_SLOTS];
static void *g_controller_vt[VT_SLOTS];
static void *g_env_vt[VT_SLOTS];

static HRESULT __stdcall wv_navigate(Obj *self, const wchar_t *url)
{
    if (self != &g_webview) printf("FAIL Navigate this\n");
    show("navigate", url);
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

static HRESULT __stdcall ctrl_put_bounds(Obj *self, RECT r)
{
    if (self != &g_controller) printf("FAIL put_Bounds this\n");
    printf("bounds %ld %ld %ld %ld\n", r.left, r.top, r.right, r.bottom);
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
    printf("close exec=%d bad_stack=%d\n", g_exec, g_bad_stack);
    return S_OK;
}

static HRESULT __stdcall env_create_controller(Obj *self, HWND parent, void *handler)
{
    if (self != &g_env || parent == NULL)
        printf("FAIL CreateCoreWebView2Controller args\n");
    complete(handler, &g_controller);
    return S_OK;
}

static void fill(void **vt)
{
    for (int i = 0; i < VT_SLOTS; i++)
        vt[i] = (void *)bad_slot;
}

__declspec(dllexport) HRESULT __stdcall
CreateCoreWebView2EnvironmentWithOptions(const wchar_t *browser, const wchar_t *data,
                                         void *options, void *handler)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    if (browser != NULL || data != NULL || options != NULL)
        printf("FAIL environment args\n");
    fill(g_env_vt);
    fill(g_controller_vt);
    fill(g_webview_vt);
    g_env.vtbl = g_env_vt;
    g_controller.vtbl = g_controller_vt;
    g_webview.vtbl = g_webview_vt;
    g_env_vt[3] = (void *)env_create_controller;
    g_controller_vt[6] = (void *)ctrl_put_bounds;
    g_controller_vt[24] = (void *)ctrl_close;
    g_controller_vt[25] = (void *)ctrl_get_webview;
    g_webview_vt[5] = (void *)wv_navigate;
    g_webview_vt[6] = (void *)wv_navigate_to_string;
    g_webview_vt[29] = (void *)wv_execute_script;
    complete(handler, &g_env);
    return S_OK;
}

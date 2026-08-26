/*
 * Salam Programming Language (2024-2026)
 *
 * CEF implementation of cef_shim.h. Built against the CEF binary
 * distribution; see BUILD.md.
 *
 * Two CEF requirements shape this file:
 *
 *  1. Multi-process. CEF re-executes the program for its renderer, GPU and
 *     utility processes. Rather than force every Salam program to call
 *     CefExecuteProcess as the first statement of main - which the language
 *     cannot enforce and macOS forbids anyway - we point
 *     CefSettings.browser_subprocess_path at the separate salam_cef_helper
 *     executable built alongside this library. The host program's main is
 *     then left alone.
 *
 *  2. Windowing. Instead of per-platform native window code (HWND / GtkWindow
 *     / NSWindow) this uses CEF's Views framework, which is the one window
 *     API CEF offers on all three desktop platforms.
 *
 * The message loop is driven by CefDoMessageLoopWork from salam_wv_cef_pump
 * rather than CefRunMessageLoop, so that the caller's loop stays in control
 * and WebView.Terminate() can end it.
 */

#include "cef_shim.h"

#include <cstdlib>
#include <string>

#include "include/cef_app.h"
#include "include/cef_browser.h"
#include "include/cef_client.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_browser_view_delegate.h"
#include "include/views/cef_window.h"
#include "include/views/cef_window_delegate.h"
#include "include/wrapper/cef_helpers.h"

namespace {

// ---------------------------------------------------------------- base64 --
// CEF removed CefFrame::LoadString, and loading HTML from memory is now done
// with a data: URL. Keep the encoder local so the shim has no dependencies
// beyond CEF itself.
const char kB64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Base64(const std::string &in) {
  std::string out;
  out.reserve(((in.size() + 2) / 3) * 4);
  size_t i = 0;
  while (i + 2 < in.size()) {
    unsigned v = (static_cast<unsigned char>(in[i]) << 16) |
                 (static_cast<unsigned char>(in[i + 1]) << 8) |
                 static_cast<unsigned char>(in[i + 2]);
    out += kB64[(v >> 18) & 63];
    out += kB64[(v >> 12) & 63];
    out += kB64[(v >> 6) & 63];
    out += kB64[v & 63];
    i += 3;
  }
  if (i + 1 == in.size()) {
    unsigned v = static_cast<unsigned char>(in[i]) << 16;
    out += kB64[(v >> 18) & 63];
    out += kB64[(v >> 12) & 63];
    out += "==";
  } else if (i + 2 == in.size()) {
    unsigned v = (static_cast<unsigned char>(in[i]) << 16) |
                 (static_cast<unsigned char>(in[i + 1]) << 8);
    out += kB64[(v >> 18) & 63];
    out += kB64[(v >> 12) & 63];
    out += kB64[(v >> 6) & 63];
    out += '=';
  }
  return out;
}

// ------------------------------------------------------------------- app --
class ShimApp : public CefApp, public CefBrowserProcessHandler {
 public:
  ShimApp() {}
  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

 private:
  IMPLEMENT_REFCOUNTING(ShimApp);
  DISALLOW_COPY_AND_ASSIGN(ShimApp);
};

// ---------------------------------------------------------------- client --
// One per window. Holds the browser and the closed flag that
// salam_wv_cef_is_closed reports.
class ShimClient : public CefClient, public CefLifeSpanHandler {
 public:
  ShimClient() : closed_(false) {}

  CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }

  void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
    CEF_REQUIRE_UI_THREAD();
    if (!browser_) {
      browser_ = browser;
    }
  }

  void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
    CEF_REQUIRE_UI_THREAD();
    browser_ = nullptr;
    closed_ = true;
  }

  CefRefPtr<CefBrowser> browser() const { return browser_; }
  bool closed() const { return closed_; }
  void set_closed() { closed_ = true; }

 private:
  CefRefPtr<CefBrowser> browser_;
  bool closed_;

  IMPLEMENT_REFCOUNTING(ShimClient);
  DISALLOW_COPY_AND_ASSIGN(ShimClient);
};

// ---------------------------------------------------------------- window --
class ShimWindowDelegate : public CefWindowDelegate {
 public:
  ShimWindowDelegate(CefRefPtr<CefBrowserView> view, int width, int height,
                     bool fixed)
      : browser_view_(view), width_(width), height_(height), fixed_(fixed) {}

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    window->AddChildView(browser_view_);
    window->Show();
    browser_view_->RequestFocus();
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(width_, height_);
  }

  bool CanResize(CefRefPtr<CefWindow> window) override { return !fixed_; }
  bool CanClose(CefRefPtr<CefWindow> window) override { return true; }

  void set_size(int w, int h, bool fixed) {
    width_ = w;
    height_ = h;
    fixed_ = fixed;
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  int width_;
  int height_;
  bool fixed_;

  IMPLEMENT_REFCOUNTING(ShimWindowDelegate);
  DISALLOW_COPY_AND_ASSIGN(ShimWindowDelegate);
};

// ------------------------------------------------------------------ view --
struct View {
  CefRefPtr<ShimClient> client;
  CefRefPtr<CefWindow> window;
  CefRefPtr<ShimWindowDelegate> delegate;
};

int g_init_state = -1;  // -1 not tried, 0 failed, 1 ok
int g_live_views = 0;

View *AsView(void *p) { return static_cast<View *>(p); }

CefRefPtr<CefFrame> MainFrame(void *p) {
  View *v = AsView(p);
  if (!v || !v->client || v->client->closed()) {
    return nullptr;
  }
  CefRefPtr<CefBrowser> b = v->client->browser();
  if (!b) {
    return nullptr;
  }
  return b->GetMainFrame();
}

}  // namespace

// ------------------------------------------------------------------- ABI --

extern "C" int salam_wv_cef_init(int debug) {
  if (g_init_state >= 0) {
    return g_init_state;
  }

  CefMainArgs main_args;
  CefSettings settings;
  settings.no_sandbox = true;
  settings.multi_threaded_message_loop = false;
  settings.external_message_pump = false;
  settings.log_severity = debug ? LOGSEVERITY_INFO : LOGSEVERITY_ERROR;
  settings.remote_debugging_port = debug ? 9222 : 0;

  // Keeps the host program's main() free of CefExecuteProcess - see the file
  // comment. BUILD.md explains where the helper must sit at run time.
  const char *helper = getenv("SALAM_CEF_HELPER");
  if (helper && *helper) {
    CefString(&settings.browser_subprocess_path) = helper;
  }
  const char *root = getenv("SALAM_CEF_ROOT");
  if (root && *root) {
    std::string base(root);
    CefString(&settings.resources_dir_path) = base;
    CefString(&settings.locales_dir_path) = base + "/locales";
  }

  CefRefPtr<ShimApp> app(new ShimApp);
  g_init_state = CefInitialize(main_args, settings, app.get(), nullptr) ? 1 : 0;
  return g_init_state;
}

extern "C" void *salam_wv_cef_create(int width, int height, int debug) {
  if (salam_wv_cef_init(debug) != 1) {
    return nullptr;
  }
  if (width <= 0) {
    width = 800;
  }
  if (height <= 0) {
    height = 600;
  }

  View *v = new View();
  v->client = new ShimClient();

  CefBrowserSettings browser_settings;
  CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
      v->client, "about:blank", browser_settings, nullptr, nullptr, nullptr);
  if (!browser_view) {
    delete v;
    return nullptr;
  }

  v->delegate = new ShimWindowDelegate(browser_view, width, height, false);
  v->window = CefWindow::CreateTopLevelWindow(v->delegate);
  if (!v->window) {
    delete v;
    return nullptr;
  }

  g_live_views++;
  return v;
}

extern "C" void salam_wv_cef_destroy(void *view) {
  View *v = AsView(view);
  if (!v) {
    return;
  }
  if (v->window) {
    v->window->Close();
    v->window = nullptr;
  }
  if (v->client) {
    v->client->set_closed();
  }
  v->delegate = nullptr;
  v->client = nullptr;
  delete v;

  if (--g_live_views <= 0) {
    g_live_views = 0;
    if (g_init_state == 1) {
      // Let the close propagate before tearing CEF down.
      for (int i = 0; i < 32; i++) {
        CefDoMessageLoopWork();
      }
      CefShutdown();
      g_init_state = -1;
    }
  }
}

extern "C" void salam_wv_cef_pump(void) {
  if (g_init_state == 1) {
    CefDoMessageLoopWork();
  }
}

extern "C" int salam_wv_cef_is_closed(void *view) {
  View *v = AsView(view);
  if (!v || !v->client) {
    return 1;
  }
  return v->client->closed() ? 1 : 0;
}

extern "C" void salam_wv_cef_set_title(void *view, const char *title) {
  View *v = AsView(view);
  if (!v || !v->window || !title) {
    return;
  }
  v->window->SetTitle(CefString(title));
}

extern "C" void salam_wv_cef_set_size(void *view, int width, int height,
                                      int fixed) {
  View *v = AsView(view);
  if (!v || !v->window || !v->delegate) {
    return;
  }
  if (width <= 0 || height <= 0) {
    return;
  }
  v->delegate->set_size(width, height, fixed != 0);
  v->window->SetSize(CefSize(width, height));
}

extern "C" void salam_wv_cef_set_html(void *view, const char *html) {
  CefRefPtr<CefFrame> frame = MainFrame(view);
  if (!frame || !html) {
    return;
  }
  std::string url = "data:text/html;charset=utf-8;base64,";
  url += Base64(std::string(html));
  frame->LoadURL(CefString(url));
}

extern "C" void salam_wv_cef_navigate(void *view, const char *url) {
  CefRefPtr<CefFrame> frame = MainFrame(view);
  if (!frame || !url) {
    return;
  }
  frame->LoadURL(CefString(url));
}

extern "C" void salam_wv_cef_eval(void *view, const char *js) {
  CefRefPtr<CefFrame> frame = MainFrame(view);
  if (!frame || !js) {
    return;
  }
  frame->ExecuteJavaScript(CefString(js), frame->GetURL(), 0);
}

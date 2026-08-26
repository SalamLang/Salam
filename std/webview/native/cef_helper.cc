/*
 * Salam Programming Language (2024-2026)
 *
 * CEF subprocess helper.
 *
 * CEF runs its renderer, GPU and utility work in separate processes. By
 * default it re-executes the host program, which would require every Salam
 * program to call CefExecuteProcess as the very first thing in main. Instead
 * cef_shim.cc sets CefSettings.browser_subprocess_path to this executable, so
 * CEF spawns this instead and the host's main is never re-entered.
 *
 * On macOS a separate helper is not an optimization but a requirement: the
 * helper must be its own .app bundle. See BUILD.md.
 */

#include "include/cef_app.h"

int main(int argc, char *argv[]) {
#if defined(_WIN32)
  CefMainArgs main_args(GetModuleHandle(nullptr));
#else
  CefMainArgs main_args(argc, argv);
#endif
  return CefExecuteProcess(main_args, nullptr, nullptr);
}

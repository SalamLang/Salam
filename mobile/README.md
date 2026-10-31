# Salam mobile apps

Native Salam IDEs for Android (`android/`) and iOS (`ios/`). Both carry the
whole Salam compiler inside the app, so code is highlighted, checked and run
offline. The old WebView wrapper around the online editor lives in
`android-webview/` for reference.

## How it fits together

```
android/ (Kotlin, framework Views)      ios/ (Swift, UIKit)
            |  JNI                            |  C
            +---------- mobile/engine --------+
                 salam_engine.c / salam_engine.h
                            |
          compiler/mobile_main.salam  ->  emitted C
          lexer, parser, sema, interpreter, fmt, translate, emit js/llvm/c
```

- `compiler/mobile_main.salam` is a library entry point next to
  `compiler/sal_web.salam`. Its `export:` block exposes `salam_m_tokens`
  (highlight spans in UTF-16 offsets), `salam_m_check` (JSON diagnostics),
  `salam_m_cli` (the full command line, run in-process) and
  `salam_m_stop`.
- `tools/bash/build-mobile-engine.sh` emits that entry as C for one target
  triple and stages a minified `std/`.
- `mobile/engine/` wraps the emitted C: `salam_engine.c` boots the module
  initializers once, `salam_jni.c` is the Android bridge (it converts Java
  strings to real UTF-8), and `android_compat.c` / `ios_compat.c` fill the
  few libc gaps of each platform.
- Programs run in the interpreter. Neither platform lets an app run a
  freshly built native binary, so `salam build` stays a desktop command;
  everything else the CLI does works on the phone.
- When embedded, `exit()` and `abort()` in a program end the run instead
  of the app, and Stop sets a flag the interpreter checks every few
  thousand steps. On Android runs happen in a separate `:engine` process,
  so a crash or a stuck native call only takes that process down.

## Android

Needs JDK 21, the Android SDK with NDK 30 and CMake 4.1 (Gradle installs
what is missing), and a `salam` compiler (`./salam` at the repository root,
or set `SALAM`).

```
cd android
./gradlew :app:assembleRelease                    # all four ABIs
./gradlew :app:assembleRelease -PsalamAbis=arm64-v8a
```

minSdk is 21 (Android 5.0). Release builds produce one APK per ABI (about
3 MB each) plus a universal APK and an AAB.

## iOS

Needs macOS with Xcode, XcodeGen and a `salam` compiler.

```
sh ios/build-engine.sh
cd ios
SALAM_VERSION=$(cat ../VERSION) SALAM_BUILD=505 xcodegen generate
open Salam.xcodeproj
```

The deployment target is iOS 15, the oldest version current Xcode can
still ship to the App Store. That covers the iPhone 6s and newer; iPhone 6
users can add the online editor to their home screen instead. Signing and
TestFlight need the team's Apple developer account.

## Testing the engine on a desktop

```
mobile/engine/test/run.sh
```

builds the engine for the host and checks tokens, diagnostics, stdin, exit
codes and Stop through the same C API the apps use.

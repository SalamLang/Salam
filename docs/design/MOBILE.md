# Android and iOS apps from the page language

Status: plan. The native UI draws every pixel itself (`gui.paint`) and the
window layer only shows a pixel buffer and delivers events (`gui.window`). A
phone is one more window backend under the same renderer. No XML layouts and
no generated Java or Swift per app.

## How an app is built

```
page -> native generator -> Salam -> emitted C -> NDK / Xcode clang -> library
                                                                          |
            prebuilt host shell (Kotlin / Swift) owns screen, touch, IME, lifecycle
```

- The app is ahead of time compiled. Neither OS runs a freshly built binary,
  so `salam run` on a phone keeps using the interpreter (`mobile/engine`).
- The host shell is written once and shipped as a template. It is not
  generated per app. The only generated file on Android is
  `AndroidManifest.xml`.
- Widgets are not mapped to Views, Compose or SwiftUI. Every property works
  the same as on desktop and the web target.

## Building for Android

```
tools/bash/build-android-app.sh app.salam --out out --abi arm64-v8a
adb install out/app.apk
```

Needs a `salam` compiler, the Android NDK, a JDK and the Android SDK. The page
is the same file that runs on the web and on the desktop. The script emits C
per ABI, builds `libsalamapp.so` with the NDK and packages it with the
Gradle project in `mobile/host/android/project`.

## Building for iOS

```
tools/bash/build-ios-app.sh app.salam --out out
xcrun simctl install booted out/App.app
```

Needs a Mac with Xcode. The default is the simulator; `--device` builds for a
phone and the app still has to be signed in Xcode. iOS uses the same window
backend as Android (`std/gui/window/mobile.salam`) over the same C calls; the
UIKit side is `mobile/host/ios/salam_host.m`. `SALAM_OS_IOS` is defined by
the compiler for iOS targets, and std tests it as `defined(SALAM_OS_IOS)`.

## Threading

`gui.window` is pump based (`_nat_pump`, `_nat_wake`); phones are callback
based. The UI loop runs on its own native thread. The shell pushes touch,
IME, surface and lifecycle events into a queue and wakes that thread. Frames
are blitted from the Salam thread (`ANativeWindow_lock` on Android, a
`CALayer` image on iOS). `gui.ui` does not change.

## Stages

Each stage is one pull request and is checked on the desktop first.

1. This document.
2. Pointer id and pointer kind (mouse, touch, pen) on `Event`; headless
   `touch down|move|up` script steps.
3. Gestures in `gui.ui`: tap, long press, drag scroll, fling.
4. Safe area and keyboard insets as window values.
5. Lifecycle events: pause, resume, save state, low memory.
6. Android window backend (`std/gui/window/android.salam`) and the Kotlin
   shell with a `SurfaceView`.
7. `salam build --ui=native --target=<android triple>` and APK packaging
   through a Gradle template.
8. Android IME through `InputConnection`, bundled fonts, back button to the
   page router.
9. iOS backend (`UIView`, `CADisplayLink`, `UITextInput`) and an Xcode
   template, tested on the simulator.
10. Host service channel (share, permissions, camera, notifications) as a
    `std/mobile` package.

## Status

Done: stages 1 to 7, the Android part of 8 (keyboard, back button, system
fonts) and clipboard and link opening from stage 10. Checked on an arm64
emulator with a counter page, a form with a text input and a Persian page.
Stage 9: the iOS host draws the counter page on the iPhone simulator. Touch,
the keyboard and Persian text were not tried on iOS yet.
Missing: bundled fonts (iOS cannot read system fonts on a phone), pinch and
other multi touch gestures, share, permissions, camera and notifications.

## Target matrix

| thing                        | desktop | Android      | iOS            |
| ---------------------------- | ------- | ------------ | -------------- |
| window backend               | x11, win32, cocoa | Android | uikit          |
| input                        | mouse, keys | touch, IME | touch, IME     |
| frame output                 | native blit | ANativeWindow | CALayer    |
| system fonts                 | read from disk | `/system/fonts` | bundled only |
| `gui.webview`, tray, Wi-Fi    | yes     | no           | no             |

## Not planned

- Compiling and running native code on the phone.
- Phone versions of desktop only packages.
- Our own Android `.so` linker. The NDK path comes first; the 16 KB page
  alignment Play requires is a separate project.

## Risks

- Software rendering a 1080x2400 buffer at 120 Hz. Damage rects first, then
  cached scroll layers, later a GPU upload.
- IME composition for Persian needs a Java `InputConnection`, so the shell
  cannot be pure `NativeActivity`.
- iOS has no system font files and needs bundled CA certificates for TLS.

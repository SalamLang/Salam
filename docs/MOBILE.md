# Salam apps on Android

The page you write for the web and the desktop (`salam run page.salam --ui=native`)
runs unchanged on Android. The window layer draws the same pixels and the
phone only shows them and sends touch, keyboard and lifecycle events.

## Build and run

You need a `salam` compiler, the Android SDK with an NDK, and a JDK 21.

```
salam app android app.salam --out out --run
```

A release archive carries the build files next to `std/`; from a source
checkout the compiler finds them too. Set `SALAM_MOBILE_ROOT` to point at
another folder. `tools/bash/build-android-app.sh` takes the same options.

This builds `out/app.apk` for arm64, installs it on the connected phone or
emulator and starts it. Other options:

| option | meaning |
| --- | --- |
| `--abi arm64-v8a,x86_64` | ABIs to build (release builds default to arm64-v8a, armeabi-v7a and x86_64) |
| `--id ir.example.app` | application id |
| `--name "My App"` | label under the icon |
| `--icon icon.png` | launcher icon |
| `--release --keystore my.jks --keystore-pass P --key-alias A` | signed release APK |
| `--aab` | App Bundle for Google Play |

What the program prints goes to logcat: `adb logcat -s salam`.

## What works

- Layout, text (Persian included, system fonts), colors, scrolling with fling,
  media queries and `page.touch`.
- Touch: tap, long press, drag scroll. Long press in a text input selects the
  word.
- The soft keyboard follows the focused input; the page root keeps away from
  the status bar, the navigation bar and the keyboard.
- Back goes to the page router first and closes the app at the root.
- Pause, resume and low memory hooks (`ui.OnPause` and friends), rotation, dark
  mode.
- Standard library: files, threads, timers, sqlite, DNS, http and https,
  clipboard and opening links.

## Not available yet

- Pinch and other multi touch gestures.
- Camera, location, share sheet, notifications and permissions.
- Bundled fonts (the phone's system fonts are used).
- `salam run` on the phone itself; build with the script above.

The plan and the design are in `design/MOBILE.md`.

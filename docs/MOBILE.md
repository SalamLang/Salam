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

## iOS

```
salam app ios app.salam --out out
xcrun simctl install booted out/App.app
xcrun simctl launch booted app.salam.app
```

`tools/bash/build-ios-app.sh` takes the same options. The default target is the simulator; `--device` builds for a phone and the app
still has to be signed in Xcode. Needs a compiler built from the current tree.
Checked on the simulator with the counter page: drawing, safe area and a
tap (`-salam-tap X,Y` launch argument). The keyboard, Persian text, pause and
resume and the clipboard are written but not tried yet. There is no
composition text, and no bundled fonts.

## What works

- Layout, text (Persian included, system fonts), colors, scrolling with fling,
  media queries and `page.touch`.
- Touch: tap, long press, drag scroll. Long press in a text input selects the
  word.
- Pinch: a second finger starts a pinch and cancels the tap, long press and
  drag of the first. Elements get `pinch` while the fingers move and
  `pinchend` when one lifts; both bubble from the element under the middle of
  the fingers. `ui.CurrentEvent().scale` is the finger distance divided by the
  distance at the start (above 1 spreading, below 1 pinching in) and `x`, `y`
  are the middle. In a headless script, `pinch #id 2` spreads two fingers.

  ```
  box:
      pinch = (): zoom = ui.CurrentEvent().scale end
  end
  ```

- The soft keyboard follows the focused input; the page root keeps away from
  the status bar, the navigation bar and the keyboard.
- Back goes to the page router first and closes the app at the root.
- Pause, resume and low memory hooks (`ui.OnPause` and friends), rotation, dark
  mode.
- Standard library: files, threads, timers, sqlite, DNS, http and https,
  clipboard and opening links.
- Device features through `gui.device` (same calls on every platform, false
  or empty where a system has no such feature): `device.Share(text, url)`
  opens the share sheet, `device.Haptic(kind)` and `device.Vibrate(ms)` give
  feedback (the build adds the `VIBRATE` permission only when you call
  `Vibrate`). `device.CheckPermission` and `device.RequestPermission` handle
  runtime permissions; declare them with `--permission camera` (also
  `microphone`, `location`, `notifications`) when you build, and call
  `device.OnChange` to react when the dialog closes. `device.ShowNotification`,
  `device.ScheduleNotification` and `device.CancelNotification` post local
  notifications (the build declares `POST_NOTIFICATIONS` when you use them).
  `device.RequestLocation`, `device.LocationStatus` and `device.LastLocation`
  give a one shot position (the build declares the location permissions). `device.PickPhoto` and
  `device.TakePhoto` open the system photo picker and the camera app, then
  `device.LastPhoto` is the path of a JPEG copy in the cache directory.

## Not available yet

- Rotate and three finger gestures.
- Bundled fonts (the phone's system fonts are used).
- `salam run` on the phone itself; build with the script above.

The plan and the design are in `design/MOBILE.md`.

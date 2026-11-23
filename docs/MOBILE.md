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
| `--icon icon.png` | one square PNG (1024 px is best): adaptive icon with a themed (monochrome) layer plus legacy mipmaps |
| `--icon-bg "#2A6DF4"` | background colour of the adaptive icon (default white) |
| `--splash-color "#101820"` | launch screen colour (default white, black in the dark theme) |
| `--splash-icon logo.png` | launch screen icon (default: `--icon`) |
| `--release --keystore my.jks --keystore-pass P --key-alias A` | signed release APK |
| `--aab` | App Bundle for Google Play |

## Version, identity and salam.toml

| option | Android | iOS |
| --- | --- | --- |
| `--version 1.2.0` | `versionName` | `CFBundleShortVersionString` |
| `--build 7` | `versionCode` | `CFBundleVersion` |
| `--min-sdk 26` / `--min-ios 16.0` | `minSdk` (21 or more) | `MinimumOSVersion` and the compile target (15.0) |
| `--target-sdk 35` | `targetSdk` | |
| `--orientation portrait\|landscape\|auto` | `screenOrientation` | supported orientations |
| `--category productivity` | `appCategory` (game, social, productivity, music, video, photo, news, maps) | `LSApplicationCategoryType` (also utilities, education, finance, health, lifestyle, travel, weather, business, entertainment, sports, books, food, reference) |

Options do not have to be repeated: a `salam.toml` next to the page (or `--config FILE`)
can hold them. `id`, `name`, `version` and `icon` come from the `[app]` table the desktop
bundle uses; everything else goes in `[app.mobile]`, and `[app.android]` or `[app.ios]`
overrides it for one platform. Keys are the option names with underscores.

```toml
[app]
id = "ir.example.notes"
name = "Notes"
version = "1.2.0"
icon = "icon.png"

[app.mobile]
build = 7
orientation = "portrait"
splash_color = "#101820"
permissions = ["camera"]

[app.android]
min_sdk = 26
abi = "arm64-v8a,x86_64"

[app.ios]
min_ios = "16.0"
```

An option on the command line wins over the file, and the file wins over the built-in default.
Paths in the file (`icon`, `splash_icon`, `fonts`, `assets`) are relative to the file.

Files such as images go in the APK with `--assets DIR` (`tools/bash/build-android-app.sh`).
They are copied to `$HOME/assets` when the app starts, and a page names one with
the `asset:` prefix: `image: url = "asset:logo.png"` for `DIR/logo.png` (folders
keep their names, `asset:icons/menu.png`). `--fonts DIR` does the same for `.ttf`,
`.otf` and `.ttc` files. A path to any other readable file works too. Images are
PNG, JPEG, GIF, BMP or WebP; SVG is not supported on the native UI, so convert it
to PNG (the run prints `ui: image NAME: SVG is not supported...` and the image
stays empty). An image that cannot be read draws a grey box when it has a size.

On both platforms the host remembers the page background it drew last and paints it behind
the next launch, so a dark page does not start with a white frame.

What the program prints goes to logcat: `adb logcat -s salam`.

To try touch on an emulator without a tap tool, start the activity with a
`salam-script` extra. It takes the same steps as the iOS `-salam-script`
(table below) and sends them through the same event queue as real touches,
after the first drawn frame. Coordinates are dp, like iOS points:

```
adb shell "am start -n app.example.app/ir.salamlang.host.SalamActivity --es salam-script 'wait 1000;tap 100,200;swipe 100,500,100,100'"
```

Quote the whole script for the device shell, since `;` ends a command there.
Without the extra the host reads the system property `debug.salam.script`
(`adb shell setprop debug.salam.script 'tap 100,200'`, 91 characters at most)
and then the file `files/salam-script` in the app directory
(`adb shell run-as PKG sh -c 'cat > files/salam-script'`). The script runs once
per process. The parser is in `mobile/host/common/salam_host_common.c`.

## iOS

```
salam app ios app.salam --out out
xcrun simctl install booted out/App.app
xcrun simctl launch booted app.salam.app
```

`tools/bash/build-ios-app.sh` takes `--out`, `--device`, `--id`, `--name` and `--permission NAME`
(`camera`, `microphone`, `location`, `notifications`) for the usage description
the system shows in its permission dialog. The build adds
`NSLocationWhenInUseUsageDescription` when the program calls
`device.RequestLocation`, and the camera and photo library descriptions when
it calls `device.PickPhoto` or `device.TakePhoto`. The default target is the
simulator; `--device` builds for a phone and the app still has to be signed in
Xcode. `--icon icon.png` (one square PNG, 1024 px is best) is compiled into an
AppIcon asset catalog with actool, or written as loose PNG files when actool is missing.
`--splash-color` and `--splash-icon` fill the `UILaunchScreen` dictionary through the same
catalog; without them the launch screen is the system background.
Needs a compiler built from the current tree.

The device calls (`gui.device`) work the same as on Android: share sheet,
haptics, permissions, local notifications, one shot location, photo picker and
camera. Composition text from keyboards such as Chinese or Japanese shows as
preedit, and the caret position goes to the keyboard. There are no bundled
fonts.

To try touch on the simulator without a tap tool, launch with
`-salam-script` (points, steps separated by `;`, run after the first frame):

```
xcrun simctl launch booted app.salam.app -salam-script "tap 100,200;swipe 100,500,100,100;pinch 200,400,2.0"
```

| step | meaning |
| --- | --- |
| `tap X,Y` | one tap |
| `long X,Y[,MS]` | press and hold (900 ms by default) |
| `swipe X1,Y1,X2,Y2[,MS]` | drag one finger (300 ms by default) |
| `pinch X,Y,SCALE[,MS]` | two fingers around the point, spreading to SCALE times the start distance |
| `type TEXT`, `mark TEXT`, `unmark`, `back` | text, composition text, commit it, delete backward |
| `wait MS` | pause |

`-salam-tap X,Y` still works for one tap.

The script starts 1.5 s after the first frame, so on a slow or busy machine put
a `wait` first (`wait 3000;tap 100,200`). It sends touches to the app view
only: system dialogs (the notification permission prompt, the photo picker) can
not be tapped with it, and `xcrun simctl privacy` can grant location and photos
but not notifications. Read a page's `println` output with `xcrun simctl launch
--console-pty`; a plain `--console` pipe buffers it. The CI job builds the
compiler from the checkout (a release compiler may predate the iOS target),
builds `mobile/host/ios/smoke.salam` and taps it with `-salam-script` on a
simulator.

## What works

- Layout, text (Persian included, system fonts), colors, scrolling with fling,
  media queries and `page.touch`.
- Touch: tap, long press, drag scroll. Long press or a double tap in a text
  input selects the word and shows two drag handles and a floating bar with
  Cut, Copy, Paste and Select all (drawn by the page, in Persian on a right to
  left input). Dragging a handle extends the selection, a tap on the selected
  text shows or hides the bar, and Paste uses the system clipboard. A password
  input has no Cut or Copy. In a headless script `selui` prints the selection
  state, `seltap copy|cut|paste|all` presses a bar button and `selhandle
  start|end X Y` drags a handle.
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

- Right to left: a page is right to left when its `lang` is Persian, Arabic,
  Hebrew, Urdu and the like, when its source is written in Persian, or when
  the page has `dir = "rtl"`. The default reaches headings, paragraphs and
  input placeholders alike. `dir` on any element (`ltr`, `rtl`, `auto`) or
  the `direction` property changes that element and its children; only
  `auto` looks at the text, taking the first strong letter. The same
  renderer draws this on Android and iOS as on desktop.
- `dialog` with `open` is a modal: centered, over a dimmed backdrop, with
  the page behind it blocked. The back button, Escape and a tap outside close it
  (`closedby = "none"` ignores the tap) and fire `close`, so bind the state:
  `dialog: open = shown close = (): shown = false end end`.
- The keyboard matches the input. `type` (`email`, `url`, `tel`, `number`,
  `password`, `search`), `inputmode` (`text`, `numeric`, `decimal`, `tel`,
  `email`, `url`, `search`, `none`), `enterkeyhint` (`enter`, `done`, `go`,
  `next`, `previous`, `search`, `send`), `autocapitalize`, `autocorrect`,
  `spellcheck` and `autocomplete` (`username`, `password`, `new-password`,
  `one-time-code`, `email`, `tel`, `name`) pick the Android `inputType` and
  `imeOptions` and the iOS keyboard type, return key, capitalization,
  correction, secure entry and content type. `inputmode = none` shows no
  keyboard. Enter in a single line input runs the hint: `next` and `previous`
  move the focus, `done` closes the keyboard, anything else submits the form.
  A textarea keeps Enter for a new line. `autocomplete` values only change the
  iOS content type; on Android a `password` one uses the password keyboard.
- The soft keyboard follows the focused input; the page root keeps away from
  the status bar, the navigation bar and the keyboard. By default the system
  insets are padding of the page, so they scroll away with the content and
  content slides under the status bar. `safe_area` (`ناحیه امن`) on the page
  changes that: `padding` is the default, `fixed` clips the scrolling content
  at the insets so it stops at the status bar, the navigation bar and the
  keyboard while the page background still fills them, and `none` ignores the
  insets, like `viewport-fit=cover` on the web.

  ```
  page:
      safe_area = "fixed"
  end
  ```

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
  `device.DataDir`, `device.CacheDir`, `device.DocumentsDir` and
  `device.TempDir` give the app folders (empty on a desktop). `device.SecureSet`, `device.SecureGet` and
  `device.SecureDelete` keep small secrets in the Android Keystore or the iOS
  Keychain.
  `device.Model`, `device.OSName`, `device.OSVersion`, `device.Language`,
  `device.Languages`, `device.TimeZone`, `device.ScreenDensity`,
  `device.AppVersion`, `device.BuildNumber`, `device.IsDarkMode`,
  `device.BatteryLevel` and `device.IsCharging` describe the phone and the app.
  `device.Network` says offline, Wi-Fi, cellular, ethernet or other, and a
  change of connection runs `device.OnChange` (the Android build declares
  `ACCESS_NETWORK_STATE` when you call it).

## Not available yet

- Rotate and three finger gestures.
- Bundled fonts (the phone's system fonts are used).
- `salam run` on the phone itself; build with the script above.

The plan and the design are in `design/MOBILE.md`.

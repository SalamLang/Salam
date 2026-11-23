package ir.salamlang.host

import android.app.Activity
import android.view.Surface

object Host {
    @Volatile
    var activity: Activity? = null

    init {
        System.loadLibrary("salamapp")
    }

    @JvmStatic external fun nativeStart(files: String, cache: String)

    @JvmStatic external fun nativeScript(script: String)

    @JvmStatic external fun nativeSurface(surface: Surface?, w: Int, h: Int, density: Float)

    @JvmStatic external fun nativePointer(action: Int, id: Int, x: Float, y: Float)

    @JvmStatic external fun nativeKey(down: Int, code: Int, meta: Int)

    @JvmStatic external fun nativeText(text: String)

    @JvmStatic external fun nativeCommit(text: String)

    @JvmStatic external fun nativePreedit(text: String, cursor: Int)

    @JvmStatic external fun nativeInsets(top: Float, right: Float, bottom: Float, left: Float, keyboard: Float)

    @JvmStatic external fun nativeLifecycle(kind: Int)

    @JvmStatic external fun nativeScheme(dark: Int)

    @JvmStatic external fun nativeDevice(kind: Int)

    @JvmStatic external fun nativePhoto(path: String)

    @JvmStatic external fun nativeLocation(ok: Int, lat: Double, lon: Double, acc: Double)

    @JvmStatic
    fun openUrl(url: String): Boolean {
        val a = activity ?: return false
        return try {
            val intent = android.content.Intent(android.content.Intent.ACTION_VIEW, android.net.Uri.parse(url))
            intent.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK)
            a.startActivity(intent)
            true
        } catch (e: Exception) {
            false
        }
    }

    @JvmStatic
    fun share(text: String, url: String): Boolean {
        val a = activity ?: return false
        val body = if (text.isEmpty()) url else if (url.isEmpty()) text else text + "\n" + url
        if (body.isEmpty()) return false
        return try {
            val send = android.content.Intent(android.content.Intent.ACTION_SEND)
            send.type = "text/plain"
            send.putExtra(android.content.Intent.EXTRA_TEXT, body)
            val chooser = android.content.Intent.createChooser(send, null)
            chooser.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK)
            a.startActivity(chooser)
            true
        } catch (e: Exception) {
            false
        }
    }

    @JvmStatic
    @Suppress("DEPRECATION")
    fun vibrate(ms: Int): Boolean {
        val a = activity ?: return false
        if (ms <= 0) return false
        return try {
            val v = a.getSystemService(android.content.Context.VIBRATOR_SERVICE) as? android.os.Vibrator ?: return false
            if (!v.hasVibrator()) return false
            if (android.os.Build.VERSION.SDK_INT >= 26) {
                v.vibrate(android.os.VibrationEffect.createOneShot(ms.toLong(), android.os.VibrationEffect.DEFAULT_AMPLITUDE))
            } else {
                v.vibrate(ms.toLong())
            }
            true
        } catch (e: Exception) {
            false
        }
    }

    @JvmStatic
    fun haptic(kind: Int): Boolean {
        val a = activity ?: return false
        val api = android.os.Build.VERSION.SDK_INT
        val code = when (kind) {
            0 -> android.view.HapticFeedbackConstants.VIRTUAL_KEY
            1 -> android.view.HapticFeedbackConstants.LONG_PRESS
            2 -> if (api >= 30) android.view.HapticFeedbackConstants.CONFIRM else android.view.HapticFeedbackConstants.VIRTUAL_KEY
            3 -> if (api >= 30) android.view.HapticFeedbackConstants.REJECT else android.view.HapticFeedbackConstants.LONG_PRESS
            else -> return false
        }
        a.runOnUiThread {
            a.window.decorView.performHapticFeedback(code)
        }
        return true
    }

    @JvmStatic
    @Suppress("DEPRECATION", "MissingPermission")
    fun locationRequest(): Boolean {
        val a = activity ?: return false
        if (android.os.Build.VERSION.SDK_INT >= 23 && !granted(a, androidPermissions("location"))) return false
        val lm = a.getSystemService(android.content.Context.LOCATION_SERVICE) as? android.location.LocationManager ?: return false
        val provider = when {
            lm.isProviderEnabled(android.location.LocationManager.GPS_PROVIDER) -> android.location.LocationManager.GPS_PROVIDER
            lm.isProviderEnabled(android.location.LocationManager.NETWORK_PROVIDER) -> android.location.LocationManager.NETWORK_PROVIDER
            else -> return false
        }
        val done = { l: android.location.Location? ->
            if (l == null) nativeLocation(0, 0.0, 0.0, 0.0)
            else nativeLocation(1, l.latitude, l.longitude, if (l.hasAccuracy()) l.accuracy.toDouble() else 0.0)
        }
        try {
            if (android.os.Build.VERSION.SDK_INT >= 30) {
                lm.getCurrentLocation(provider, null, a.mainExecutor) { l -> done(l) }
            } else {
                lm.requestSingleUpdate(provider, { l -> done(l) }, android.os.Looper.getMainLooper())
            }
        } catch (e: Exception) {
            return false
        }
        return true
    }

    private var photoFile: java.io.File? = null

    @JvmStatic
    fun photoRequest(camera: Int): Boolean {
        val a = activity ?: return false
        return try {
            val intent: android.content.Intent
            if (camera != 0) {
                val dir = java.io.File(a.cacheDir, "photos")
                dir.mkdirs()
                val f = java.io.File(dir, "capture_" + System.currentTimeMillis() + ".jpg")
                photoFile = f
                intent = android.content.Intent(android.provider.MediaStore.ACTION_IMAGE_CAPTURE)
                intent.putExtra(
                    android.provider.MediaStore.EXTRA_OUTPUT,
                    android.net.Uri.parse("content://" + a.packageName + ".salamfiles/photos/" + f.name)
                )
                intent.addFlags(android.content.Intent.FLAG_GRANT_WRITE_URI_PERMISSION or android.content.Intent.FLAG_GRANT_READ_URI_PERMISSION)
            } else {
                photoFile = null
                intent = if (android.os.Build.VERSION.SDK_INT >= 33) {
                    android.content.Intent(android.provider.MediaStore.ACTION_PICK_IMAGES)
                } else {
                    val i = android.content.Intent(android.content.Intent.ACTION_GET_CONTENT)
                    i.type = "image/*"
                    i.addCategory(android.content.Intent.CATEGORY_OPENABLE)
                    i
                }
            }
            a.runOnUiThread {
                try {
                    a.startActivityForResult(intent, 4243)
                } catch (e: Exception) {
                    nativePhoto("")
                }
            }
            true
        } catch (e: Exception) {
            false
        }
    }

    @JvmStatic
    fun photoResult(resultCode: Int, data: android.content.Intent?) {
        val a = activity
        val cap = photoFile
        photoFile = null
        if (a == null || resultCode != Activity.RESULT_OK) {
            if (cap != null) cap.delete()
            nativePhoto("")
            return
        }
        try {
            if (cap != null) {
                nativePhoto(if (cap.length() > 0) cap.absolutePath else "")
                return
            }
            val uri = data?.data
            if (uri == null) {
                nativePhoto("")
                return
            }
            val dir = java.io.File(a.cacheDir, "photos")
            dir.mkdirs()
            val out = java.io.File(dir, "pick_" + System.currentTimeMillis() + ".jpg")
            a.contentResolver.openInputStream(uri)?.use { input ->
                out.outputStream().use { o -> input.copyTo(o) }
            }
            nativePhoto(if (out.length() > 0) out.absolutePath else "")
        } catch (e: Exception) {
            nativePhoto("")
        }
    }

    private const val CHANNEL = "salam_default"

    private fun notificationAllowed(c: android.content.Context): Boolean {
        val nm = c.getSystemService(android.content.Context.NOTIFICATION_SERVICE) as android.app.NotificationManager
        if (!nm.areNotificationsEnabled()) return false
        if (android.os.Build.VERSION.SDK_INT >= 33) {
            return c.checkSelfPermission("android.permission.POST_NOTIFICATIONS") == android.content.pm.PackageManager.PERMISSION_GRANTED
        }
        return true
    }

    @JvmStatic
    fun postNotification(c: android.content.Context, id: Int, title: String, body: String): Boolean {
        if (!notificationAllowed(c)) return false
        val nm = c.getSystemService(android.content.Context.NOTIFICATION_SERVICE) as android.app.NotificationManager
        if (android.os.Build.VERSION.SDK_INT >= 26) {
            val label = c.applicationInfo.loadLabel(c.packageManager).toString()
            nm.createNotificationChannel(
                android.app.NotificationChannel(CHANNEL, label, android.app.NotificationManager.IMPORTANCE_DEFAULT)
            )
        }
        val launch = android.content.Intent(c, SalamActivity::class.java)
        launch.addFlags(android.content.Intent.FLAG_ACTIVITY_NEW_TASK or android.content.Intent.FLAG_ACTIVITY_SINGLE_TOP)
        var flags = android.app.PendingIntent.FLAG_UPDATE_CURRENT
        if (android.os.Build.VERSION.SDK_INT >= 23) flags = flags or android.app.PendingIntent.FLAG_IMMUTABLE
        val tap = android.app.PendingIntent.getActivity(c, id, launch, flags)
        val icon = if (c.applicationInfo.icon != 0) c.applicationInfo.icon else android.R.drawable.ic_dialog_info
        @Suppress("DEPRECATION")
        val b = if (android.os.Build.VERSION.SDK_INT >= 26) {
            android.app.Notification.Builder(c, CHANNEL)
        } else {
            android.app.Notification.Builder(c)
        }
        b.setSmallIcon(icon).setContentTitle(title).setContentText(body).setContentIntent(tap).setAutoCancel(true)
        nm.notify(id, b.build())
        return true
    }

    private fun alarmIntent(c: android.content.Context, id: Int, title: String, body: String): android.app.PendingIntent {
        val i = android.content.Intent(c, NotifyReceiver::class.java)
        i.putExtra("id", id)
        i.putExtra("title", title)
        i.putExtra("body", body)
        var flags = android.app.PendingIntent.FLAG_UPDATE_CURRENT
        if (android.os.Build.VERSION.SDK_INT >= 23) flags = flags or android.app.PendingIntent.FLAG_IMMUTABLE
        return android.app.PendingIntent.getBroadcast(c, id, i, flags)
    }

    @JvmStatic
    fun notify(id: Int, title: String, body: String, delayMs: Int): Boolean {
        val a = activity ?: return false
        if (delayMs <= 0) return postNotification(a, id, title, body)
        if (!notificationAllowed(a)) return false
        val am = a.getSystemService(android.content.Context.ALARM_SERVICE) as android.app.AlarmManager
        val at = System.currentTimeMillis() + delayMs
        val pi = alarmIntent(a, id, title, body)
        if (android.os.Build.VERSION.SDK_INT >= 23) {
            am.setAndAllowWhileIdle(android.app.AlarmManager.RTC_WAKEUP, at, pi)
        } else {
            am.set(android.app.AlarmManager.RTC_WAKEUP, at, pi)
        }
        return true
    }

    @JvmStatic
    fun cancelNotification(id: Int): Boolean {
        val a = activity ?: return false
        val nm = a.getSystemService(android.content.Context.NOTIFICATION_SERVICE) as android.app.NotificationManager
        nm.cancel(id)
        val am = a.getSystemService(android.content.Context.ALARM_SERVICE) as android.app.AlarmManager
        am.cancel(alarmIntent(a, id, "", ""))
        return true
    }

    private fun androidPermissions(name: String): List<String> =
        when (name) {
            "camera" -> listOf("android.permission.CAMERA")
            "microphone" -> listOf("android.permission.RECORD_AUDIO")
            "location" -> listOf("android.permission.ACCESS_FINE_LOCATION", "android.permission.ACCESS_COARSE_LOCATION")
            "notifications" -> if (android.os.Build.VERSION.SDK_INT >= 33) listOf("android.permission.POST_NOTIFICATIONS") else emptyList()
            else -> emptyList()
        }

    private fun asked(a: Activity): android.content.SharedPreferences = a.getSharedPreferences("salam_permissions", 0)

    private fun granted(a: Activity, perms: List<String>): Boolean =
        perms.any { a.checkSelfPermission(it) == android.content.pm.PackageManager.PERMISSION_GRANTED }

    @Volatile
    private var pending = ""

    @JvmStatic
    fun permResult() {
        val a = activity
        if (a != null && pending.isNotEmpty()) {
            asked(a).edit().putBoolean(pending, true).apply()
        }
        pending = ""
        nativeDevice(1)
    }

    @JvmStatic
    fun permState(name: String): Int {
        val a = activity ?: return 0
        if (name == "notifications" && android.os.Build.VERSION.SDK_INT < 33) {
            val nm = a.getSystemService(android.content.Context.NOTIFICATION_SERVICE) as android.app.NotificationManager
            return if (nm.areNotificationsEnabled()) 1 else 2
        }
        val perms = androidPermissions(name)
        if (perms.isEmpty()) return 2
        if (android.os.Build.VERSION.SDK_INT < 23) return 1
        if (granted(a, perms)) return 1
        return if (asked(a).getBoolean(name, false)) 2 else 0
    }

    @JvmStatic
    fun permRequest(name: String): Boolean {
        val a = activity ?: return false
        val perms = androidPermissions(name)
        if (perms.isEmpty() || android.os.Build.VERSION.SDK_INT < 23) {
            nativeDevice(1)
            return perms.isNotEmpty()
        }
        if (granted(a, perms)) {
            nativeDevice(1)
            return true
        }
        pending = name
        a.runOnUiThread { a.requestPermissions(perms.toTypedArray(), 4242) }
        return true
    }

    private fun clipboard(): android.content.ClipboardManager? =
        activity?.getSystemService(android.content.Context.CLIPBOARD_SERVICE) as? android.content.ClipboardManager

    @JvmStatic
    fun clipSet(text: String): Boolean {
        val c = clipboard() ?: return false
        c.setPrimaryClip(android.content.ClipData.newPlainText("salam", text))
        return true
    }

    @JvmStatic
    fun clipGet(): String {
        val c = clipboard() ?: return ""
        val clip = c.primaryClip ?: return ""
        if (clip.itemCount == 0) return ""
        return clip.getItemAt(0).coerceToText(activity).toString()
    }

    @JvmStatic
    fun showIme(show: Boolean) {
        val a = activity ?: return
        a.runOnUiThread {
            val v = a.window.decorView.findViewById<android.view.View>(android.R.id.content) ?: return@runOnUiThread
            val target = (v as? android.view.ViewGroup)?.getChildAt(0) ?: v
            val imm = a.getSystemService(android.content.Context.INPUT_METHOD_SERVICE) as android.view.inputmethod.InputMethodManager
            if (show) {
                target.requestFocus()
                imm.showSoftInput(target, 0)
            } else {
                imm.hideSoftInputFromWindow(target.windowToken, 0)
            }
        }
    }

    @JvmStatic
    fun barsLight(top: Int, bottom: Int) {
        val a = activity as? SalamActivity ?: return
        a.runOnUiThread { a.setBarContrast(top != 0, bottom != 0) }
    }

    @JvmStatic
    fun onExit() {
        val a = activity ?: return
        a.runOnUiThread {
            a.finishAndRemoveTask()
            android.os.Handler(android.os.Looper.getMainLooper()).postDelayed({
                android.os.Process.killProcess(android.os.Process.myPid())
            }, 200)
        }
    }
}

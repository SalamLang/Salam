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

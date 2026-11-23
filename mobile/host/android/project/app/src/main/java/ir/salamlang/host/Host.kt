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

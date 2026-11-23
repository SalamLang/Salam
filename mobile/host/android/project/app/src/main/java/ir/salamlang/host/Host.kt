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
    fun onExit() {
        val a = activity ?: return
        a.runOnUiThread { a.finishAndRemoveTask() }
    }
}

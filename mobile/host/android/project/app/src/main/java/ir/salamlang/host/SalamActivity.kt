package ir.salamlang.host

import android.app.Activity
import android.content.ComponentCallbacks2
import android.content.res.Configuration
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.view.KeyEvent
import android.view.SurfaceHolder
import android.view.View
import android.view.WindowInsets
import android.view.WindowManager

class SalamActivity : Activity(), SurfaceHolder.Callback {
    private lateinit var view: SalamView
    private var density = 1f

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Host.activity = this
        if (savedInstanceState == null) {
            java.io.File(filesDir, ".salam_state").delete()
        }
        density = resources.displayMetrics.density
        window.statusBarColor = Color.TRANSPARENT
        window.navigationBarColor = Color.TRANSPARENT
        if (Build.VERSION.SDK_INT >= 28) {
            val lp = window.attributes
            lp.layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
            window.attributes = lp
        }
        view =SalamView(this)
        view.holder.addCallback(this)
        view.setOnApplyWindowInsetsListener { _, insets ->
            sendInsets(insets)
            insets
        }
        setContentView(view)
        applyBars(isDark(resources.configuration))
        Host.nativeScheme(if (isDark(resources.configuration)) 1 else 0)
    }

    private var lightTop = true
    private var lightBottom = true

    @Suppress("DEPRECATION")
    private fun applyBars(dark: Boolean) {
        lightTop = !dark
        lightBottom = !dark
        pushBars()
    }

    fun setBarContrast(top: Boolean, bottom: Boolean) {
        if (top == lightTop && bottom == lightBottom) return
        lightTop = top
        lightBottom = bottom
        pushBars()
    }

    @Suppress("DEPRECATION")
    private fun pushBars() {
        var flags =
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
        if (lightTop && Build.VERSION.SDK_INT >= 23) flags = flags or View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
        if (lightBottom && Build.VERSION.SDK_INT >= 26) flags = flags or View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR
        window.decorView.systemUiVisibility = flags
    }

    private fun isDark(c: Configuration): Boolean =
        (c.uiMode and Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES

    @Suppress("DEPRECATION")
    private fun sendInsets(insets: WindowInsets) {
        var top: Int
        var right: Int
        var bottom: Int
        var left: Int
        var keyboard = 0
        if (Build.VERSION.SDK_INT >= 30) {
            val bars = insets.getInsets(WindowInsets.Type.systemBars() or WindowInsets.Type.displayCutout())
            top = bars.top
            right = bars.right
            bottom = bars.bottom
            left = bars.left
            keyboard = insets.getInsets(WindowInsets.Type.ime()).bottom
        } else {
            top = insets.systemWindowInsetTop
            right = insets.systemWindowInsetRight
            left = insets.systemWindowInsetLeft
            bottom = insets.stableInsetBottom
            keyboard = maxOf(0, insets.systemWindowInsetBottom - insets.stableInsetBottom)
        }
        Host.nativeInsets(top.toFloat(), right.toFloat(), bottom.toFloat(), left.toFloat(), keyboard.toFloat())
    }

    override fun surfaceCreated(holder: SurfaceHolder) {}

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
        Host.nativeSurface(holder.surface, width, height, density)
        Fonts.install(this)
        Assets.install(this)
        Host.nativeStart(filesDir.absolutePath, cacheDir.absolutePath)
    }

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        Host.nativeSurface(null, 0, 0, density)
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        applyBars(isDark(newConfig))
        Host.nativeScheme(if (isDark(newConfig)) 1 else 0)
    }

    override fun onKeyDown(keyCode: Int, event: KeyEvent): Boolean {
        Host.nativeKey(1, keyCode, event.metaState)
        val ch = event.unicodeChar
        if (ch > 0 && !event.isCtrlPressed) {
            Host.nativeText(String(Character.toChars(ch)))
        }
        return true
    }

    override fun onKeyUp(keyCode: Int, event: KeyEvent): Boolean {
        Host.nativeKey(0, keyCode, event.metaState)
        return true
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: android.content.Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 4243) {
            Thread { Host.photoResult(resultCode, data) }.start()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        Host.permResult()
    }

    override fun onPause() {
        super.onPause()
        Host.nativeLifecycle(0)
    }

    override fun onResume() {
        super.onResume()
        Host.nativeLifecycle(1)
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        Host.nativeLifecycle(2)
    }

    @Suppress("DEPRECATION")
    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
        if (level >= ComponentCallbacks2.TRIM_MEMORY_RUNNING_LOW) {
            Host.nativeLifecycle(3)
        }
    }
}

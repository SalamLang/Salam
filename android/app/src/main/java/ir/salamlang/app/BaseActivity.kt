package ir.salamlang.app

import android.app.Activity
import android.content.Context
import android.content.res.ColorStateList
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.graphics.drawable.RippleDrawable
import android.os.Build
import android.os.Bundle
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.ImageButton
import android.widget.LinearLayout
import android.widget.TextView

abstract class BaseActivity : Activity() {
    lateinit var palette: Palette
    private var appliedDark = false
    private var appliedLang = ""

    override fun attachBaseContext(newBase: Context) {
        super.attachBaseContext(Prefs.wrapLocale(newBase))
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        appliedDark = Prefs.isDark(this)
        appliedLang = Prefs.effectiveUiLang(this)
        setTheme(if (appliedDark) R.style.Theme_Salam_Dark else R.style.Theme_Salam)
        palette = Palette(appliedDark)
        super.onCreate(savedInstanceState)
        window.statusBarColor = palette.surface
        window.navigationBarColor = palette.surface
        if (Build.VERSION.SDK_INT >= 23 && !appliedDark) {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR
        }
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS)
    }

    override fun onResume() {
        super.onResume()
        if (Prefs.isDark(this) != appliedDark || Prefs.effectiveUiLang(this) != appliedLang) {
            recreate()
        }
    }

    fun dp(v: Float): Int = TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, v, resources.displayMetrics).toInt()

    fun dp(v: Int): Int = dp(v.toFloat())

    fun rounded(
        color: Int,
        radius: Float,
        stroke: Int = 0,
    ): GradientDrawable =
        GradientDrawable().apply {
            setColor(color)
            cornerRadius = dp(radius).toFloat()
            if (stroke != 0) {
                setStroke(dp(1), stroke)
            }
        }

    fun ripple(
        color: Int,
        radius: Float,
        stroke: Int = 0,
    ): RippleDrawable = RippleDrawable(ColorStateList.valueOf(palette.brand and 0x40FFFFFF), rounded(color, radius, stroke), null)

    fun label(
        text: CharSequence,
        size: Float = 15f,
        color: Int = palette.text,
        bold: Boolean = false,
    ): TextView =
        TextView(this).apply {
            this.text = text
            setTextSize(TypedValue.COMPLEX_UNIT_SP, size)
            setTextColor(color)
            if (bold) {
                typeface = Typeface.DEFAULT_BOLD
            }
        }

    fun iconButton(
        icon: Int,
        description: Int,
        tint: Int = palette.text,
        onClick: (View) -> Unit,
    ): ImageButton =
        ImageButton(this).apply {
            setImageResource(icon)
            imageTintList = ColorStateList.valueOf(tint)
            contentDescription = getString(description)
            background = ripple(0, 20f)
            val p = dp(10)
            setPadding(p, p, p, p)
            layoutParams = LinearLayout.LayoutParams(dp(44), dp(44))
            setOnClickListener(onClick)
            setOnLongClickListener {
                android.widget.Toast.makeText(context, contentDescription, android.widget.Toast.LENGTH_SHORT).show()
                true
            }
        }

    fun pillButton(
        text: CharSequence,
        primary: Boolean,
        onClick: (View) -> Unit,
    ): TextView =
        label(text, 14f, if (primary) 0xFFFFFFFF.toInt() else palette.text, true).apply {
            gravity = Gravity.CENTER
            background = ripple(if (primary) palette.brand else palette.surfaceHigh, 20f)
            setPadding(dp(16), dp(9), dp(16), dp(9))
            setOnClickListener(onClick)
        }

    fun topBar(): LinearLayout =
        LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            gravity = Gravity.CENTER_VERTICAL
            setBackgroundColor(palette.surface)
            setPadding(dp(4), dp(4), dp(4), dp(4))
            layoutParams = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        }

    fun weight(
        v: View,
        w: Float = 1f,
    ): View {
        v.layoutParams = LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, w)
        return v
    }
}

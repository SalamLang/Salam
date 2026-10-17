package ir.salamlang.app

import android.content.Context
import android.content.SharedPreferences
import android.content.res.Configuration
import java.util.Locale

object Prefs {
    const val THEME_SYSTEM = 0
    const val THEME_LIGHT = 1
    const val THEME_DARK = 2

    private fun sp(ctx: Context): SharedPreferences = ctx.getSharedPreferences("salam", Context.MODE_PRIVATE)

    fun theme(ctx: Context): Int = sp(ctx).getInt("theme", THEME_SYSTEM)

    fun setTheme(
        ctx: Context,
        v: Int,
    ) = sp(ctx).edit().putInt("theme", v).apply()

    fun uiLang(ctx: Context): String = sp(ctx).getString("ui_lang", "") ?: ""

    fun setUiLang(
        ctx: Context,
        v: String,
    ) = sp(ctx).edit().putString("ui_lang", v).apply()

    fun fontSize(ctx: Context): Float = sp(ctx).getFloat("font_size", 15f)

    fun setFontSize(
        ctx: Context,
        v: Float,
    ) = sp(ctx).edit().putFloat("font_size", v).apply()

    fun tabWidth(ctx: Context): Int = sp(ctx).getInt("tab_width", 4)

    fun setTabWidth(
        ctx: Context,
        v: Int,
    ) = sp(ctx).edit().putInt("tab_width", v).apply()

    fun wrap(ctx: Context): Boolean = sp(ctx).getBoolean("wrap", true)

    fun setWrap(
        ctx: Context,
        v: Boolean,
    ) = sp(ctx).edit().putBoolean("wrap", v).apply()

    fun lineNumbers(ctx: Context): Boolean = sp(ctx).getBoolean("line_numbers", true)

    fun setLineNumbers(
        ctx: Context,
        v: Boolean,
    ) = sp(ctx).edit().putBoolean("line_numbers", v).apply()

    fun liveCheck(ctx: Context): Boolean = sp(ctx).getBoolean("live_check", true)

    fun setLiveCheck(
        ctx: Context,
        v: Boolean,
    ) = sp(ctx).edit().putBoolean("live_check", v).apply()

    fun newFileLang(ctx: Context): String = sp(ctx).getString("new_lang", "") ?: ""

    fun setNewFileLang(
        ctx: Context,
        v: String,
    ) = sp(ctx).edit().putString("new_lang", v).apply()

    fun isDark(ctx: Context): Boolean =
        when (theme(ctx)) {
            THEME_LIGHT -> false
            THEME_DARK -> true
            else -> (ctx.resources.configuration.uiMode and Configuration.UI_MODE_NIGHT_MASK) == Configuration.UI_MODE_NIGHT_YES
        }

    fun effectiveUiLang(ctx: Context): String {
        val chosen = uiLang(ctx)
        if (chosen.isNotEmpty()) {
            return chosen
        }
        val sys = Locale.getDefault().language
        return if (sys == "en") "en" else "fa"
    }

    @Suppress("DEPRECATION")
    fun wrapLocale(base: Context): Context {
        val lang = effectiveUiLang(base)
        val locale = Locale(lang)
        Locale.setDefault(locale)
        val config = Configuration(base.resources.configuration)
        config.setLocale(locale)
        config.setLayoutDirection(locale)
        return base.createConfigurationContext(config)
    }
}

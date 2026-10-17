package ir.salamlang.app

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.CompoundButton
import android.widget.LinearLayout
import android.widget.RadioButton
import android.widget.RadioGroup
import android.widget.ScrollView
import android.widget.SeekBar
import android.widget.Switch
import android.widget.TextView

class SettingsActivity : BaseActivity() {
    private lateinit var body: LinearLayout

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val root =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.background)
            }
        val bar = topBar()
        bar.addView(iconButton(R.drawable.ic_back, R.string.back) { finish() })
        bar.addView(label(getString(R.string.settings), 17f, palette.text, true))
        root.addView(bar)
        body =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(16), dp(8), dp(16), dp(24))
            }
        root.addView(ScrollView(this).apply { addView(body) }, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)

        section(R.string.appearance)
        choice(
            R.string.theme,
            listOf(R.string.theme_system, R.string.theme_light, R.string.theme_dark),
            Prefs.theme(this),
        ) {
            Prefs.setTheme(this, it)
            recreate()
        }
        val langs = listOf("fa", "en")
        choice(
            R.string.ui_language,
            listOf(R.string.lang_fa, R.string.lang_en),
            langs.indexOf(Prefs.effectiveUiLang(this)).coerceAtLeast(0),
        ) {
            Prefs.setUiLang(this, langs[it])
            recreate()
        }

        section(R.string.editor)
        slider()
        choice(R.string.tab_width, listOf(R.string.two_spaces, R.string.four_spaces), if (Prefs.tabWidth(this) == 2) 0 else 1) {
            Prefs.setTabWidth(this, if (it == 0) 2 else 4)
        }
        toggle(R.string.word_wrap, Prefs.wrap(this)) { Prefs.setWrap(this, it) }
        toggle(R.string.line_numbers, Prefs.lineNumbers(this)) { Prefs.setLineNumbers(this, it) }
        toggle(R.string.live_check, Prefs.liveCheck(this)) { Prefs.setLiveCheck(this, it) }

        section(R.string.about)
        val engine = label("", 14f, palette.textDim)
        body.addView(engine)
        Analyzer.version(this) { engine.text = getString(R.string.engine_version, it.trim()) }
        body.addView(label(getString(R.string.about_text), 14f, palette.textDim).apply { setPadding(0, dp(6), 0, dp(10)) })
        link(R.string.website, "https://salamlang.ir")
        link(R.string.source_code, "https://github.com/SalamLang/Salam")
    }

    private fun section(title: Int) {
        body.addView(
            label(getString(title), 13f, palette.brand, true).apply {
                setPadding(dp(4), dp(18), dp(4), dp(8))
            },
        )
    }

    private fun card(): LinearLayout {
        val c =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                background = rounded(palette.surface, 14f)
                setPadding(dp(14), dp(10), dp(14), dp(10))
            }
        val lp = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        lp.bottomMargin = dp(8)
        body.addView(c, lp)
        return c
    }

    private fun choice(
        title: Int,
        options: List<Int>,
        selected: Int,
        onPick: (Int) -> Unit,
    ) {
        val c = card()
        c.addView(label(getString(title), 15f, palette.text, true))
        val group = RadioGroup(this).apply { orientation = RadioGroup.VERTICAL }
        val ids = ArrayList<Int>()
        options.forEachIndexed { i, res ->
            val b =
                RadioButton(this).apply {
                    id = View.generateViewId()
                    text = getString(res)
                    setTextColor(palette.text)
                    buttonTintList = android.content.res.ColorStateList.valueOf(palette.brand)
                }
            ids.add(b.id)
            group.addView(b)
            if (i == selected) {
                b.isChecked = true
            }
        }
        group.setOnCheckedChangeListener { _, id ->
            val i = ids.indexOf(id)
            if (i >= 0 && i != selected) {
                onPick(i)
            }
        }
        c.addView(group)
    }

    private fun toggle(
        title: Int,
        value: Boolean,
        onChange: (Boolean) -> Unit,
    ) {
        val c = card()
        val s =
            Switch(this).apply {
                text = getString(title)
                setTextColor(palette.text)
                textSize = 15f
                isChecked = value
                setOnCheckedChangeListener { _: CompoundButton, v: Boolean -> onChange(v) }
            }
        c.addView(s)
    }

    private fun slider() {
        val c = card()
        val size = Prefs.fontSize(this)
        val title = label(getString(R.string.font_size, size.toInt()), 15f, palette.text, true)
        c.addView(title)
        val sample =
            label("func main:  // سلام", size, palette.textDim).apply {
                typeface = android.graphics.Typeface.MONOSPACE
                setPadding(0, dp(6), 0, dp(6))
            }
        c.addView(sample)
        val bar =
            SeekBar(this).apply {
                max = 22
                progress = (size - 10f).toInt().coerceIn(0, 22)
                setOnSeekBarChangeListener(
                    object : SeekBar.OnSeekBarChangeListener {
                        override fun onProgressChanged(
                            s: SeekBar?,
                            p: Int,
                            fromUser: Boolean,
                        ) {
                            val v = 10f + p
                            title.text = getString(R.string.font_size, v.toInt())
                            sample.textSize = v
                            Prefs.setFontSize(this@SettingsActivity, v)
                        }

                        override fun onStartTrackingTouch(s: SeekBar?) {}

                        override fun onStopTrackingTouch(s: SeekBar?) {}
                    },
                )
            }
        c.addView(bar)
    }

    private fun link(
        title: Int,
        url: String,
    ) {
        val t: TextView =
            pillButton(getString(title), false) {
                try {
                    startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)))
                } catch (_: Exception) {
                }
            }
        t.gravity = Gravity.CENTER
        val lp = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT)
        lp.bottomMargin = dp(8)
        body.addView(t, lp)
    }
}

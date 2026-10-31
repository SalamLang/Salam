package ir.salamlang.app

import android.content.ClipData
import android.content.ClipboardManager
import android.graphics.Typeface
import android.os.Bundle
import android.util.TypedValue
import android.view.View
import android.view.ViewGroup
import android.widget.HorizontalScrollView
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView
import android.widget.Toast
import java.io.File

class ViewerActivity : BaseActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val f = File(intent.getStringExtra(EditorActivity.EXTRA_PATH) ?: "")
        val raw =
            try {
                f.readText()
            } catch (_: Exception) {
                ""
            }
        val text = if (raw.length > LIMIT) raw.substring(0, LIMIT) + "\n…" else raw
        val root =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.background)
            }
        val bar = topBar()
        bar.addView(iconButton(R.drawable.ic_back, R.string.back) { finish() })
        bar.addView(weight(label(intent.getStringExtra(EXTRA_TITLE) ?: f.name, 16f, palette.text, true)))
        bar.addView(
            iconButton(R.drawable.ic_save, R.string.copy) {
                val cm = getSystemService(CLIPBOARD_SERVICE) as ClipboardManager
                cm.setPrimaryClip(ClipData.newPlainText(f.name, raw))
                Toast.makeText(this, R.string.copied, Toast.LENGTH_SHORT).show()
            },
        )
        root.addView(bar)
        val body =
            TextView(this).apply {
                this.text = text
                typeface = Typeface.MONOSPACE
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 12.5f)
                setTextColor(palette.text)
                setTextIsSelectable(true)
                textDirection = View.TEXT_DIRECTION_LTR
                setPadding(dp(12), dp(10), dp(12), dp(24))
            }
        val h = HorizontalScrollView(this).apply { addView(body) }
        val v = ScrollView(this).apply { addView(h) }
        root.addView(v, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)
    }

    companion object {
        const val EXTRA_TITLE = "title"
        private const val LIMIT = 400_000
    }
}

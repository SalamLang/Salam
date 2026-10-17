package ir.salamlang.app

import android.annotation.SuppressLint
import android.graphics.Typeface
import android.util.TypedValue
import android.view.Gravity
import android.widget.HorizontalScrollView
import android.widget.LinearLayout
import android.widget.TextView

@SuppressLint("ViewConstructor")
class SymbolBar(
    private val act: BaseActivity,
    private val editor: CodeEditor,
) : HorizontalScrollView(act) {
    private val row = LinearLayout(act)
    private var lang = ""

    init {
        isHorizontalScrollBarEnabled = false
        setBackgroundColor(act.palette.surface)
        row.orientation = LinearLayout.HORIZONTAL
        row.setPadding(act.dp(4), act.dp(4), act.dp(4), act.dp(4))
        addView(row)
        setLang("en")
    }

    fun setLang(code: String) {
        if (code == lang) {
            return
        }
        lang = code
        row.removeAllViews()
        val end = if (code == "fa") "پایان" else "end"
        val quote = if (code == "fa") "«»" else "\"\""
        key("⇥") { editor.indentSelection(false) }
        key("⇤") { editor.indentSelection(true) }
        key(":") { editor.insert(":") }
        key(end) { editor.insert(end) }
        key(":=") { editor.insert(" := ") }
        key("( )") { editor.insert("()", 1) }
        key("[ ]") { editor.insert("[]", 1) }
        key(quote) { editor.insert(quote, 1) }
        for (s in listOf(".", ",", "=", "+", "-", "*", "/", "%", "<", ">", "!", "&", "|", "_", "{", "}")) {
            key(s) { editor.insert(s) }
        }
        key("//") { editor.insert("// ") }
        key("↶") { editor.undo() }
        key("↷") { editor.redo() }
    }

    private fun key(
        text: String,
        action: () -> Unit,
    ) {
        val v =
            TextView(act).apply {
                this.text = text
                gravity = Gravity.CENTER
                typeface = Typeface.MONOSPACE
                textDirection = android.view.View.TEXT_DIRECTION_LTR
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 15f)
                setTextColor(act.palette.text)
                minWidth = act.dp(40)
                background = act.ripple(act.palette.surfaceHigh, 8f)
                setPadding(act.dp(10), act.dp(8), act.dp(10), act.dp(8))
                setOnClickListener {
                    action()
                    editor.requestFocus()
                }
            }
        val lp = LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT)
        lp.marginEnd = act.dp(4)
        row.addView(v, lp)
    }
}

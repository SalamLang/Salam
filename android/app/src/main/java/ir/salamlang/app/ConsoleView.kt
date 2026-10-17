package ir.salamlang.app

import android.annotation.SuppressLint
import android.graphics.Typeface
import android.text.SpannableStringBuilder
import android.text.Spanned
import android.text.style.ForegroundColorSpan
import android.util.TypedValue
import android.view.Gravity
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.EditorInfo
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TextView

@SuppressLint("ViewConstructor")
class ConsoleView(
    private val act: BaseActivity,
) : LinearLayout(act) {
    interface Listener {
        fun onInput(line: String)

        fun onStop()

        fun onClose()

        fun onResize(dy: Float)
    }

    var listener: Listener? = null
    private val p = act.palette
    private val buffer = SpannableStringBuilder()
    private val output: TextView
    private val scroll: ScrollView
    private val status: TextView
    private val stopButton: View
    private val inputRow: LinearLayout
    private val input: EditText
    private var pendingFlush = false

    init {
        orientation = VERTICAL
        setBackgroundColor(p.surface)

        val handle =
            View(act).apply {
                background = act.rounded(p.border, 3f)
            }
        val handleBox =
            LinearLayout(act).apply {
                gravity = Gravity.CENTER
                setPadding(0, act.dp(6), 0, act.dp(2))
                addView(handle, LayoutParams(act.dp(40), act.dp(4)))
            }
        var lastY = 0f
        handleBox.setOnTouchListener { _, e ->
            when (e.actionMasked) {
                MotionEvent.ACTION_DOWN -> lastY = e.rawY
                MotionEvent.ACTION_MOVE -> {
                    listener?.onResize(lastY - e.rawY)
                    lastY = e.rawY
                }
            }
            true
        }
        addView(handleBox, LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        val header =
            LinearLayout(act).apply {
                orientation = HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                setPadding(act.dp(12), 0, act.dp(4), 0)
            }
        header.addView(act.label(act.getString(R.string.console), 14f, p.text, true))
        status = act.label("", 13f, p.textDim).apply { setPadding(act.dp(10), 0, 0, 0) }
        header.addView(act.weight(status))
        stopButton =
            act.iconButton(R.drawable.ic_stop, R.string.stop, p.error) { listener?.onStop() }.apply {
                visibility = GONE
            }
        header.addView(stopButton)
        header.addView(act.iconButton(R.drawable.ic_close, R.string.close, p.textDim) { listener?.onClose() })
        addView(header)

        output =
            TextView(act).apply {
                typeface = Typeface.MONOSPACE
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 13f)
                setTextColor(p.text)
                setTextIsSelectable(true)
                textDirection = View.TEXT_DIRECTION_FIRST_STRONG
                setPadding(act.dp(12), act.dp(4), act.dp(12), act.dp(8))
            }
        scroll =
            ScrollView(act).apply {
                isFillViewport = true
                setBackgroundColor(p.background)
                addView(output)
            }
        addView(scroll, LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))

        inputRow =
            LinearLayout(act).apply {
                orientation = HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                setPadding(act.dp(8), act.dp(4), act.dp(4), act.dp(4))
                visibility = GONE
            }
        input =
            EditText(act).apply {
                hint = act.getString(R.string.input_hint)
                setSingleLine(true)
                typeface = Typeface.MONOSPACE
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
                setTextColor(p.text)
                setHintTextColor(p.textDim)
                background = act.rounded(p.background, 18f, p.border)
                setPadding(act.dp(14), act.dp(8), act.dp(14), act.dp(8))
                imeOptions = EditorInfo.IME_ACTION_SEND or EditorInfo.IME_FLAG_NO_EXTRACT_UI
                setOnEditorActionListener { _, action, event ->
                    if (action == EditorInfo.IME_ACTION_SEND || event?.keyCode == KeyEvent.KEYCODE_ENTER) {
                        send()
                        true
                    } else {
                        false
                    }
                }
            }
        inputRow.addView(act.weight(input))
        inputRow.addView(act.iconButton(R.drawable.ic_send, R.string.send, p.brand) { send() })
        addView(inputRow)
    }

    private fun send() {
        val line = input.text.toString()
        input.setText("")
        append(line + "\n", p.textDim)
        listener?.onInput(line + "\n")
    }

    fun clear() {
        buffer.clear()
        output.text = ""
    }

    fun setRunning(running: Boolean) {
        stopButton.visibility = if (running) VISIBLE else GONE
        inputRow.visibility = if (running) VISIBLE else GONE
        if (running) {
            status.text = act.getString(R.string.running)
            status.setTextColor(p.textDim)
        }
    }

    fun focusInput() {
        input.requestFocus()
    }

    fun setStatus(
        text: String,
        color: Int,
    ) {
        status.text = text
        status.setTextColor(color)
    }

    fun append(
        text: String,
        color: Int = p.text,
    ) {
        if (text.isEmpty()) {
            return
        }
        val start = buffer.length
        buffer.append(text)
        if (color != p.text) {
            buffer.setSpan(ForegroundColorSpan(color), start, buffer.length, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE)
        }
        if (buffer.length > LIMIT) {
            buffer.delete(0, buffer.length - LIMIT / 2)
        }
        if (!pendingFlush) {
            pendingFlush = true
            postDelayed({
                pendingFlush = false
                output.text = buffer
                scroll.post { scroll.fullScroll(View.FOCUS_DOWN) }
            }, 50)
        }
    }

    companion object {
        private const val LIMIT = 200_000
    }
}

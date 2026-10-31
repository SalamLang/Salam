package ir.salamlang.app

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Paint
import android.graphics.Path
import android.graphics.Typeface
import android.os.SystemClock
import android.text.Editable
import android.text.InputType
import android.text.Spanned
import android.text.TextWatcher
import android.text.style.ForegroundColorSpan
import android.util.TypedValue
import android.view.Gravity
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View
import android.view.inputmethod.EditorInfo
import android.widget.EditText
import kotlin.math.max
import kotlin.math.min

@SuppressLint("ViewConstructor")
class CodeEditor(
    ctx: Context,
    var palette: Palette,
) : EditText(ctx) {
    interface Listener {
        fun onEdited()

        fun onZoom(size: Float)
    }

    private class HlSpan(
        color: Int,
    ) : ForegroundColorSpan(color)

    private class Edit(
        var start: Int,
        var removed: String,
        var inserted: String,
        var time: Long,
    )

    var listener: Listener? = null
    var tabWidth = 4
    var lang = "en"
        set(v) {
            field = v
            applyWrap()
        }
    var wrap = true
        set(v) {
            field = v
            applyWrap()
        }
    var showLineNumbers = true
        set(v) {
            field = v
            updateGutter()
        }
    var diagnostics: List<Diagnostic> = emptyList()
        set(v) {
            field = v
            invalidate()
        }

    var version = 0
        private set

    private var tokens = IntArray(0)
    private var tokensVersion = -1
    private var hlFrom = 0
    private var hlTo = 0
    private var hlVersion = -1

    private val undoStack = ArrayList<Edit>()
    private val redoStack = ArrayList<Edit>()
    private var applying = false
    private var autoEditing = false
    private var removedText = ""
    private var pendingNewline = -1
    private var pendingWordEnd = -1

    private var newlineVersion = -1
    private var newlines = IntArray(0)
    private var newlineCount = 0

    private val gutterPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val numberPaint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val linePaint = Paint()
    private val squigglePaint =
        Paint(Paint.ANTI_ALIAS_FLAG).apply {
            style = Paint.Style.STROKE
        }
    private val squiggle = Path()
    private var gutterWidth = 0
    private var gutterDigits = 0

    private val scaler =
        ScaleGestureDetector(
            ctx,
            object : ScaleGestureDetector.SimpleOnScaleGestureListener() {
                override fun onScale(d: ScaleGestureDetector): Boolean {
                    val sp = textSize / resources.displayMetrics.scaledDensity
                    val next = (sp * d.scaleFactor).coerceIn(9f, 34f)
                    setTextSize(TypedValue.COMPLEX_UNIT_SP, next)
                    updateGutter()
                    listener?.onZoom(next)
                    return true
                }
            },
        )

    init {
        typeface = Typeface.MONOSPACE
        background = null
        gravity = Gravity.TOP or Gravity.START
        inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
        imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI or EditorInfo.IME_FLAG_NO_FULLSCREEN
        textDirection = View.TEXT_DIRECTION_FIRST_STRONG
        isVerticalScrollBarEnabled = true
        setLineSpacing(0f, 1.15f)
        includeFontPadding = false
        squigglePaint.strokeWidth = resources.displayMetrics.density * 1.2f
        applyPalette(palette)
        addTextChangedListener(
            object : TextWatcher {
                override fun beforeTextChanged(
                    s: CharSequence,
                    start: Int,
                    count: Int,
                    after: Int,
                ) {
                    removedText = if (applying || count == 0) "" else s.subSequence(start, start + count).toString()
                }

                override fun onTextChanged(
                    s: CharSequence,
                    start: Int,
                    before: Int,
                    count: Int,
                ) {
                    if (applying) {
                        return
                    }
                    val inserted = if (count == 0) "" else s.subSequence(start, start + count).toString()
                    record(start, removedText, inserted)
                    pendingNewline = -1
                    pendingWordEnd = -1
                    if (!autoEditing && before == 0 && count == 1) {
                        if (s[start] == '\n') {
                            pendingNewline = start
                        } else if (Character.isLetter(s[start])) {
                            pendingWordEnd = start + 1
                        }
                    }
                }

                override fun afterTextChanged(e: Editable) {
                    version++
                    if (!autoEditing) {
                        autoEditing = true
                        try {
                            if (pendingNewline >= 0) {
                                autoIndent(e, pendingNewline)
                            } else if (pendingWordEnd >= 0) {
                                autoDedent(e, pendingWordEnd)
                            }
                        } finally {
                            autoEditing = false
                        }
                    }
                    pendingNewline = -1
                    pendingWordEnd = -1
                    updateGutter()
                    listener?.onEdited()
                }
            },
        )
    }

    fun applyPalette(p: Palette) {
        palette = p
        setTextColor(p.text)
        setHintTextColor(p.textDim)
        highlightColor = p.selection
        gutterPaint.color = p.gutter
        numberPaint.color = p.gutterText
        numberPaint.typeface = Typeface.MONOSPACE
        linePaint.color = p.currentLine
        hlVersion = -1
        applyHighlight()
        invalidate()
    }

    fun load(text: String) {
        applying = true
        setText(text)
        applying = false
        undoStack.clear()
        redoStack.clear()
        tokens = IntArray(0)
        tokensVersion = -1
        hlVersion = -1
        version++
        updateGutter()
    }

    fun setTokens(
        packed: IntArray,
        forVersion: Int,
    ) {
        if (forVersion != version) {
            return
        }
        tokens = packed
        tokensVersion = forVersion
        hlVersion = -1
        applyHighlight()
    }

    fun canUndo() = undoStack.isNotEmpty()

    fun canRedo() = redoStack.isNotEmpty()

    fun undo() {
        val e = undoStack.removeLastOrNull() ?: return
        val t = text ?: return
        applying = true
        t.replace(e.start, min(t.length, e.start + e.inserted.length), e.removed)
        applying = false
        setSelection(min(t.length, e.start + e.removed.length))
        redoStack.add(e)
    }

    fun redo() {
        val e = redoStack.removeLastOrNull() ?: return
        val t = text ?: return
        applying = true
        t.replace(e.start, min(t.length, e.start + e.removed.length), e.inserted)
        applying = false
        setSelection(min(t.length, e.start + e.inserted.length))
        undoStack.add(e)
    }

    fun insert(
        s: String,
        caretBack: Int = 0,
    ) {
        val t = text ?: return
        val a = max(0, min(selectionStart, selectionEnd))
        val b = max(0, max(selectionStart, selectionEnd))
        t.replace(a, b, s)
        setSelection((a + s.length - caretBack).coerceIn(0, t.length))
    }

    fun indentSelection(out: Boolean) {
        val t = text ?: return
        val a = min(selectionStart, selectionEnd)
        val b = max(selectionStart, selectionEnd)
        if (a == b && !out) {
            insert(" ".repeat(tabWidth))
            return
        }
        var line = t.lastIndexOf('\n', max(0, a - 1)).let { if (a == 0) 0 else it + 1 }
        val pad = " ".repeat(tabWidth)
        var end = b
        while (line <= end && line <= t.length) {
            if (out) {
                var n = 0
                while (n < tabWidth && line + n < t.length && t[line + n] == ' ') {
                    n++
                }
                if (n > 0) {
                    t.delete(line, line + n)
                    end -= n
                }
            } else {
                t.insert(line, pad)
                end += tabWidth
            }
            val next = t.indexOf('\n', line)
            if (next < 0) {
                break
            }
            line = next + 1
        }
    }

    fun goToLine(n: Int) {
        val t = text ?: return
        var line = 1
        var i = 0
        while (line < n && i < t.length) {
            if (t[i] == '\n') {
                line++
            }
            i++
        }
        setSelection(i)
        bringPointIntoView(i)
    }

    fun reveal(
        start: Int,
        end: Int,
    ) {
        val len = text?.length ?: 0
        setSelection(start.coerceIn(0, len), end.coerceIn(0, len))
        bringPointIntoView(start.coerceIn(0, len))
    }

    fun find(
        query: String,
        forward: Boolean,
    ): Boolean {
        val t = text?.toString() ?: return false
        if (query.isEmpty()) {
            return false
        }
        val from = if (forward) max(selectionStart, selectionEnd) else min(selectionStart, selectionEnd) - 1
        var at = if (forward) t.indexOf(query, from, true) else t.lastIndexOf(query, max(0, from), true)
        if (at < 0) {
            at = if (forward) t.indexOf(query, 0, true) else t.lastIndexOf(query, t.length, true)
        }
        if (at < 0) {
            return false
        }
        reveal(at, at + query.length)
        return true
    }

    fun replaceCurrent(
        query: String,
        with: String,
    ): Boolean {
        val t = text ?: return false
        val a = min(selectionStart, selectionEnd)
        val b = max(selectionStart, selectionEnd)
        if (b > a && t.subSequence(a, b).toString().equals(query, true)) {
            t.replace(a, b, with)
            setSelection(a + with.length)
        }
        return find(query, true)
    }

    fun replaceAll(
        query: String,
        with: String,
    ): Int {
        val t = text ?: return 0
        if (query.isEmpty()) {
            return 0
        }
        var count = 0
        var at = t.toString().indexOf(query, 0, true)
        while (at >= 0) {
            t.replace(at, at + query.length, with)
            count++
            at = t.toString().indexOf(query, at + with.length, true)
        }
        return count
    }

    fun lineCount(): Int {
        refreshNewlines()
        return newlineCount + 1
    }

    fun lineOf(offset: Int): Int {
        refreshNewlines()
        var lo = 0
        var hi = newlineCount
        while (lo < hi) {
            val mid = (lo + hi) ushr 1
            if (newlines[mid] < offset) lo = mid + 1 else hi = mid
        }
        return lo + 1
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaler.onTouchEvent(event)
        if (scaler.isInProgress || event.pointerCount > 1) {
            return true
        }
        return super.onTouchEvent(event)
    }

    override fun onScrollChanged(
        horiz: Int,
        vert: Int,
        oldHoriz: Int,
        oldVert: Int,
    ) {
        super.onScrollChanged(horiz, vert, oldHoriz, oldVert)
        if (vert != oldVert) {
            val (from, to) = visibleRange()
            if (from < hlFrom || to > hlTo) {
                removeCallbacks(highlightLater)
                postDelayed(highlightLater, 60)
            }
        }
    }

    private val highlightLater = Runnable { applyHighlight() }

    override fun onDraw(canvas: Canvas) {
        val layout = layout
        if (layout == null) {
            super.onDraw(canvas)
            return
        }
        val top = extendedPaddingTop
        val left = compoundPaddingLeft
        val caret = selectionStart
        if (caret >= 0 && isFocused) {
            val line = layout.getLineForOffset(caret)
            var first = line
            var last = line
            val t = text
            while (first > 0 && t[layout.getLineStart(first) - 1] != '\n') {
                first--
            }
            while (last < layout.lineCount - 1 && t[layout.getLineEnd(last) - 1] != '\n') {
                last++
            }
            canvas.drawRect(
                scrollX.toFloat(),
                (top + layout.getLineTop(first)).toFloat(),
                (scrollX + width).toFloat(),
                (top + layout.getLineBottom(last)).toFloat(),
                linePaint,
            )
        }
        super.onDraw(canvas)
        drawSquiggles(canvas, layout, top, left)
        if (showLineNumbers) {
            drawGutter(canvas, layout, top)
        }
    }

    private fun drawGutter(
        canvas: Canvas,
        layout: android.text.Layout,
        top: Int,
    ) {
        val sx = scrollX.toFloat()
        canvas.drawRect(sx, scrollY.toFloat(), sx + gutterWidth, (scrollY + height).toFloat(), gutterPaint)
        val first = layout.getLineForVertical(max(0, scrollY - top))
        val last = layout.getLineForVertical(scrollY + height - top)
        val t = text
        val caretLine = if (selectionStart >= 0) lineOf(selectionStart) else -1
        val errorLines = HashMap<Int, Boolean>()
        for (d in diagnostics) {
            errorLines[d.line] = errorLines[d.line] == true || d.error
        }
        numberPaint.textSize = textSize * 0.82f
        numberPaint.textAlign = Paint.Align.RIGHT
        val x = sx + gutterWidth - resources.displayMetrics.density * 8
        for (l in first..last) {
            val start = layout.getLineStart(l)
            val isStart = l == 0 || t[start - 1] == '\n'
            if (!isStart) {
                continue
            }
            val number = lineOf(start)
            val y = (top + layout.getLineBaseline(l)).toFloat()
            val err = errorLines[number]
            numberPaint.color =
                when {
                    err == true -> palette.error
                    err == false -> palette.warning
                    number == caretLine -> palette.text
                    else -> palette.gutterText
                }
            canvas.drawText(number.toString(), x, y, numberPaint)
            if (err != null) {
                val r = resources.displayMetrics.density * 2.5f
                canvas.drawCircle(sx + r * 2, y - textSize * 0.32f, r, numberPaint)
            }
        }
    }

    private fun drawSquiggles(
        canvas: Canvas,
        layout: android.text.Layout,
        top: Int,
        left: Int,
    ) {
        if (diagnostics.isEmpty()) {
            return
        }
        val len = text?.length ?: return
        val d = resources.displayMetrics.density
        val visFirst = layout.getLineForVertical(max(0, scrollY - top))
        val visLast = layout.getLineForVertical(scrollY + height - top)
        for (diag in diagnostics) {
            val s = diag.start.coerceIn(0, len)
            val e = diag.end.coerceIn(s, len)
            val l1 = layout.getLineForOffset(s)
            val l2 = layout.getLineForOffset(max(s, e - 1))
            if (l2 < visFirst || l1 > visLast) {
                continue
            }
            squigglePaint.color = if (diag.error) palette.error else palette.warning
            for (l in l1..l2) {
                var x1 = if (l == l1) layout.getPrimaryHorizontal(s) else layout.getLineLeft(l)
                var x2 = if (l == l2 && e > s) layout.getPrimaryHorizontal(e) else layout.getLineRight(l)
                if (x1 > x2) {
                    val tmp = x1
                    x1 = x2
                    x2 = tmp
                }
                if (x2 - x1 < d * 6) {
                    x2 = x1 + d * 6
                }
                val y = top + layout.getLineBaseline(l) + d * 3
                squiggle.reset()
                squiggle.moveTo(left + x1, y)
                var x = x1
                var up = true
                val step = d * 2.5f
                while (x < x2) {
                    x += step
                    squiggle.lineTo(left + min(x, x2), if (up) y - d * 1.6f else y)
                    up = !up
                }
                canvas.drawPath(squiggle, squigglePaint)
            }
        }
    }

    private fun refreshNewlines() {
        if (newlineVersion == version) {
            return
        }
        val t = text ?: return
        var count = 0
        for (i in 0 until t.length) {
            if (t[i] == '\n') {
                count++
            }
        }
        if (newlines.size < count) {
            newlines = IntArray(count + 64)
        }
        var n = 0
        for (i in 0 until t.length) {
            if (t[i] == '\n') {
                newlines[n++] = i
            }
        }
        newlineCount = count
        newlineVersion = version
    }

    private fun updateGutter() {
        if (!showLineNumbers) {
            if (gutterWidth != 0 || paddingLeft == 0) {
                gutterWidth = 0
                setPadding(dp(8), paddingTop, dp(8), paddingBottom)
            }
            return
        }
        val digits = max(2, lineCount().toString().length)
        numberPaint.textSize = textSize * 0.82f
        val want = (numberPaint.measureText("9".repeat(digits)) + dp(18)).toInt()
        if (digits != gutterDigits || want != gutterWidth) {
            gutterDigits = digits
            gutterWidth = want
            setPadding(want + dp(6), paddingTop, dp(8), paddingBottom)
        }
    }

    private fun applyWrap() {
        val wide = !wrap && lang != "fa"
        setHorizontallyScrolling(wide)
        isHorizontalScrollBarEnabled = wide
    }

    private fun dp(v: Int): Int = (v * resources.displayMetrics.density).toInt()

    private fun visibleRange(): Pair<Int, Int> {
        val layout = layout ?: return 0 to 0
        val top = extendedPaddingTop
        val first = layout.getLineForVertical(max(0, scrollY - top))
        val last = layout.getLineForVertical(scrollY + height - top)
        return layout.getLineStart(first) to layout.getLineEnd(last)
    }

    private fun applyHighlight() {
        val e = text ?: return
        if (tokensVersion != version || layout == null) {
            return
        }
        val (vs, ve) = visibleRange()
        val span = max(2000, ve - vs)
        val from = max(0, vs - span)
        val to = min(e.length, ve + span)
        if (hlVersion == version && from >= hlFrom && to <= hlTo) {
            return
        }
        for (old in e.getSpans(from, to, HlSpan::class.java)) {
            e.removeSpan(old)
        }
        val n = tokens.size / 3
        var lo = 0
        var hi = n
        while (lo < hi) {
            val mid = (lo + hi) ushr 1
            if (tokens[mid * 3 + 1] + tokens[mid * 3 + 2] <= from) lo = mid + 1 else hi = mid
        }
        var i = lo
        while (i < n) {
            val kind = tokens[i * 3]
            val s = tokens[i * 3 + 1]
            if (s >= to) {
                break
            }
            val end = min(e.length, s + tokens[i * 3 + 2])
            if (s < end) {
                e.setSpan(HlSpan(palette.token(kind)), s, end, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE)
            }
            i++
        }
        hlFrom = from
        hlTo = to
        hlVersion = version
    }

    private fun record(
        start: Int,
        removed: String,
        inserted: String,
    ) {
        if (removed.isEmpty() && inserted.isEmpty()) {
            return
        }
        val now = SystemClock.uptimeMillis()
        val last = undoStack.lastOrNull()
        redoStack.clear()
        if (last != null && now - last.time < 1500) {
            if (removed.isEmpty() && inserted.length == 1 && inserted != "\n" && last.removed.isEmpty() &&
                start == last.start + last.inserted.length
            ) {
                last.inserted += inserted
                last.time = now
                return
            }
            if (inserted.isEmpty() && removed.length == 1 && last.inserted.isEmpty() && start + 1 == last.start) {
                last.start = start
                last.removed = removed + last.removed
                last.time = now
                return
            }
        }
        undoStack.add(Edit(start, removed, inserted, now))
        if (undoStack.size > 400) {
            undoStack.removeAt(0)
        }
    }

    private fun lineStart(
        e: CharSequence,
        at: Int,
    ): Int {
        var i = at
        while (i > 0 && e[i - 1] != '\n') {
            i--
        }
        return i
    }

    private fun indentOf(
        e: CharSequence,
        start: Int,
    ): Int {
        var i = start
        while (i < e.length && (e[i] == ' ' || e[i] == '\t')) {
            i++
        }
        return i - start
    }

    private fun autoIndent(
        e: Editable,
        newline: Int,
    ) {
        val ls = lineStart(e, newline)
        val prev = e.subSequence(ls, newline).toString()
        var indent = indentOf(prev, 0)
        val trimmed = prev.trim()
        if (trimmed.endsWith(":") && !trimmed.startsWith("//")) {
            indent += tabWidth
        }
        if (indent > 0) {
            e.insert(newline + 1, " ".repeat(indent))
        }
    }

    private fun autoDedent(
        e: Editable,
        wordEnd: Int,
    ) {
        val ls = lineStart(e, wordEnd)
        var le = wordEnd
        while (le < e.length && e[le] != '\n') {
            le++
        }
        val word = e.subSequence(ls, le).toString().trim()
        if (word !in DEDENT_WORDS) {
            return
        }
        val indent = indentOf(e, ls)
        if (indent < tabWidth) {
            return
        }
        var p = ls - 1
        var prevIndent = -1
        while (p > 0) {
            val ps = lineStart(e, p)
            val line = e.subSequence(ps, p).toString()
            if (line.isNotBlank()) {
                prevIndent = indentOf(line, 0) + if (line.trim().endsWith(":")) tabWidth else 0
                break
            }
            p = ps - 1
        }
        if (prevIndent >= 0 && indent >= prevIndent) {
            e.delete(ls, ls + tabWidth)
        }
    }

    companion object {
        private val DEDENT_WORDS = setOf("end", "else", "پایان", "وگرنه")
    }
}

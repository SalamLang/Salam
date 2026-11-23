package ir.salamlang.host

import android.content.Context
import android.text.InputType
import android.view.KeyEvent
import android.view.MotionEvent
import android.view.SurfaceView
import android.view.inputmethod.BaseInputConnection
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputConnection

class SalamView(context: Context) : SurfaceView(context) {
    init {
        isFocusable = true
        isFocusableInTouchMode = true
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val a = event.actionMasked
        when (a) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                val i = event.actionIndex
                Host.nativePointer(0, event.getPointerId(i), event.getX(i), event.getY(i))
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                val i = event.actionIndex
                Host.nativePointer(1, event.getPointerId(i), event.getX(i), event.getY(i))
            }
            MotionEvent.ACTION_CANCEL -> {
                for (i in 0 until event.pointerCount) {
                    Host.nativePointer(1, event.getPointerId(i), event.getX(i), event.getY(i))
                }
            }
            MotionEvent.ACTION_MOVE -> {
                for (i in 0 until event.pointerCount) {
                    Host.nativePointer(2, event.getPointerId(i), event.getX(i), event.getY(i))
                }
            }
        }
        return true
    }

    override fun onCheckIsTextEditor(): Boolean = true

    override fun onCreateInputConnection(outAttrs: EditorInfo): InputConnection {
        outAttrs.inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_MULTI_LINE
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_FULLSCREEN or EditorInfo.IME_FLAG_NO_EXTRACT_UI
        return SalamInput(this)
    }
}

private class SalamInput(view: SalamView) : BaseInputConnection(view, false) {
    override fun commitText(text: CharSequence, newCursorPosition: Int): Boolean {
        Host.nativeCommit(text.toString())
        return true
    }

    override fun setComposingText(text: CharSequence, newCursorPosition: Int): Boolean {
        val s = text.toString()
        Host.nativePreedit(s, if (newCursorPosition > 0) s.toByteArray(Charsets.UTF_8).size else 0)
        return true
    }

    override fun finishComposingText(): Boolean = true

    override fun deleteSurroundingText(beforeLength: Int, afterLength: Int): Boolean {
        repeat(beforeLength) { key(KeyEvent.KEYCODE_DEL) }
        repeat(afterLength) { key(KeyEvent.KEYCODE_FORWARD_DEL) }
        return true
    }

    override fun sendKeyEvent(event: KeyEvent): Boolean {
        Host.nativeKey(if (event.action == KeyEvent.ACTION_DOWN) 1 else 0, event.keyCode, event.metaState)
        val ch = event.unicodeChar
        if (event.action == KeyEvent.ACTION_DOWN && ch > 0) {
            Host.nativeText(String(Character.toChars(ch)))
        }
        return true
    }

    override fun performEditorAction(actionCode: Int): Boolean {
        key(KeyEvent.KEYCODE_ENTER)
        return true
    }

    private fun key(code: Int) {
        Host.nativeKey(1, code, 0)
        Host.nativeKey(0, code, 0)
    }
}

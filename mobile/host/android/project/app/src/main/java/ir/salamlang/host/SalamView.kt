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
        val cfg = Host.imeCfg
        val kb = cfg and 15
        val enter = (cfg shr 4) and 15
        val caps = (cfg shr 8) and 3
        val fix = (cfg shr 10) and 3
        val content = (cfg shr 12) and 15
        val multi = (cfg and 65536) != 0
        var type = InputType.TYPE_CLASS_TEXT
        when (kb) {
            1 -> type = type or InputType.TYPE_TEXT_VARIATION_EMAIL_ADDRESS
            2 -> type = type or InputType.TYPE_TEXT_VARIATION_URI
            3 -> type = InputType.TYPE_CLASS_PHONE
            4 -> type = InputType.TYPE_CLASS_NUMBER
            5 -> type = InputType.TYPE_CLASS_NUMBER or InputType.TYPE_NUMBER_FLAG_DECIMAL or InputType.TYPE_NUMBER_FLAG_SIGNED
            6 -> type = type or InputType.TYPE_TEXT_VARIATION_PASSWORD
            7 -> type = type or InputType.TYPE_TEXT_VARIATION_WEB_EDIT_TEXT
        }
        if (content == 2 || content == 3) {
            if (kb != 6) type = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
        }
        if ((type and InputType.TYPE_MASK_CLASS) == InputType.TYPE_CLASS_TEXT) {
            when (caps) {
                0 -> if (kb == 0 && fix != 1) type = type or InputType.TYPE_TEXT_FLAG_CAP_SENTENCES
                2 -> type = type or InputType.TYPE_TEXT_FLAG_CAP_WORDS
                3 -> type = type or InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS
            }
            if (fix == 1) type = type or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
            if (fix == 2 || (fix == 0 && kb == 0)) type = type or InputType.TYPE_TEXT_FLAG_AUTO_CORRECT
            if (multi) type = type or InputType.TYPE_TEXT_FLAG_MULTI_LINE
        }
        outAttrs.inputType = type
        val action = when (enter) {
            2 -> EditorInfo.IME_ACTION_DONE
            3 -> EditorInfo.IME_ACTION_GO
            4 -> EditorInfo.IME_ACTION_NEXT
            5 -> EditorInfo.IME_ACTION_PREVIOUS
            6 -> EditorInfo.IME_ACTION_SEARCH
            7 -> EditorInfo.IME_ACTION_SEND
            else -> EditorInfo.IME_ACTION_UNSPECIFIED
        }
        outAttrs.imeOptions = action or EditorInfo.IME_FLAG_NO_FULLSCREEN or EditorInfo.IME_FLAG_NO_EXTRACT_UI
        if (multi && enter == 0) outAttrs.imeOptions = outAttrs.imeOptions or EditorInfo.IME_FLAG_NO_ENTER_ACTION
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

package ir.salamlang.app

import android.content.Context
import android.os.Handler
import android.os.HandlerThread
import android.os.Looper
import org.json.JSONObject

class Diagnostic(
    val start: Int,
    val end: Int,
    val line: Int,
    val error: Boolean,
    val code: String,
    val message: String,
)

object Analyzer {
    private val thread = HandlerThread("salam-analyzer").apply { start() }
    private val worker = Handler(thread.looper)
    private val main = Handler(Looper.getMainLooper())

    @Volatile
    private var booted = false

    fun boot(ctx: Context) {
        val app = ctx.applicationContext
        worker.post { bootNow(app) }
    }

    private fun bootNow(ctx: Context) {
        if (!booted) {
            val std = StdInstaller.ensure(ctx)
            Engine.load()
            Engine.nativeBoot(std.absolutePath)
            booted = true
        }
    }

    fun whenReady(
        ctx: Context,
        done: () -> Unit,
    ) {
        val app = ctx.applicationContext
        worker.post {
            bootNow(app)
            main.post(done)
        }
    }

    fun tokens(
        ctx: Context,
        source: String,
        lang: String,
        done: (IntArray) -> Unit,
    ) {
        val app = ctx.applicationContext
        worker.removeCallbacksAndMessages(TOKENS)
        worker.postAtTime({
            bootNow(app)
            val packed = parseTokens(Engine.nativeTokens(source, lang))
            main.post { done(packed) }
        }, TOKENS, android.os.SystemClock.uptimeMillis())
    }

    fun check(
        ctx: Context,
        source: String,
        path: String,
        lang: String,
        done: (List<Diagnostic>) -> Unit,
    ) {
        val app = ctx.applicationContext
        worker.removeCallbacksAndMessages(CHECK)
        worker.postAtTime({
            bootNow(app)
            val list = parseDiagnostics(source, Engine.nativeCheck(source, path, lang))
            main.post { done(list) }
        }, CHECK, android.os.SystemClock.uptimeMillis())
    }

    fun detect(
        ctx: Context,
        source: String,
        done: (String) -> Unit,
    ) {
        val app = ctx.applicationContext
        worker.post {
            bootNow(app)
            val lang = Engine.nativeDetect(source)
            main.post { done(lang) }
        }
    }

    fun version(
        ctx: Context,
        done: (String) -> Unit,
    ) {
        val app = ctx.applicationContext
        worker.post {
            bootNow(app)
            val v = Engine.nativeVersion()
            main.post { done(v) }
        }
    }

    private val TOKENS = Any()
    private val CHECK = Any()

    private fun parseTokens(packed: String): IntArray {
        if (packed.isEmpty()) {
            return IntArray(0)
        }
        val out = IntArray(packed.count { it == ';' } * 3)
        var n = 0
        var value = 0
        for (c in packed) {
            when (c) {
                ',', ';' -> {
                    if (n < out.size) {
                        out[n++] = value
                    }
                    value = 0
                }
                else -> value = value * 10 + (c - '0')
            }
        }
        return out
    }

    fun parseDiagnostics(
        source: String,
        json: String,
    ): List<Diagnostic> {
        if (json.isBlank()) {
            return emptyList()
        }
        val starts = lineStarts(source)
        val list = ArrayList<Diagnostic>()
        for (raw in json.split('\n')) {
            val text = raw.trim()
            if (!text.startsWith("{")) {
                continue
            }
            try {
                val o = JSONObject(text)
                val line = o.optInt("line", 1).coerceAtLeast(1)
                val col = o.optInt("column", 1).coerceAtLeast(1)
                val len = o.optInt("length", 0)
                val lineStart = starts.getOrElse(line - 1) { source.length }
                val lineEnd = starts.getOrElse(line) { source.length + 1 } - 1
                val start = byteColumnToOffset(source, lineStart, lineEnd, col - 1)
                var end = byteColumnToOffset(source, lineStart, lineEnd, col - 1 + len)
                if (end <= start) {
                    end = wordEnd(source, start, lineEnd)
                }
                list.add(
                    Diagnostic(
                        start,
                        end,
                        line,
                        o.optString("level") != "warning",
                        o.optString("code"),
                        o.optString("message"),
                    ),
                )
            } catch (_: Exception) {
            }
        }
        list.sortBy { it.start }
        return list
    }

    private fun lineStarts(s: String): IntArray {
        var count = 1
        for (c in s) {
            if (c == '\n') {
                count++
            }
        }
        val out = IntArray(count)
        var n = 1
        for (i in s.indices) {
            if (s[i] == '\n') {
                out[n++] = i + 1
            }
        }
        return out
    }

    private fun byteColumnToOffset(
        s: String,
        lineStart: Int,
        lineEnd: Int,
        bytes: Int,
    ): Int {
        var i = lineStart
        var b = 0
        val limit = lineEnd.coerceAtMost(s.length)
        while (i < limit && b < bytes) {
            val c = s[i]
            b +=
                when {
                    c.code < 0x80 -> 1
                    c.code < 0x800 -> 2
                    Character.isHighSurrogate(c) -> 4
                    Character.isLowSurrogate(c) -> 0
                    else -> 3
                }
            i++
        }
        return i.coerceAtMost(s.length)
    }

    private fun wordEnd(
        s: String,
        start: Int,
        lineEnd: Int,
    ): Int {
        var i = start
        val limit = lineEnd.coerceAtMost(s.length)
        while (i < limit && (Character.isLetterOrDigit(s[i]) || s[i] == '_' || s[i] == '‌')) {
            i++
        }
        return if (i == start) (start + 1).coerceAtMost(s.length) else i
    }
}

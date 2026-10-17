package ir.salamlang.app

import android.app.Service
import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.HandlerThread
import android.os.IBinder
import android.os.Message
import android.os.Messenger
import android.os.ParcelFileDescriptor
import android.os.Process
import android.os.SystemClock
import java.io.FileInputStream
import java.io.FileOutputStream
import java.io.InputStream
import java.nio.ByteBuffer
import java.nio.CharBuffer
import java.nio.charset.CodingErrorAction

class EngineService : Service() {
    private val control = HandlerThread("salam-control").apply { start() }
    private lateinit var messenger: Messenger

    @Volatile
    private var client: Messenger? = null

    @Volatile
    private var running = false
    private var stdin: FileOutputStream? = null
    private var ready = false

    override fun onCreate() {
        super.onCreate()
        messenger = Messenger(Handler(control.looper) { handle(it) })
    }

    override fun onBind(intent: Intent?): IBinder = messenger.binder

    private fun boot() {
        if (ready) {
            return
        }
        val std = StdInstaller.ensure(this)
        Engine.load()
        Engine.nativeBoot(std.absolutePath)
        val fds = Engine.nativeRedirect() ?: IntArray(0)
        if (fds.size == 3) {
            pump(fds[0], RunClient.MSG_OUT, true)
            pump(fds[1], RunClient.MSG_ERR, false)
            stdin = FileOutputStream(ParcelFileDescriptor.adoptFd(fds[2]).fileDescriptor)
        }
        ready = true
    }

    private fun handle(msg: Message): Boolean {
        msg.replyTo?.let { client = it }
        when (msg.what) {
            RunClient.MSG_RUN -> start(msg.data)
            RunClient.MSG_INPUT -> {
                val text = msg.data.getString("text") ?: ""
                try {
                    stdin?.write(text.toByteArray())
                    stdin?.flush()
                } catch (_: Exception) {
                }
            }
            RunClient.MSG_STOP -> stop()
        }
        return true
    }

    private fun start(data: Bundle) {
        if (running) {
            return
        }
        boot()
        running = true
        val args = data.getStringArrayList("args") ?: arrayListOf()
        val cwd = data.getString("cwd") ?: filesDir.absolutePath
        val token = data.getInt("token")
        Thread(null, {
            Engine.nativeDrainStdin()
            Engine.nativeClearStop()
            Engine.nativeChdir(cwd)
            val t0 = SystemClock.elapsedRealtime()
            val rc = Engine.nativeCli(args.joinToString("\n"))
            val ms = SystemClock.elapsedRealtime() - t0
            Engine.nativeMark("$MARK$token:$rc:$ms$MARK_END")
        }, "salam-run", STACK).start()
    }

    private fun stop() {
        if (!running) {
            return
        }
        Engine.nativeStop()
        Handler(control.looper).postDelayed({
            if (running) {
                send(RunClient.MSG_KILLED, Bundle())
                Process.killProcess(Process.myPid())
            }
        }, 1500)
    }

    private fun send(
        what: Int,
        data: Bundle,
    ) {
        val c = client ?: return
        try {
            val m = Message.obtain(null, what)
            m.data = data
            c.send(m)
        } catch (_: Exception) {
        }
    }

    private fun pump(
        fd: Int,
        what: Int,
        watchMark: Boolean,
    ) {
        val input: InputStream = FileInputStream(ParcelFileDescriptor.adoptFd(fd).fileDescriptor)
        Thread({
            val decoder =
                Charsets.UTF_8
                    .newDecoder()
                    .onMalformedInput(CodingErrorAction.REPLACE)
                    .onUnmappableCharacter(CodingErrorAction.REPLACE)
            val bytes = ByteArray(8192)
            val inBuf = ByteBuffer.allocate(16384)
            val chars = CharBuffer.allocate(16384)
            val pending = StringBuilder()
            while (true) {
                val n =
                    try {
                        input.read(bytes)
                    } catch (_: Exception) {
                        -1
                    }
                if (n <= 0) {
                    break
                }
                inBuf.put(bytes, 0, n)
                inBuf.flip()
                decoder.decode(inBuf, chars, false)
                inBuf.compact()
                chars.flip()
                pending.append(chars)
                chars.clear()
                if (watchMark) {
                    flushWithMarks(pending)
                } else if (pending.isNotEmpty()) {
                    send(what, Bundle().apply { putString("text", pending.toString()) })
                    pending.setLength(0)
                }
            }
        }, "salam-pipe-$what").apply {
            isDaemon = true
            start()
        }
    }

    private fun flushWithMarks(pending: StringBuilder) {
        while (true) {
            val at = pending.indexOf(MARK)
            if (at < 0) {
                var keep = 0
                for (k in MARK.length - 1 downTo 1) {
                    if (pending.length >= k && pending.regionMatchesAt(pending.length - k, MARK, k)) {
                        keep = k
                        break
                    }
                }
                val emit = pending.length - keep
                if (emit > 0) {
                    send(RunClient.MSG_OUT, Bundle().apply { putString("text", pending.substring(0, emit)) })
                    pending.delete(0, emit)
                }
                return
            }
            val close = pending.indexOf(MARK_END, at + MARK.length)
            if (close < 0) {
                if (at > 0) {
                    send(RunClient.MSG_OUT, Bundle().apply { putString("text", pending.substring(0, at)) })
                    pending.delete(0, at)
                }
                return
            }
            if (at > 0) {
                send(RunClient.MSG_OUT, Bundle().apply { putString("text", pending.substring(0, at)) })
            }
            val parts = pending.substring(at + MARK.length, close).split(':')
            pending.delete(0, close + MARK_END.length)
            running = false
            send(
                RunClient.MSG_DONE,
                Bundle().apply {
                    putInt("token", parts.getOrNull(0)?.toIntOrNull() ?: 0)
                    putInt("rc", parts.getOrNull(1)?.toIntOrNull() ?: 1)
                    putLong("ms", parts.getOrNull(2)?.toLongOrNull() ?: 0L)
                },
            )
        }
    }

    private fun StringBuilder.regionMatchesAt(
        offset: Int,
        other: String,
        len: Int,
    ): Boolean {
        for (i in 0 until len) {
            if (this[offset + i] != other[i]) {
                return false
            }
        }
        return true
    }

    companion object {
        private const val MARK = "\u0000\u001eSALAM-END:"
        private const val MARK_END = "\u001e\u0000"
        private const val STACK = 64L * 1024 * 1024
    }
}

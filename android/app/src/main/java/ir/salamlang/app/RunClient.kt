package ir.salamlang.app

import android.content.ComponentName
import android.content.Context
import android.content.Intent
import android.content.ServiceConnection
import android.os.Bundle
import android.os.Handler
import android.os.IBinder
import android.os.Looper
import android.os.Message
import android.os.Messenger

class RunClient(
    private val ctx: Context,
    private val listener: Listener,
) {
    interface Listener {
        fun onOutput(
            text: String,
            error: Boolean,
        )

        fun onDone(
            token: Int,
            rc: Int,
            ms: Long,
        )
    }

    private val incoming =
        Messenger(
            Handler(Looper.getMainLooper()) { msg ->
                when (msg.what) {
                    MSG_OUT -> listener.onOutput(msg.data.getString("text") ?: "", false)
                    MSG_ERR -> listener.onOutput(msg.data.getString("text") ?: "", true)
                    MSG_DONE -> finish(msg.data.getInt("token"), msg.data.getInt("rc"), msg.data.getLong("ms"))
                    MSG_KILLED -> finish(activeToken, RC_STOPPED, 0)
                }
                true
            },
        )
    private var service: Messenger? = null
    private var bound = false
    private val queue = ArrayList<Message>()
    private var nextToken = 1
    var activeToken = 0
        private set
    val isRunning: Boolean get() = activeToken != 0

    private val connection =
        object : ServiceConnection {
            override fun onServiceConnected(
                name: ComponentName?,
                binder: IBinder?,
            ) {
                service = Messenger(binder)
                val pending = ArrayList(queue)
                queue.clear()
                pending.forEach { deliver(it) }
            }

            override fun onServiceDisconnected(name: ComponentName?) {
                service = null
                if (activeToken != 0) {
                    finish(activeToken, RC_STOPPED, 0)
                }
            }
        }

    fun bind() {
        if (!bound) {
            bound = ctx.bindService(Intent(ctx, EngineService::class.java), connection, Context.BIND_AUTO_CREATE)
        }
    }

    fun unbind() {
        if (bound) {
            try {
                ctx.unbindService(connection)
            } catch (_: Exception) {
            }
            bound = false
            service = null
        }
    }

    fun run(
        args: List<String>,
        cwd: String,
    ): Int {
        bind()
        val token = nextToken++
        activeToken = token
        val m = Message.obtain(null, MSG_RUN)
        m.data =
            Bundle().apply {
                putStringArrayList("args", ArrayList(args))
                putString("cwd", cwd)
                putInt("token", token)
            }
        send(m)
        return token
    }

    fun input(text: String) {
        val m = Message.obtain(null, MSG_INPUT)
        m.data = Bundle().apply { putString("text", text) }
        send(m)
    }

    fun stop() {
        if (activeToken != 0) {
            send(Message.obtain(null, MSG_STOP))
        }
    }

    private fun finish(
        token: Int,
        rc: Int,
        ms: Long,
    ) {
        if (token == 0 || token != activeToken) {
            return
        }
        activeToken = 0
        listener.onDone(token, rc, ms)
    }

    private fun send(m: Message) {
        if (service == null) {
            bind()
            queue.add(m)
            return
        }
        deliver(m)
    }

    private fun deliver(m: Message) {
        m.replyTo = incoming
        try {
            service?.send(m)
        } catch (_: Exception) {
            queue.add(m)
            service = null
            bound = false
            bind()
        }
    }

    companion object {
        const val MSG_RUN = 1
        const val MSG_INPUT = 2
        const val MSG_STOP = 3
        const val MSG_OUT = 10
        const val MSG_ERR = 11
        const val MSG_DONE = 12
        const val MSG_KILLED = 13
        const val RC_STOPPED = 130
    }
}

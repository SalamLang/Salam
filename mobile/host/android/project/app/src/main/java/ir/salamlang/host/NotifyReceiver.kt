package ir.salamlang.host

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent

class NotifyReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        Host.postNotification(
            context,
            intent.getIntExtra("id", 0),
            intent.getStringExtra("title") ?: "",
            intent.getStringExtra("body") ?: ""
        )
    }
}

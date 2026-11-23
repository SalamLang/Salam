package ir.salamlang.host

import android.content.Context
import java.io.File

object Fonts {
    fun install(context: Context) {
        val names = try {
            context.assets.list("fonts")
        } catch (e: Exception) {
            null
        } ?: return
        if (names.isEmpty()) return
        val dir = File(context.filesDir, "fonts")
        dir.mkdirs()
        val stamp = File(dir, ".stamp")
        val version = try {
            context.packageManager.getPackageInfo(context.packageName, 0).lastUpdateTime.toString()
        } catch (e: Exception) {
            return
        }
        if (stamp.exists() && stamp.readText() == version) return
        for (name in names) {
            try {
                val tmp = File(dir, "$name.part")
                context.assets.open("fonts/$name").use { input ->
                    tmp.outputStream().use { input.copyTo(it) }
                }
                tmp.renameTo(File(dir, name))
            } catch (e: Exception) {
            }
        }
        stamp.writeText(version)
    }
}

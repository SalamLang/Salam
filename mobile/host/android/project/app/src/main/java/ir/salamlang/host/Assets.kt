package ir.salamlang.host

import android.content.Context
import java.io.File

object Assets {
    fun install(context: Context) {
        val top = try {
            context.assets.list("assets")
        } catch (e: Exception) {
            null
        } ?: return
        if (top.isEmpty()) return
        val dir = File(context.filesDir, "assets")
        dir.mkdirs()
        val stamp = File(dir, ".stamp")
        val version = try {
            context.packageManager.getPackageInfo(context.packageName, 0).lastUpdateTime.toString()
        } catch (e: Exception) {
            return
        }
        if (stamp.exists() && stamp.readText() == version) return
        copy(context, "assets", dir)
        stamp.writeText(version)
    }

    private fun copy(context: Context, path: String, target: File) {
        val names = try {
            context.assets.list(path)
        } catch (e: Exception) {
            null
        } ?: return
        for (name in names) {
            val child = "$path/$name"
            val sub = try {
                context.assets.list(child)
            } catch (e: Exception) {
                null
            }
            if (sub != null && sub.isNotEmpty()) {
                val d = File(target, name)
                d.mkdirs()
                copy(context, child, d)
                continue
            }
            try {
                val tmp = File(target, "$name.part")
                context.assets.open(child).use { input ->
                    tmp.outputStream().use { input.copyTo(it) }
                }
                tmp.renameTo(File(target, name))
            } catch (e: Exception) {
            }
        }
    }
}

package ir.salamlang.app

import android.content.Context
import java.io.File
import java.util.zip.ZipInputStream

object StdInstaller {
    fun dir(ctx: Context): File = File(ctx.filesDir, "std")

    fun isReady(ctx: Context): Boolean {
        val marker = File(dir(ctx), ".version")
        return marker.isFile && marker.readText() == bundledVersion(ctx)
    }

    @Synchronized
    fun ensure(ctx: Context): File {
        val root = dir(ctx)
        if (isReady(ctx)) {
            return root
        }
        val tmp = File(ctx.filesDir, "std.tmp")
        tmp.deleteRecursively()
        tmp.mkdirs()
        val base = tmp.canonicalPath + File.separator
        ctx.assets.open("std.zip").use { raw ->
            ZipInputStream(raw.buffered()).use { zip ->
                while (true) {
                    val entry = zip.nextEntry ?: break
                    val out = File(tmp, entry.name)
                    if (!out.canonicalPath.startsWith(base)) {
                        continue
                    }
                    if (entry.isDirectory) {
                        out.mkdirs()
                    } else {
                        out.parentFile?.mkdirs()
                        out.outputStream().use { zip.copyTo(it) }
                    }
                }
            }
        }
        File(tmp, ".version").writeText(bundledVersion(ctx))
        root.deleteRecursively()
        if (!tmp.renameTo(root)) {
            tmp.copyRecursively(root, overwrite = true)
            tmp.deleteRecursively()
        }
        return root
    }

    private fun bundledVersion(ctx: Context): String = ctx.assets.open("std.version").use { it.readBytes().decodeToString().trim() }
}

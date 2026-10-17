package ir.salamlang.app

import android.content.Context
import java.io.File

object Workspace {
    fun dir(ctx: Context): File = File(ctx.filesDir, "workspace").apply { mkdirs() }

    fun cache(ctx: Context): File = File(ctx.cacheDir, "salam").apply { mkdirs() }

    fun files(ctx: Context): List<File> =
        dir(ctx)
            .listFiles { f -> f.isFile && !f.name.startsWith(".") }
            ?.sortedByDescending { it.lastModified() }
            ?: emptyList()

    fun cleanName(raw: String): String {
        var name = raw.trim().replace(Regex("""[\\/:*?"<>|\u0000-\u001f]"""), "_")
        if (name.isEmpty()) {
            name = "main"
        }
        if (!name.contains('.')) {
            name += ".salam"
        }
        return name
    }

    fun unique(
        ctx: Context,
        wanted: String,
    ): File {
        val base = cleanName(wanted)
        var f = File(dir(ctx), base)
        if (!f.exists()) {
            return f
        }
        val stem = base.substringBeforeLast('.')
        val ext = base.substringAfterLast('.', "salam")
        var i = 2
        while (f.exists()) {
            f = File(dir(ctx), "$stem-$i.$ext")
            i++
        }
        return f
    }

    fun template(lang: String): String =
        if (lang == "fa") {
            "تابع ریشه:\n    سرچاپ «سلام دنیا!»\nپایان\n"
        } else {
            "func main:\n    println \"Hello, Salam!\"\nend\n"
        }

    class Example(
        val id: String,
        val mode: String,
        val titleEn: String,
        val titleFa: String,
    )

    fun examples(ctx: Context): List<Example> =
        try {
            ctx.assets
                .open("examples/index.tsv")
                .bufferedReader()
                .readLines()
                .mapNotNull { line ->
                    val p = line.split('\t')
                    if (p.size >= 4) Example(p[0], p[1], p[2], p[3]) else null
                }
        } catch (_: Exception) {
            emptyList()
        }

    fun exampleSource(
        ctx: Context,
        id: String,
        lang: String,
    ): String? =
        try {
            ctx.assets
                .open("examples/$lang/$id.salam")
                .bufferedReader()
                .readText()
        } catch (_: Exception) {
            null
        }
}

package ir.salamlang.app

import android.content.Context

class Palette(
    val dark: Boolean,
) {
    val brand = 0xFFFF5C00.toInt()
    val background = if (dark) 0xFF15171C.toInt() else 0xFFFFFFFF.toInt()
    val surface = if (dark) 0xFF1E2128.toInt() else 0xFFF5F6F8.toInt()
    val surfaceHigh = if (dark) 0xFF272B33.toInt() else 0xFFEBEDF0.toInt()
    val border = if (dark) 0xFF30343D.toInt() else 0xFFDDE0E5.toInt()
    val text = if (dark) 0xFFE6E8EC.toInt() else 0xFF1B1D22.toInt()
    val textDim = if (dark) 0xFF8D93A0.toInt() else 0xFF6B7280.toInt()
    val gutter = if (dark) 0xFF181A20.toInt() else 0xFFF7F8FA.toInt()
    val gutterText = if (dark) 0xFF5C6370.toInt() else 0xFFA0A6B1.toInt()
    val currentLine = if (dark) 0x14FFFFFF else 0x0A000000
    val selection = if (dark) 0x553D7EFF else 0x403D7EFF
    val error = if (dark) 0xFFFF6B6B.toInt() else 0xFFD93025.toInt()
    val warning = if (dark) 0xFFFFC14D.toInt() else 0xFFB7791F.toInt()
    val ok = if (dark) 0xFF4ADE80.toInt() else 0xFF15803D.toInt()

    private val keyword = if (dark) 0xFFFF8A4C.toInt() else 0xFFD4480B.toInt()
    private val string = if (dark) 0xFF98C379.toInt() else 0xFF2E7D32.toInt()
    private val number = if (dark) 0xFFD19A66.toInt() else 0xFF9C5D00.toInt()
    private val comment = if (dark) 0xFF6A7280.toInt() else 0xFF8A919C.toInt()
    private val builtin = if (dark) 0xFF56B6C2.toInt() else 0xFF00838F.toInt()
    private val func = if (dark) 0xFF61AFEF.toInt() else 0xFF1E5BC6.toInt()
    private val type = if (dark) 0xFFE5C07B.toInt() else 0xFF7A4BC2.toInt()
    private val operator = if (dark) 0xFFC8CCD4.toInt() else 0xFF4B5563.toInt()
    private val constant = if (dark) 0xFFC678DD.toInt() else 0xFFA21CAF.toInt()
    private val meta = if (dark) 0xFF8B95A5.toInt() else 0xFF6B7280.toInt()
    private val tag = if (dark) 0xFFE06C75.toInt() else 0xFFC2185B.toInt()
    private val attr = if (dark) 0xFFD19A66.toInt() else 0xFF9C5D00.toInt()

    fun token(kind: Int): Int =
        when (kind) {
            1 -> keyword
            2 -> string
            3 -> number
            4 -> comment
            5 -> builtin
            6 -> func
            7 -> type
            8 -> operator
            9 -> constant
            10 -> meta
            11 -> tag
            12 -> attr
            else -> text
        }

    companion object {
        fun of(ctx: Context) = Palette(Prefs.isDark(ctx))
    }
}

package ir.salamlang.app

object Engine {
    @Volatile
    private var loaded = false

    @Synchronized
    fun load() {
        if (!loaded) {
            System.loadLibrary("salam")
            loaded = true
        }
    }

    @JvmStatic external fun nativeBoot(stdRoot: String): Int

    @JvmStatic external fun nativeCli(args: String): Int

    @JvmStatic external fun nativeStop()

    @JvmStatic external fun nativeClearStop()

    @JvmStatic external fun nativeTokens(
        source: String,
        lang: String,
    ): String

    @JvmStatic external fun nativeCheck(
        source: String,
        path: String,
        lang: String,
    ): String

    @JvmStatic external fun nativeDetect(source: String): String

    @JvmStatic external fun nativeVersion(): String

    @JvmStatic external fun nativeRedirect(): IntArray?

    @JvmStatic external fun nativeChdir(dir: String): Int

    @JvmStatic external fun nativeDrainStdin()

    @JvmStatic external fun nativeMark(text: String)

    fun cli(vararg args: String): Int = nativeCli(args.joinToString("\n"))
}

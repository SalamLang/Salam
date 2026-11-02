package ir.salamlang.app

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.ViewGroup
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.LinearLayout
import java.io.File

class PreviewActivity : BaseActivity() {
    private var web: WebView? = null

    @SuppressLint("SetJavaScriptEnabled")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val f = File(intent.getStringExtra(EditorActivity.EXTRA_PATH) ?: "")
        val html =
            try {
                f.readText()
            } catch (_: Exception) {
                ""
            }
        val root =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.background)
            }
        val bar = topBar()
        bar.addView(iconButton(R.drawable.ic_back, R.string.back) { finish() })
        bar.addView(label(getString(R.string.preview_page), 16f, palette.text, true))
        root.addView(bar)
        val view =
            WebView(this).apply {
                settings.javaScriptEnabled = true
                settings.domStorageEnabled = true
                settings.allowFileAccess = false
                settings.allowContentAccess = false
                webViewClient = WebViewClient()
                loadDataWithBaseURL("https://preview.salamlang.ir/", html, "text/html", "utf-8", null)
            }
        web = view
        root.addView(view, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)
    }

    override fun onDestroy() {
        web?.destroy()
        web = null
        super.onDestroy()
    }
}

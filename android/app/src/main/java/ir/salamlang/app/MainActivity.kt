package ir.salamlang.app

import android.annotation.SuppressLint
import android.content.ActivityNotFoundException
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.Message
import android.util.Log
import android.view.View
import android.view.ViewGroup
import android.view.animation.DecelerateInterpolator
import android.webkit.ConsoleMessage
import android.webkit.CookieManager
import android.webkit.ValueCallback
import android.webkit.WebChromeClient
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ImageView
import android.widget.ProgressBar
import androidx.activity.ComponentActivity
import androidx.activity.OnBackPressedCallback
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.net.toUri
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.core.view.ViewCompat
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat

class MainActivity : ComponentActivity() {
    private lateinit var contentArea: FrameLayout
    private lateinit var webView: WebView
    private lateinit var topProgress: ProgressBar
    private lateinit var errorView: View
    private lateinit var retryButton: Button
    private lateinit var splashOverlay: View
    private lateinit var splashLogo: ImageView

    private val mainHandler = Handler(Looper.getMainLooper())
    private val splashTimeoutRunnable = Runnable { hideSplash() }
    private var splashHidden = false

    private var filePathCallback: ValueCallback<Array<Uri>>? = null

    private var popupWebView: WebView? = null

    private val fileChooserLauncher =
        registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
            val callback = filePathCallback ?: return@registerForActivityResult
            callback.onReceiveValue(
                WebChromeClient.FileChooserParams.parseResult(result.resultCode, result.data),
            )
            filePathCallback = null
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        installSplashScreen()
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContentView(R.layout.activity_main)

        bindViews()
        setupSystemBars()
        setupInsets()
        setupWebView()
        setupBackNavigation()
        animateSplashEntrance()
        retryButton.setOnClickListener { reload() }

        val restored =
            savedInstanceState != null && webView.restoreState(savedInstanceState) != null
        if (!restored) {
            webView.loadUrl(HOME_URL)
        }

        mainHandler.postDelayed(splashTimeoutRunnable, SPLASH_TIMEOUT_MS)
    }

    private fun bindViews() {
        contentArea = findViewById(R.id.contentArea)
        webView = findViewById(R.id.webView)
        topProgress = findViewById(R.id.topProgress)
        errorView = findViewById(R.id.errorView)
        retryButton = findViewById(R.id.retryButton)
        splashOverlay = findViewById(R.id.splashOverlay)
        splashLogo = findViewById(R.id.splashLogo)
    }

    private fun animateSplashEntrance() {
        splashLogo.alpha = 0f
        splashLogo.scaleX = 0.85f
        splashLogo.scaleY = 0.85f
        splashLogo
            .animate()
            .alpha(1f)
            .scaleX(1f)
            .scaleY(1f)
            .setStartDelay(60L)
            .setDuration(280L)
            .setInterpolator(DecelerateInterpolator())
            .start()
    }

    private fun setupSystemBars() {
        val controller = WindowCompat.getInsetsController(window, window.decorView)
        controller.isAppearanceLightStatusBars = false
        controller.isAppearanceLightNavigationBars = false
    }

    private fun setupInsets() {
        ViewCompat.setOnApplyWindowInsetsListener(contentArea) { v, insets ->
            val bars = insets.getInsets(WindowInsetsCompat.Type.systemBars())
            val ime = insets.getInsets(WindowInsetsCompat.Type.ime())
            v.setPadding(bars.left, bars.top, bars.right, maxOf(bars.bottom, ime.bottom))
            insets
        }
    }

    @SuppressLint("SetJavaScriptEnabled")
    private fun applyWebSettings(settings: WebSettings) {
        settings.apply {
            javaScriptEnabled = true
            domStorageEnabled = true
            javaScriptCanOpenWindowsAutomatically = true
            setSupportMultipleWindows(true)
            loadWithOverviewMode = true
            useWideViewPort = true
            cacheMode = WebSettings.LOAD_DEFAULT
            mediaPlaybackRequiresUserGesture = false

            mixedContentMode = WebSettings.MIXED_CONTENT_NEVER_ALLOW
            allowFileAccess = false
            allowContentAccess = false

            setSupportZoom(true)
            builtInZoomControls = false
            displayZoomControls = false
        }
    }

    private fun setupWebView() {
        if (0 != applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE) {
            WebView.setWebContentsDebuggingEnabled(true)
        }

        applyWebSettings(webView.settings)

        CookieManager.getInstance().apply {
            setAcceptCookie(true)
            setAcceptThirdPartyCookies(webView, true)
        }

        webView.webViewClient = SalamWebViewClient()
        webView.webChromeClient = SalamWebChromeClient()

        webView.setDownloadListener { url, _, _, _, _ ->
            openExternally(url.toUri())
        }
    }

    private fun openPopupWebView(): WebView {
        closePopup()
        val popup = WebView(this)
        applyWebSettings(popup.settings)
        CookieManager.getInstance().setAcceptThirdPartyCookies(popup, true)
        popup.webViewClient = SalamWebViewClient()
        popup.webChromeClient = SalamWebChromeClient()
        popup.layoutParams =
            FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT,
            )
        popup.setBackgroundColor(android.graphics.Color.WHITE)
        contentArea.addView(popup)
        popupWebView = popup
        return popup
    }

    private fun closePopup() {
        popupWebView?.let { popup ->
            contentArea.removeView(popup)
            popup.webChromeClient = null
            popup.destroy()
        }
        popupWebView = null
    }

    private fun setupBackNavigation() {
        onBackPressedDispatcher.addCallback(
            this,
            object : OnBackPressedCallback(true) {
                override fun handleOnBackPressed() {
                    val popup = popupWebView
                    when {
                        popup != null && popup.canGoBack() -> {
                            popup.goBack()
                        }

                        popup != null -> {
                            closePopup()
                        }

                        errorView.visibility != View.VISIBLE && webView.canGoBack() -> {
                            webView.goBack()
                        }

                        else -> {
                            isEnabled = false
                            onBackPressedDispatcher.onBackPressed()
                        }
                    }
                }
            },
        )
    }

    private fun hideSplash() {
        if (splashHidden) return
        splashHidden = true
        mainHandler.removeCallbacks(splashTimeoutRunnable)
        splashOverlay
            .animate()
            .alpha(0f)
            .setDuration(350L)
            .withEndAction { splashOverlay.visibility = View.GONE }
            .start()
    }

    private fun showError() {
        topProgress.visibility = View.GONE
        errorView.visibility = View.VISIBLE
        hideSplash()
    }

    private fun reload() {
        errorView.visibility = View.GONE
        if (webView.url == null) webView.loadUrl(HOME_URL) else webView.reload()
    }

    @SuppressLint("QueryPermissionsNeeded")
    private fun openExternally(uri: Uri) {
        try {
            val intent =
                if (uri.scheme?.lowercase() == "intent") {
                    val rawIntent = Intent.parseUri(uri.toString(), Intent.URI_INTENT_SCHEME)

                    rawIntent.action = Intent.ACTION_VIEW
                    rawIntent.addCategory(Intent.CATEGORY_BROWSABLE)
                    rawIntent.component = null
                    rawIntent.selector = null
                    rawIntent.flags = Intent.FLAG_ACTIVITY_NEW_TASK

                    val packageManager = packageManager
                    val activities =
                        packageManager.queryIntentActivities(
                            rawIntent,
                            PackageManager.MATCH_DEFAULT_ONLY,
                        )
                    if (activities.isEmpty()) {
                        Log.w(TAG, "No activity found for intent, ignoring")
                        return
                    }

                    val data = rawIntent.data
                    if (data != null) {
                        val scheme = data.scheme?.lowercase()
                        if (scheme != "http" && scheme != "https" && scheme != "tel" && scheme != "mailto") {
                            Log.w(TAG, "Unsupported scheme: $scheme, ignoring")
                            return
                        }
                    }

                    rawIntent
                } else {
                    Intent(Intent.ACTION_VIEW, uri).apply {
                        addCategory(Intent.CATEGORY_BROWSABLE)
                        component = null
                        selector = null
                    }
                }

            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(intent)
        } catch (e: Exception) {
            Log.e(TAG, "Failed to open external URI: $uri", e)
        }
    }

    override fun onPause() {
        webView.onPause()
        super.onPause()
    }

    override fun onResume() {
        super.onResume()
        webView.onResume()
    }

    override fun onSaveInstanceState(outState: Bundle) {
        super.onSaveInstanceState(outState)
        webView.saveState(outState)
    }

    override fun onDestroy() {
        mainHandler.removeCallbacks(splashTimeoutRunnable)
        closePopup()
        (webView.parent as? ViewGroup)?.removeView(webView)
        webView.destroy()
        super.onDestroy()
    }

    private inner class SalamWebViewClient : WebViewClient() {
        // Both spellings delegate here so the two can never drift apart.
        // The framework calls exactly one of them: the request-based
        // override only exists from API 24, so on 21-23 the deprecated
        // String form below is the only one that ever fires - without it
        // those releases would hand tel:/mailto:/intent: links to the
        // WebView itself, which cannot open them.
        private fun routeUrl(url: Uri): Boolean =
            when (url.scheme?.lowercase()) {
                "http", "https" -> false

                else -> {
                    openExternally(url)
                    true
                }
            }

        override fun shouldOverrideUrlLoading(
            view: WebView,
            request: WebResourceRequest,
        ): Boolean {
            Log.d(TAG, "shouldOverride: ${request.url} (mainFrame=${request.isForMainFrame})")
            return routeUrl(request.url)
        }

        @Deprecated("Superseded by the WebResourceRequest form on API 24+; kept for 21-23.")
        override fun shouldOverrideUrlLoading(
            view: WebView,
            url: String?,
        ): Boolean {
            Log.d(TAG, "shouldOverride (legacy): $url")
            if (url == null) return false
            return routeUrl(url.toUri())
        }

        override fun onPageFinished(
            view: WebView,
            url: String,
        ) {
            Log.d(TAG, "onPageFinished: $url")
            hideSplash()
        }

        override fun onReceivedError(
            view: WebView,
            request: WebResourceRequest,
            error: WebResourceError,
        ) {
            Log.w(
                TAG,
                "onReceivedError: ${request.url} code=${error.errorCode} '${error.description}' mainFrame=${request.isForMainFrame}",
            )
            if (request.isForMainFrame && view === webView) {
                showError()
            }
        }

        // The request-based form above starts at API 23, so on 21-22 this
        // is what reports a failed load. It has no isForMainFrame: the
        // older callback is only raised for the main frame anyway, which
        // is exactly the case that should show the retry screen.
        @Deprecated("Superseded by the WebResourceRequest form on API 23+; kept for 21-22.")
        override fun onReceivedError(
            view: WebView,
            errorCode: Int,
            description: String?,
            failingUrl: String?,
        ) {
            Log.w(TAG, "onReceivedError (legacy): $failingUrl code=$errorCode '$description'")
            if (view === webView) {
                showError()
            }
        }

        override fun onReceivedHttpError(
            view: WebView,
            request: WebResourceRequest,
            errorResponse: android.webkit.WebResourceResponse,
        ) {
            Log.w(TAG, "onReceivedHttpError: ${request.url} status=${errorResponse.statusCode}")
        }
    }

    private inner class SalamWebChromeClient : WebChromeClient() {
        override fun onProgressChanged(
            view: WebView,
            newProgress: Int,
        ) {
            topProgress.progress = newProgress
            topProgress.visibility = if (newProgress in 1..99) View.VISIBLE else View.GONE
            if (newProgress == 100) hideSplash()
        }

        override fun onConsoleMessage(message: ConsoleMessage): Boolean {
            Log.d(
                TAG,
                "console[${message.messageLevel()}]: ${message.message()} (${message.sourceId()}:${message.lineNumber()})",
            )
            return true
        }

        override fun onShowFileChooser(
            webView: WebView,
            callback: ValueCallback<Array<Uri>>,
            params: FileChooserParams,
        ): Boolean {
            filePathCallback?.onReceiveValue(null)
            filePathCallback = callback
            return try {
                fileChooserLauncher.launch(params.createIntent())
                true
            } catch (e: ActivityNotFoundException) {
                Log.w(TAG, "No activity found to handle file chooser intent", e)
                filePathCallback = null
                false
            }
        }

        override fun onCreateWindow(
            view: WebView,
            isDialog: Boolean,
            isUserGesture: Boolean,
            resultMsg: Message,
        ): Boolean {
            Log.d(TAG, "onCreateWindow: isUserGesture=$isUserGesture isDialog=$isDialog")
            val transport = resultMsg.obj as? WebView.WebViewTransport ?: return false
            val popup = openPopupWebView()
            transport.webView = popup
            resultMsg.sendToTarget()
            return true
        }

        override fun onCloseWindow(window: WebView) {
            closePopup()
        }
    }

    companion object {
        private const val TAG = "SalamWV"
        private const val HOME_URL = "https://salamlang.github.io/Salam"
        private const val SPLASH_TIMEOUT_MS = 12_000L
    }
}

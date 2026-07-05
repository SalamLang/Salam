/**
 * Hosts the Android app WebView experience and manages related UI integrations.
 */
package ir.salamlang.editor

import android.Manifest
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
import android.webkit.ConsoleMessage
import android.webkit.CookieManager
import android.webkit.GeolocationPermissions
import android.webkit.ValueCallback
import android.webkit.WebChromeClient
import android.webkit.WebResourceError
import android.webkit.WebResourceRequest
import android.webkit.WebSettings
import android.webkit.WebView
import android.webkit.WebViewClient
import android.widget.Button
import android.widget.FrameLayout
import android.widget.ProgressBar
import androidx.activity.OnBackPressedCallback
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import androidx.core.splashscreen.SplashScreen.Companion.installSplashScreen
import androidx.core.view.ViewCompat
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.net.toUri

/**
 * Main launcher activity for the Salam Android application.
 */
class MainActivity : AppCompatActivity() {

    private lateinit var contentArea: FrameLayout
    private lateinit var webView: WebView
    private lateinit var topProgress: ProgressBar
    private lateinit var errorView: View
    private lateinit var retryButton: Button
    private lateinit var splashOverlay: View

    private val mainHandler = Handler(Looper.getMainLooper())
    private val splashTimeoutRunnable = Runnable { hideSplash() }
    private var splashHidden = false

    private var geolocationOrigin: String? = null
    private var geolocationCallback: GeolocationPermissions.Callback? = null

    private var filePathCallback: ValueCallback<Array<Uri>>? = null

    private var popupWebView: WebView? = null

    private val locationPermissionLauncher =
        registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions()) { result ->
            val granted = result[Manifest.permission.ACCESS_FINE_LOCATION] == true ||
                result[Manifest.permission.ACCESS_COARSE_LOCATION] == true
            geolocationCallback?.invoke(geolocationOrigin.orEmpty(), granted, false)
            geolocationCallback = null
            geolocationOrigin = null
        }

    private val fileChooserLauncher =
        registerForActivityResult(ActivityResultContracts.StartActivityForResult()) { result ->
            val callback = filePathCallback ?: return@registerForActivityResult
            callback.onReceiveValue(
                WebChromeClient.FileChooserParams.parseResult(result.resultCode, result.data)
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
        retryButton.setOnClickListener { reload() }

        ensureLocationPermission()

        val restored = savedInstanceState != null && webView.restoreState(savedInstanceState) != null
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
            setGeolocationEnabled(true)
            javaScriptCanOpenWindowsAutomatically = true
            setSupportMultipleWindows(true)
            loadWithOverviewMode = true
            useWideViewPort = true
            cacheMode = WebSettings.LOAD_DEFAULT
            mediaPlaybackRequiresUserGesture = false
            mixedContentMode = WebSettings.MIXED_CONTENT_COMPATIBILITY_MODE
            allowFileAccess = true
            allowContentAccess = true
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
        popup.layoutParams = FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT
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
        onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
            override fun handleOnBackPressed() {
                val popup = popupWebView
                when {
                    popup != null && popup.canGoBack() -> popup.goBack()
                    popup != null -> closePopup()
                    errorView.visibility != View.VISIBLE && webView.canGoBack() -> webView.goBack()
                    else -> {
                        isEnabled = false
                        onBackPressedDispatcher.onBackPressed()
                    }
                }
            }
        })
    }

    private fun hasLocationPermission(): Boolean =
        ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) ==
            PackageManager.PERMISSION_GRANTED ||
            ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION) ==
            PackageManager.PERMISSION_GRANTED

    private fun ensureLocationPermission() {
        if (!hasLocationPermission()) {
            locationPermissionLauncher.launch(
                arrayOf(
                    Manifest.permission.ACCESS_FINE_LOCATION,
                    Manifest.permission.ACCESS_COARSE_LOCATION
                )
            )
        }
    }

    private fun hideSplash() {
        if (splashHidden) return
        splashHidden = true
        mainHandler.removeCallbacks(splashTimeoutRunnable)
        splashOverlay.animate()
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

    private val trustedGatewayHosts = listOf("zarinpal.com", "zibal.ir", "shaparak.ir")

    private fun isTrustedGatewayHost(host: String): Boolean =
        trustedGatewayHosts.any { host == it || host.endsWith(".$it") }

    private fun openExternally(uri: Uri) {
        try {
            val intent = if (uri.scheme == "intent") {
                Intent.parseUri(uri.toString(), Intent.URI_INTENT_SCHEME)
            } else {
                Intent(Intent.ACTION_VIEW, uri)
            }
            intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK)
            startActivity(intent)
        } catch (_: Exception) { }
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
        override fun shouldOverrideUrlLoading(
            view: WebView,
            request: WebResourceRequest
        ): Boolean {
            val url = request.url
            Log.d(TAG, "shouldOverride: $url (mainFrame=${request.isForMainFrame})")
            return when (url.scheme?.lowercase()) {
                "http", "https" -> false
                else -> {
                    openExternally(url)
                    true
                }
            }
        }

        override fun onPageFinished(view: WebView, url: String) {
            Log.d(TAG, "onPageFinished: $url")
            hideSplash()
        }

        override fun onReceivedError(
            view: WebView,
            request: WebResourceRequest,
            error: WebResourceError
        ) {
            Log.w(TAG, "onReceivedError: ${request.url} code=${error.errorCode} '${error.description}' mainFrame=${request.isForMainFrame}")
            if (request.isForMainFrame && view === webView) {
                showError()
            }
        }

        override fun onReceivedHttpError(
            view: WebView,
            request: WebResourceRequest,
            errorResponse: android.webkit.WebResourceResponse
        ) {
            Log.w(TAG, "onReceivedHttpError: ${request.url} status=${errorResponse.statusCode}")
        }

        @SuppressLint("WebViewClientOnReceivedSslError")
        override fun onReceivedSslError(
            view: WebView,
            handler: android.webkit.SslErrorHandler,
            error: android.net.http.SslError
        ) {
            val host = error.url.toUri().host.orEmpty()
            if (isTrustedGatewayHost(host)) {
                Log.w(TAG, "SSL bypass (trusted gateway) host=$host primaryError=${error.primaryError}")
                handler.proceed()
            } else {
                Log.w(TAG, "SSL error canceled host=$host primaryError=${error.primaryError}")
                handler.cancel()
            }
        }
    }

    private inner class SalamWebChromeClient : WebChromeClient() {
        override fun onProgressChanged(view: WebView, newProgress: Int) {
            topProgress.progress = newProgress
            topProgress.visibility = if (newProgress in 1..99) View.VISIBLE else View.GONE
            if (newProgress == 100) hideSplash()
        }

        override fun onConsoleMessage(message: ConsoleMessage): Boolean {
            Log.d(TAG, "console[${message.messageLevel()}]: ${message.message()} (${message.sourceId()}:${message.lineNumber()})")
            return true
        }

        override fun onGeolocationPermissionsShowPrompt(
            origin: String,
            callback: GeolocationPermissions.Callback
        ) {
            if (hasLocationPermission()) {
                callback.invoke(origin, true, false)
            } else {
                geolocationOrigin = origin
                geolocationCallback = callback
                locationPermissionLauncher.launch(
                    arrayOf(
                        Manifest.permission.ACCESS_FINE_LOCATION,
                        Manifest.permission.ACCESS_COARSE_LOCATION
                    )
                )
            }
        }

        override fun onShowFileChooser(
            webView: WebView,
            callback: ValueCallback<Array<Uri>>,
            params: FileChooserParams
        ): Boolean {
            filePathCallback?.onReceiveValue(null)
            filePathCallback = callback
            return try {
                fileChooserLauncher.launch(params.createIntent())
                true
            } catch (_: ActivityNotFoundException) {
                filePathCallback = null
                false
            }
        }

        override fun onCreateWindow(
            view: WebView,
            isDialog: Boolean,
            isUserGesture: Boolean,
            resultMsg: Message
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
        private const val HOME_URL = "https://salamlang.ir/"
        private const val SPLASH_TIMEOUT_MS = 12_000L
    }
}

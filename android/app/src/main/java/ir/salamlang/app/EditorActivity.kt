package ir.salamlang.app

import android.app.AlertDialog
import android.content.Intent
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.InputType
import android.util.TypedValue
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.inputmethod.InputMethodManager
import android.widget.EditText
import android.widget.LinearLayout
import android.widget.PopupMenu
import android.widget.TextView
import android.widget.Toast
import java.io.File

class EditorActivity :
    BaseActivity(),
    CodeEditor.Listener,
    RunClient.Listener,
    ConsoleView.Listener {
    private lateinit var file: File
    private lateinit var editor: CodeEditor
    private lateinit var console: ConsoleView
    private lateinit var symbols: SymbolBar
    private lateinit var title: TextView
    private lateinit var subtitle: TextView
    private lateinit var problems: TextView
    private lateinit var runButton: View
    private lateinit var findBar: LinearLayout
    private lateinit var findQuery: EditText
    private lateinit var replaceRow: LinearLayout
    private lateinit var replaceText: EditText
    private lateinit var run: RunClient

    private val handler = Handler(Looper.getMainLooper())
    private var lang = "en"
    private var dirty = false
    private var diagIndex = -1
    private var consoleHeight = 0
    private val jobs = HashMap<Int, (Int) -> Unit>()

    private val tokensTask = Runnable { requestTokens() }
    private val checkTask = Runnable { requestCheck() }
    private val saveTask = Runnable { save() }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val path = intent.getStringExtra(EXTRA_PATH)
        if (path == null) {
            finish()
            return
        }
        file = File(path)
        run = RunClient(this, this)
        buildUi()
        val text =
            try {
                file.readText()
            } catch (_: Exception) {
                ""
            }
        editor.load(text)
        updateTitle()
        Analyzer.detect(this, text) { detected ->
            setLang(if (detected == "fa") "fa" else "en")
            requestTokens()
            if (Prefs.liveCheck(this)) {
                requestCheck()
            }
        }
    }

    private fun buildUi() {
        val root =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.background)
            }

        val bar = topBar()
        bar.addView(iconButton(R.drawable.ic_back, R.string.back) { finish() })
        val titles =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(4), 0, dp(4), 0)
            }
        title = label("", 16f, palette.text, true).apply { setSingleLine(true) }
        subtitle = label("", 12f, palette.textDim).apply { setSingleLine(true) }
        titles.addView(title)
        titles.addView(subtitle)
        bar.addView(weight(titles))
        bar.addView(iconButton(R.drawable.ic_undo, R.string.undo) { editor.undo() })
        bar.addView(iconButton(R.drawable.ic_redo, R.string.redo) { editor.redo() })
        runButton =
            iconButton(R.drawable.ic_play, R.string.run, 0xFFFFFFFF.toInt()) { toggleRun() }.apply {
                background = ripple(palette.brand, 22f)
            }
        bar.addView(runButton)
        val more = iconButton(R.drawable.ic_more, R.string.more) { showMenu(it) }
        bar.addView(more)
        root.addView(bar)

        findBar = buildFindBar()
        root.addView(findBar)

        editor =
            CodeEditor(this, palette).apply {
                listener = this@EditorActivity
                tabWidth = Prefs.tabWidth(this@EditorActivity)
                wrap = Prefs.wrap(this@EditorActivity)
                showLineNumbers = Prefs.lineNumbers(this@EditorActivity)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, Prefs.fontSize(this@EditorActivity))
                setPadding(paddingLeft, dp(8), dp(8), dp(24))
            }
        root.addView(editor, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))

        problems =
            label("", 13f, palette.textDim).apply {
                setSingleLine(true)
                setPadding(dp(12), dp(6), dp(12), dp(6))
                setBackgroundColor(palette.surface)
                setOnClickListener { nextProblem() }
                textDirection = View.TEXT_DIRECTION_LOCALE
            }
        root.addView(problems, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        console =
            ConsoleView(this).apply {
                listener = this@EditorActivity
                visibility = View.GONE
            }
        consoleHeight = (resources.displayMetrics.heightPixels * 0.36f).toInt()
        root.addView(console, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, consoleHeight))

        symbols = SymbolBar(this, editor)
        root.addView(symbols, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))

        setContentView(root)
    }

    private fun buildFindBar(): LinearLayout {
        val box =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.surface)
                setPadding(dp(8), 0, dp(4), dp(6))
                visibility = View.GONE
            }
        fun field(hint: Int): EditText =
            EditText(this).apply {
                setHint(hint)
                setSingleLine(true)
                setTextSize(TypedValue.COMPLEX_UNIT_SP, 14f)
                setTextColor(palette.text)
                setHintTextColor(palette.textDim)
                background = rounded(palette.background, 16f, palette.border)
                setPadding(dp(12), dp(7), dp(12), dp(7))
                inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS
            }
        val row1 =
            LinearLayout(this).apply {
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
            }
        findQuery = field(R.string.find_hint)
        findQuery.setOnEditorActionListener { _, _, _ ->
            if (!editor.find(findQuery.text.toString(), true)) toast(R.string.not_found)
            true
        }
        row1.addView(weight(findQuery))
        row1.addView(iconButton(R.drawable.ic_up, R.string.previous) { if (!editor.find(findQuery.text.toString(), false)) toast(R.string.not_found) })
        row1.addView(iconButton(R.drawable.ic_down, R.string.next) { if (!editor.find(findQuery.text.toString(), true)) toast(R.string.not_found) })
        row1.addView(
            iconButton(R.drawable.ic_more, R.string.replace) {
                replaceRow.visibility = if (replaceRow.visibility == View.VISIBLE) View.GONE else View.VISIBLE
            },
        )
        row1.addView(iconButton(R.drawable.ic_close, R.string.close) { box.visibility = View.GONE })
        box.addView(row1)
        replaceRow =
            LinearLayout(this).apply {
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                visibility = View.GONE
                setPadding(0, dp(4), dp(4), 0)
            }
        replaceText = field(R.string.replace_hint)
        replaceRow.addView(weight(replaceText))
        replaceRow.addView(
            pillButton(getString(R.string.replace), false) {
                editor.replaceCurrent(findQuery.text.toString(), replaceText.text.toString())
            }.apply { (layoutParams as? LinearLayout.LayoutParams)?.marginStart = dp(6) },
        )
        val all =
            pillButton(getString(R.string.replace_all), false) {
                val n = editor.replaceAll(findQuery.text.toString(), replaceText.text.toString())
                Toast.makeText(this, getString(R.string.replaced_n, n), Toast.LENGTH_SHORT).show()
            }
        replaceRow.addView(all)
        (all.layoutParams as? LinearLayout.LayoutParams)?.marginStart = dp(6)
        box.addView(replaceRow)
        return box
    }

    private fun setLang(code: String) {
        lang = code
        editor.lang = code
        symbols.setLang(code)
        updateTitle()
    }

    private fun updateTitle() {
        title.text = (if (dirty) "• " else "") + file.name
        subtitle.text = getString(if (lang == "fa") R.string.lang_fa else R.string.lang_en)
    }

    override fun onEdited() {
        if (!dirty) {
            dirty = true
            updateTitle()
        }
        diagIndex = -1
        handler.removeCallbacks(tokensTask)
        handler.postDelayed(tokensTask, 90)
        handler.removeCallbacks(checkTask)
        if (Prefs.liveCheck(this)) {
            handler.postDelayed(checkTask, 800)
        }
        handler.removeCallbacks(saveTask)
        handler.postDelayed(saveTask, 2000)
    }

    override fun onZoom(size: Float) {
        Prefs.setFontSize(this, size)
    }

    private fun requestTokens() {
        val v = editor.version
        Analyzer.tokens(this, editor.text.toString(), lang) { editor.setTokens(it, v) }
    }

    private fun requestCheck() {
        val v = editor.version
        val source = editor.text.toString()
        Analyzer.check(this, source, file.absolutePath, lang) { list ->
            if (v == editor.version) {
                editor.diagnostics = list
                showProblems(list)
            }
        }
    }

    private fun showProblems(list: List<Diagnostic>) {
        if (list.isEmpty()) {
            problems.text = getString(R.string.no_problems)
            problems.setTextColor(palette.ok)
            return
        }
        val errors = list.count { it.error }
        val warnings = list.size - errors
        val first = list.firstOrNull { it.error } ?: list[0]
        val head =
            buildString {
                if (errors > 0) append(getString(R.string.n_errors, errors))
                if (warnings > 0) {
                    if (isNotEmpty()) append(" · ")
                    append(getString(R.string.n_warnings, warnings))
                }
            }
        problems.text = getString(R.string.problem_line, head, first.line, first.message)
        problems.setTextColor(if (errors > 0) palette.error else palette.warning)
    }

    private fun nextProblem() {
        val list = editor.diagnostics
        if (list.isEmpty()) {
            return
        }
        diagIndex = (diagIndex + 1) % list.size
        val d = list[diagIndex]
        editor.reveal(d.start, d.end)
        editor.requestFocus()
        problems.text = getString(R.string.problem_at, diagIndex + 1, list.size, d.line, d.message)
    }

    private fun save(): Boolean {
        handler.removeCallbacks(saveTask)
        if (!dirty) {
            return true
        }
        return try {
            file.writeText(editor.text.toString())
            dirty = false
            updateTitle()
            true
        } catch (_: Exception) {
            toast(R.string.save_failed)
            false
        }
    }

    override fun onPause() {
        super.onPause()
        save()
    }

    override fun onStart() {
        super.onStart()
        run.bind()
    }

    override fun onDestroy() {
        handler.removeCallbacksAndMessages(null)
        if (::run.isInitialized) {
            run.unbind()
        }
        super.onDestroy()
    }

    private fun toggleRun() {
        if (run.isRunning) {
            run.stop()
        } else {
            startJob(listOf("exec", file.absolutePath), file.parentFile ?: filesDir, true, null)
        }
    }

    private fun startJob(
        args: List<String>,
        cwd: File,
        interactive: Boolean,
        done: ((Int) -> Unit)?,
    ) {
        if (run.isRunning) {
            toast(R.string.busy)
            return
        }
        save()
        hideKeyboard()
        console.visibility = View.VISIBLE
        console.clear()
        console.setRunning(interactive)
        val token = run.run(args, cwd.absolutePath)
        if (done != null) {
            jobs[token] = done
        }
        if (interactive) {
            setRunIcon(true)
        }
    }

    private fun setRunIcon(running: Boolean) {
        (runButton as? android.widget.ImageButton)?.setImageResource(if (running) R.drawable.ic_stop else R.drawable.ic_play)
        runButton.background = ripple(if (running) palette.error else palette.brand, 22f)
        runButton.contentDescription = getString(if (running) R.string.stop else R.string.run)
    }

    override fun onOutput(
        text: String,
        error: Boolean,
    ) {
        console.append(text, if (error) palette.error else palette.text)
    }

    override fun onDone(
        token: Int,
        rc: Int,
        ms: Long,
    ) {
        console.setRunning(false)
        setRunIcon(false)
        when (rc) {
            RunClient.RC_STOPPED -> console.setStatus(getString(R.string.stopped), palette.warning)
            0 -> console.setStatus(getString(R.string.finished, formatMs(ms)), palette.ok)
            else -> console.setStatus(getString(R.string.exit_code, rc, formatMs(ms)), palette.error)
        }
        jobs.remove(token)?.invoke(rc)
    }

    private fun formatMs(ms: Long): String = if (ms < 1000) "$ms ms" else String.format(java.util.Locale.US, "%.2f s", ms / 1000.0)

    override fun onInput(line: String) {
        run.input(line)
    }

    override fun onStop() {
        run.stop()
    }

    override fun onClose() {
        if (run.isRunning) {
            run.stop()
        }
        console.visibility = View.GONE
    }

    override fun onResize(dy: Float) {
        val limit = (findViewById<View>(android.R.id.content).height * 0.8f).toInt().coerceAtLeast(dp(120))
        consoleHeight = (consoleHeight + dy.toInt()).coerceIn(dp(90), limit)
        console.layoutParams = console.layoutParams.apply { height = consoleHeight }
    }

    private fun showMenu(anchor: View) {
        val menu = PopupMenu(this, anchor)
        val m = menu.menu
        m.add(0, 1, 0, R.string.save)
        m.add(0, 2, 0, R.string.format)
        m.add(0, 3, 0, if (lang == "fa") R.string.translate_to_en else R.string.translate_to_fa)
        m.add(0, 4, 0, R.string.find_replace)
        m.add(0, 5, 0, R.string.go_to_line)
        m.add(0, 6, 0, R.string.check_now)
        m.add(0, 7, 0, R.string.preview_page)
        val inspect = m.addSubMenu(0, 8, 0, R.string.inspect)
        inspect.add(0, 20, 0, R.string.inspect_tokens)
        inspect.add(0, 21, 0, R.string.inspect_ast)
        inspect.add(0, 22, 0, R.string.inspect_symbols)
        inspect.add(0, 23, 0, R.string.inspect_js)
        inspect.add(0, 24, 0, R.string.inspect_llvm)
        inspect.add(0, 25, 0, R.string.inspect_c)
        m.add(0, 9, 0, R.string.share)
        m.add(0, 10, 0, R.string.console)
        m.add(0, 11, 0, R.string.settings)
        menu.setOnMenuItemClickListener {
            when (it.itemId) {
                1 -> if (save()) toast(R.string.saved)
                2 -> formatFile()
                3 -> translateFile()
                4 -> {
                    findBar.visibility = View.VISIBLE
                    findQuery.requestFocus()
                }
                5 -> askGoToLine()
                6 -> requestCheck()
                7 -> previewPage()
                9 -> shareFile()
                10 -> console.visibility = View.VISIBLE
                11 -> startActivity(Intent(this, SettingsActivity::class.java))
                20 -> inspect("--emit-tokens", "xml", R.string.inspect_tokens)
                21 -> inspect("--emit-ast", "xml", R.string.inspect_ast)
                22 -> inspect("--emit-symbol", "xml", R.string.inspect_symbols)
                23 -> emitTo("js", "js", R.string.inspect_js)
                24 -> emitTo("llvm", "ll", R.string.inspect_llvm)
                25 -> emitC()
            }
            true
        }
        menu.show()
    }

    private fun replaceAllText(text: String) {
        val e = editor.text ?: return
        val caret = editor.selectionStart.coerceAtMost(text.length)
        e.replace(0, e.length, text)
        editor.setSelection(caret.coerceIn(0, e.length))
    }

    private fun formatFile() {
        startJob(listOf("format", file.absolutePath), file.parentFile ?: filesDir, false) { rc ->
            if (rc == 0) {
                val text = file.readText()
                if (text != editor.text.toString()) {
                    replaceAllText(text)
                    save()
                }
                console.visibility = View.GONE
                toast(R.string.formatted)
            }
        }
    }

    private fun translateFile() {
        val target = if (lang == "fa") "en" else "fa"
        val out = File(Workspace.cache(this), "translated.salam")
        out.delete()
        startJob(
            listOf("translate", target, file.absolutePath, "--output=" + out.absolutePath),
            file.parentFile ?: filesDir,
            false,
        ) { rc ->
            if (rc == 0 && out.isFile) {
                replaceAllText(out.readText())
                setLang(target)
                save()
                console.visibility = View.GONE
                requestTokens()
                requestCheck()
            }
        }
    }

    private fun inspect(
        flag: String,
        ext: String,
        titleRes: Int,
    ) {
        val out = File(Workspace.cache(this), "inspect.$ext")
        out.delete()
        startJob(listOf("inspect", file.absolutePath, flag, "--xml-out=" + out.absolutePath), file.parentFile ?: filesDir, false) { rc ->
            if (rc == 0 && out.isFile) {
                openViewer(out, getString(titleRes))
            }
        }
    }

    private fun emitTo(
        command: String,
        ext: String,
        titleRes: Int,
    ) {
        val out = File(Workspace.cache(this), "emit.$ext")
        out.delete()
        startJob(listOf(command, file.absolutePath, "--output=" + out.absolutePath), file.parentFile ?: filesDir, false) { rc ->
            if (rc == 0 && out.isFile) {
                openViewer(out, getString(titleRes))
            }
        }
    }

    private fun emitC() {
        val work = File(Workspace.cache(this), "c")
        work.deleteRecursively()
        work.mkdirs()
        startJob(
            listOf("build", "--backend=c", "--emit-c", file.absolutePath, "--output=" + File(work, "program").absolutePath),
            work,
            false,
        ) { rc ->
            val gen = File(work, ".salam-build")
            val main = File(gen, "salam_mod_main.c").takeIf { it.isFile } ?: gen.listFiles { f -> f.name.endsWith(".c") }?.firstOrNull()
            if (rc == 0 && main != null) {
                openViewer(main, getString(R.string.inspect_c))
            }
        }
    }

    private fun previewPage() {
        val out = File(Workspace.cache(this), "preview.html")
        out.delete()
        startJob(
            listOf("layout", "build", file.absolutePath, "--inline", "--output=" + out.absolutePath),
            file.parentFile ?: filesDir,
            false,
        ) { rc ->
            if (rc == 0 && out.isFile) {
                console.visibility = View.GONE
                startActivity(Intent(this, PreviewActivity::class.java).putExtra(EXTRA_PATH, out.absolutePath))
            }
        }
    }

    private fun openViewer(
        f: File,
        heading: String,
    ) {
        console.visibility = View.GONE
        startActivity(
            Intent(this, ViewerActivity::class.java)
                .putExtra(EXTRA_PATH, f.absolutePath)
                .putExtra(ViewerActivity.EXTRA_TITLE, heading),
        )
    }

    private fun askGoToLine() {
        val input =
            EditText(this).apply {
                inputType = InputType.TYPE_CLASS_NUMBER
                hint = "1 - ${editor.lineCount()}"
            }
        val box = LinearLayout(this).apply { setPadding(dp(20), dp(8), dp(20), 0) }
        box.addView(input, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        AlertDialog
            .Builder(this)
            .setTitle(R.string.go_to_line)
            .setView(box)
            .setPositiveButton(R.string.go) { _, _ ->
                input.text.toString().toIntOrNull()?.let {
                    editor.goToLine(it)
                    editor.requestFocus()
                }
            }.setNegativeButton(R.string.cancel, null)
            .show()
    }

    private fun shareFile() {
        save()
        val send =
            Intent(Intent.ACTION_SEND)
                .setType("text/plain")
                .putExtra(Intent.EXTRA_SUBJECT, file.name)
                .putExtra(Intent.EXTRA_TEXT, editor.text.toString())
        startActivity(Intent.createChooser(send, getString(R.string.share)))
    }

    private fun hideKeyboard() {
        val imm = getSystemService(INPUT_METHOD_SERVICE) as InputMethodManager
        imm.hideSoftInputFromWindow(editor.windowToken, 0)
    }

    private fun toast(res: Int) = Toast.makeText(this, res, Toast.LENGTH_SHORT).show()

    companion object {
        const val EXTRA_PATH = "path"

        fun open(
            ctx: android.content.Context,
            f: File,
        ) {
            ctx.startActivity(Intent(ctx, EditorActivity::class.java).putExtra(EXTRA_PATH, f.absolutePath))
        }
    }
}

package ir.salamlang.app

import android.app.AlertDialog
import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.provider.OpenableColumns
import android.text.format.DateUtils
import android.text.format.Formatter
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.EditText
import android.widget.ImageView
import android.widget.LinearLayout
import android.widget.ListView
import android.widget.ProgressBar
import android.widget.RadioButton
import android.widget.RadioGroup
import android.widget.TextView
import android.widget.Toast
import java.io.File

class MainActivity : BaseActivity() {
    private lateinit var list: ListView
    private lateinit var empty: View
    private lateinit var loading: View
    private lateinit var version: TextView
    private var files: List<File> = emptyList()
    private var exportSource: File? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        buildUi()
        if (!StdInstaller.isReady(this)) {
            loading.visibility = View.VISIBLE
        }
        Analyzer.whenReady(this) {
            loading.visibility = View.GONE
        }
        Analyzer.version(this) { version.text = getString(R.string.version_line, it.trim()) }
        handleShare(intent)
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        handleShare(intent)
    }

    override fun onResume() {
        super.onResume()
        refresh()
    }

    private fun buildUi() {
        val root =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setBackgroundColor(palette.background)
            }
        val header =
            LinearLayout(this).apply {
                orientation = LinearLayout.HORIZONTAL
                gravity = Gravity.CENTER_VERTICAL
                setPadding(dp(16), dp(14), dp(6), dp(6))
            }
        val logo =
            ImageView(this).apply {
                setImageResource(R.drawable.logo)
                layoutParams = LinearLayout.LayoutParams(dp(40), dp(40))
            }
        header.addView(logo)
        val titles =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(12), 0, dp(8), 0)
            }
        titles.addView(label(getString(R.string.app_name), 22f, palette.text, true))
        version = label(getString(R.string.tagline), 13f, palette.textDim)
        titles.addView(version)
        header.addView(weight(titles))
        header.addView(iconButton(R.drawable.ic_settings, R.string.settings) { startActivity(Intent(this, SettingsActivity::class.java)) })
        root.addView(header)

        val actions =
            LinearLayout(this).apply {
                orientation = LinearLayout.HORIZONTAL
                setPadding(dp(16), dp(8), dp(16), dp(12))
            }
        val newButton = pillButton("＋ " + getString(R.string.new_file), true) { askNewFile() }
        actions.addView(newButton)
        val importButton = pillButton(getString(R.string.import_file), false) { pickImport() }
        actions.addView(importButton)
        (importButton.layoutParams as? LinearLayout.LayoutParams)?.marginStart = dp(8)
        val examplesButton = pillButton(getString(R.string.examples), false) { showExamples() }
        actions.addView(examplesButton)
        (examplesButton.layoutParams as? LinearLayout.LayoutParams)?.marginStart = dp(8)
        root.addView(actions)

        root.addView(
            label(getString(R.string.your_files), 13f, palette.textDim, true).apply {
                setPadding(dp(20), dp(4), dp(20), dp(6))
            },
        )

        val body = android.widget.FrameLayout(this)
        list =
            ListView(this).apply {
                divider = null
                selector = android.graphics.drawable.ColorDrawable(0)
                setPadding(dp(8), 0, dp(8), dp(16))
                clipToPadding = false
                adapter = FilesAdapter()
                setOnItemClickListener { _, _, pos, _ -> EditorActivity.open(this@MainActivity, files[pos]) }
                setOnItemLongClickListener { _, _, pos, _ ->
                    fileOptions(files[pos])
                    true
                }
            }
        body.addView(list)
        empty =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                gravity = Gravity.CENTER
                setPadding(dp(32), dp(32), dp(32), dp(32))
                addView(label(getString(R.string.empty_title), 17f, palette.text, true).apply { gravity = Gravity.CENTER })
                addView(label(getString(R.string.empty_hint), 14f, palette.textDim).apply { gravity = Gravity.CENTER })
            }
        body.addView(empty)
        loading =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                gravity = Gravity.CENTER
                setBackgroundColor(palette.background)
                isClickable = true
                visibility = View.GONE
                addView(ProgressBar(this@MainActivity))
                addView(label(getString(R.string.preparing), 14f, palette.textDim).apply { setPadding(0, dp(12), 0, 0) })
            }
        body.addView(loading)
        root.addView(body, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f))
        setContentView(root)
    }

    private fun refresh() {
        files = Workspace.files(this)
        (list.adapter as BaseAdapter).notifyDataSetChanged()
        empty.visibility = if (files.isEmpty()) View.VISIBLE else View.GONE
    }

    private inner class FilesAdapter : BaseAdapter() {
        override fun getCount() = files.size

        override fun getItem(position: Int) = files[position]

        override fun getItemId(position: Int) = position.toLong()

        override fun getView(
            position: Int,
            convertView: View?,
            parent: ViewGroup?,
        ): View {
            val item = (convertView as? LinearLayout) ?: createRow()
            val f = files[position]
            item.findViewWithTag<TextView>("name").text = f.name
            item.findViewWithTag<TextView>("meta").text =
                DateUtils.getRelativeTimeSpanString(f.lastModified()).toString() + " · " +
                Formatter.formatShortFileSize(this@MainActivity, f.length())
            return item
        }

        private fun createRow(): LinearLayout {
            val row =
                LinearLayout(this@MainActivity).apply {
                    orientation = LinearLayout.HORIZONTAL
                    gravity = Gravity.CENTER_VERTICAL
                    setPadding(dp(14), dp(12), dp(14), dp(12))
                    background = ripple(palette.surface, 14f)
                    isDuplicateParentStateEnabled = true
                }
            val icon =
                ImageView(this@MainActivity).apply {
                    setImageResource(R.drawable.ic_file)
                    imageTintList = android.content.res.ColorStateList.valueOf(palette.brand)
                }
            row.addView(icon, LinearLayout.LayoutParams(dp(24), dp(24)))
            val texts =
                LinearLayout(this@MainActivity).apply {
                    orientation = LinearLayout.VERTICAL
                    setPadding(dp(14), 0, 0, 0)
                }
            texts.addView(label("", 15f, palette.text, true).apply { tag = "name" })
            texts.addView(label("", 12f, palette.textDim).apply { tag = "meta" })
            row.addView(weight(texts))
            return LinearLayout(this@MainActivity).apply {
                setPadding(0, dp(3), 0, dp(3))
                addView(row, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
            }
        }
    }

    private fun askNewFile(
        initialName: String = "",
        content: String? = null,
    ) {
        val box =
            LinearLayout(this).apply {
                orientation = LinearLayout.VERTICAL
                setPadding(dp(20), dp(8), dp(20), 0)
            }
        val name =
            EditText(this).apply {
                hint = getString(R.string.file_name_hint)
                setSingleLine(true)
                setText(initialName)
            }
        box.addView(name)
        val group = RadioGroup(this).apply { orientation = RadioGroup.HORIZONTAL }
        val fa =
            RadioButton(this).apply {
                id = View.generateViewId()
                text = getString(R.string.lang_fa)
            }
        val en =
            RadioButton(this).apply {
                id = View.generateViewId()
                text = getString(R.string.lang_en)
            }
        group.addView(fa)
        group.addView(en)
        val last = Prefs.newFileLang(this).ifEmpty { Prefs.effectiveUiLang(this) }
        group.check(if (last == "fa") fa.id else en.id)
        if (content == null) {
            box.addView(group)
        }
        AlertDialog
            .Builder(this)
            .setTitle(R.string.new_file)
            .setView(box)
            .setPositiveButton(R.string.create) { _, _ ->
                val lang = if (group.checkedRadioButtonId == fa.id) "fa" else "en"
                Prefs.setNewFileLang(this, lang)
                val f = Workspace.unique(this, name.text.toString().ifBlank { "main" })
                f.writeText(content ?: Workspace.template(lang))
                EditorActivity.open(this, f)
            }.setNegativeButton(R.string.cancel, null)
            .show()
    }

    private fun showExamples() {
        val all = Workspace.examples(this)
        if (all.isEmpty()) {
            return
        }
        val fa = Prefs.effectiveUiLang(this) == "fa"
        val titles = all.map { if (fa) it.titleFa else it.titleEn }.toTypedArray()
        AlertDialog
            .Builder(this)
            .setTitle(R.string.examples)
            .setItems(titles) { _, which ->
                val ex = all[which]
                val lang = if (fa) "fa" else "en"
                val src = Workspace.exampleSource(this, ex.id, lang) ?: Workspace.exampleSource(this, ex.id, "en") ?: return@setItems
                val f = Workspace.unique(this, ex.id)
                f.writeText(src)
                EditorActivity.open(this, f)
            }.show()
    }

    private fun fileOptions(f: File) {
        val items =
            arrayOf(
                getString(R.string.rename),
                getString(R.string.duplicate),
                getString(R.string.export),
                getString(R.string.share),
                getString(R.string.delete),
            )
        AlertDialog
            .Builder(this)
            .setTitle(f.name)
            .setItems(items) { _, which ->
                when (which) {
                    0 -> rename(f)
                    1 -> {
                        Workspace.unique(this, f.name).writeText(f.readText())
                        refresh()
                    }
                    2 -> export(f)
                    3 ->
                        startActivity(
                            Intent.createChooser(
                                Intent(Intent.ACTION_SEND)
                                    .setType("text/plain")
                                    .putExtra(Intent.EXTRA_SUBJECT, f.name)
                                    .putExtra(Intent.EXTRA_TEXT, f.readText()),
                                getString(R.string.share),
                            ),
                        )
                    4 ->
                        AlertDialog
                            .Builder(this)
                            .setMessage(getString(R.string.delete_confirm, f.name))
                            .setPositiveButton(R.string.delete) { _, _ ->
                                f.delete()
                                refresh()
                            }.setNegativeButton(R.string.cancel, null)
                            .show()
                }
            }.show()
    }

    private fun rename(f: File) {
        val input =
            EditText(this).apply {
                setSingleLine(true)
                setText(f.name)
            }
        val box = LinearLayout(this).apply { setPadding(dp(20), dp(8), dp(20), 0) }
        box.addView(input, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        AlertDialog
            .Builder(this)
            .setTitle(R.string.rename)
            .setView(box)
            .setPositiveButton(R.string.ok) { _, _ ->
                val target = File(Workspace.dir(this), Workspace.cleanName(input.text.toString()))
                if (target.exists() && target != f) {
                    Toast.makeText(this, R.string.name_taken, Toast.LENGTH_SHORT).show()
                } else {
                    f.renameTo(target)
                    refresh()
                }
            }.setNegativeButton(R.string.cancel, null)
            .show()
    }

    private fun pickImport() {
        val pick =
            Intent(Intent.ACTION_OPEN_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .setType("*/*")
        try {
            startActivityForResult(pick, REQ_IMPORT)
        } catch (_: Exception) {
            Toast.makeText(this, R.string.no_picker, Toast.LENGTH_SHORT).show()
        }
    }

    private fun export(f: File) {
        exportSource = f
        val create =
            Intent(Intent.ACTION_CREATE_DOCUMENT)
                .addCategory(Intent.CATEGORY_OPENABLE)
                .setType("text/plain")
                .putExtra(Intent.EXTRA_TITLE, f.name)
        try {
            startActivityForResult(create, REQ_EXPORT)
        } catch (_: Exception) {
            Toast.makeText(this, R.string.no_picker, Toast.LENGTH_SHORT).show()
        }
    }

    @Deprecated("framework Activity result API")
    override fun onActivityResult(
        requestCode: Int,
        resultCode: Int,
        data: Intent?,
    ) {
        super.onActivityResult(requestCode, resultCode, data)
        val uri = data?.data ?: return
        if (resultCode != RESULT_OK) {
            return
        }
        try {
            when (requestCode) {
                REQ_IMPORT -> {
                    val text = contentResolver.openInputStream(uri)?.use { it.readBytes().decodeToString() } ?: return
                    val f = Workspace.unique(this, displayName(uri) ?: "imported.salam")
                    f.writeText(text)
                    EditorActivity.open(this, f)
                }
                REQ_EXPORT -> {
                    val src = exportSource ?: return
                    contentResolver.openOutputStream(uri)?.use { it.write(src.readBytes()) }
                    Toast.makeText(this, R.string.exported, Toast.LENGTH_SHORT).show()
                }
            }
        } catch (_: Exception) {
            Toast.makeText(this, R.string.io_failed, Toast.LENGTH_SHORT).show()
        }
    }

    private fun displayName(uri: Uri): String? =
        try {
            contentResolver.query(uri, arrayOf(OpenableColumns.DISPLAY_NAME), null, null, null)?.use {
                if (it.moveToFirst()) it.getString(0) else null
            }
        } catch (_: Exception) {
            null
        }

    private fun handleShare(intent: Intent?) {
        if (intent?.action != Intent.ACTION_SEND) {
            return
        }
        val text = intent.getStringExtra(Intent.EXTRA_TEXT) ?: return
        intent.action = null
        askNewFile(intent.getStringExtra(Intent.EXTRA_SUBJECT) ?: "shared", text)
    }

    companion object {
        private const val REQ_IMPORT = 10
        private const val REQ_EXPORT = 11
    }
}

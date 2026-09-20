# Salam for JetBrains IDEs

Works in IntelliJ IDEA, PyCharm, WebStorm, CLion, GoLand, Rider, PhpStorm,
RubyMine and Android Studio.

## Option 1: the custom file type (no plugin)

`Settings > Editor > File Types > Import...` and pick `Salam.xml`.

Or copy it in directly and restart the IDE:

| Platform | Path                                                                         |
| -------- | ---------------------------------------------------------------------------- |
| Linux    | `~/.config/JetBrains/<IDE><version>/filetypes/Salam.xml`                     |
| macOS    | `~/Library/Application Support/JetBrains/<IDE><version>/filetypes/Salam.xml` |
| Windows  | `%APPDATA%\JetBrains\<IDE><version>\filetypes\Salam.xml`                     |

This gives keyword, type, built-in and constant highlighting in all three
languages, `//` and `/* */` comment toggling, brace matching, string and
number literals, and basic completion from words in the file.

## Option 2: the TextMate bundle (richer highlighting)

Install the bundled **TextMate Bundles** plugin, then
`Settings > Editor > TextMate Bundles > +` and select the `extensions/vscode`
directory of this repository. It uses the same TextMate grammar Visual Studio Code does,
so declaration names, `//!` directive headers, layout-DSL elements and escape
sequences are highlighted too.

The two options can be used together: keep the TextMate bundle for colouring
and the file type for comment toggling.

## Generated

`Salam.xml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).

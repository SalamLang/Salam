# Salam for Notepad++

Syntax highlighting (a User Defined Language), autocompletion, the Function
List panel, and Run menu commands for `salam`.

| File                          | What it is                                        |
| ----------------------------- | ------------------------------------------------- |
| `salam.xml`                   | Highlighting for the default (light) theme        |
| `salam-dark.xml`              | The same language named "Salam Dark", dark colours |
| `autoCompletion/Salam.xml`    | Autocompletion list (generated from the keywords) |
| `functionList/salam.xml`      | Function List rules for func, struct, enum, interface, component and page |
| `shortcuts-fragment.xml`      | Run menu commands with shortcuts                  |
| `nppexec/salam.nppexec`       | The same commands as NppExec scripts              |
| `install.ps1`                 | Copies the files into place                 |

## Install

Close Notepad++ first. In PowerShell, from this folder:

```powershell
.\install.ps1
```

That copies the files to the places below. To do it by hand, copy under
`%APPDATA%\Notepad++`:

```
userDefineLangs\salam.xml
userDefineLangs\salam-dark.xml
autoCompletion\Salam.xml
autoCompletion\Salam Dark.xml        (a copy of Salam.xml)
functionList\salam.xml
```

Two steps need a hand edit because the files already exist on most installs.

### Function List

Add the associations to `%APPDATA%\Notepad++\functionList\overrideMap.xml`
(create the file with this content when it is missing):

```xml
<NotepadPlus>
    <functionList>
        <associationMap>
            <association id="salam.xml" userDefinedLangName="Salam" />
            <association id="salam.xml" userDefinedLangName="Salam Dark" />
        </associationMap>
    </functionList>
</NotepadPlus>
```

Then `View > Function List` shows entries such as `func main`, `struct Box`,
`component Counter` and `page Home`, also for the Persian keywords.

### Run menu commands

With Notepad++ closed, copy the `<Command>` lines from `shortcuts-fragment.xml`
into the `<UserDefinedCommands>` element of `%APPDATA%\Notepad++\shortcuts.xml`
(create it next to the other `<Commands>` sections if it is missing). They then
appear in the Run menu:

| Command                    | Shortcut     | Runs                                   |
| -------------------------- | ------------ | -------------------------------------- |
| Salam: Run                 | Ctrl+F5      | `salam run FILE`                       |
| Salam: Build               | Ctrl+F6      | `salam build FILE`                     |
| Salam: Check               | Ctrl+F7      | `salam check FILE`                     |
| Salam: Format              | Ctrl+F8      | `salam format FILE`                    |
| Salam: Build for web       | Ctrl+F9      | `salam build FILE --platform=web`      |
| Salam: Serve (live reload) | Ctrl+Shift+F5 | `salam run FILE --platform=web`       |
| Salam: Run project         |              | `salam run` in the file's folder       |
| Salam: New project         |              | `salam init` in the file's folder      |

Save the file first; the commands run on the file on disk. Format rewrites the
file and Notepad++ offers to reload it. `salam` must be on the `PATH`. If a
shortcut clashes with another one, change it in `Settings > Shortcut Mapper >
Run commands`.

In a project made by `salam init`, `Salam: Run project` serves the web page with
live reload (or starts the first platform in `salam.toml`).

### NppExec alternative

If you have the NppExec plugin, `nppexec/salam.nppexec` holds the same commands
as scripts that save the file first and print into the NppExec console. Paste
each script into `Plugins > NppExec > Execute NppExec Script` and save it under
the name in its heading. In `Plugins > NppExec > Console Output Filters`, set the
highlight mask to `%FILE%:%LINE%:*` and a double click on an error line
jumps to the source.

## Colours and the dark theme

`salam.xml` carries colours for a white background. For a dark Notepad++ theme
choose `Language > Salam Dark`, which has the same keyword lists with light
colours on a dark background. Only `salam.xml` claims the `.salam` extension, so
files open as the light variant until you switch; to make the dark one the
default, move the `ext="salam"` attribute from one file to the other. The
styles can also be edited in `Language > User Defined Language > Define your
language...`.

## What it covers

Keywords, types, built-in output statements, constants, intrinsic types and
intrinsic methods in English and Persian; `//` and `/* */` comments
with folding; `"..."`, `'c'` and `` `...` `` literals; hex, binary, octal and
decimal numbers; and the operator set.

Enable autocompletion in `Settings > Preferences > Auto-Completion` (function
completion and word completion); the keyword list comes from `Salam.xml`.

## Generated

`salam.xml`, `salam-dark.xml` and `autoCompletion/Salam.xml` are generated. Edit
the generator and rerun it from the repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

The function list rules, the shortcuts and the NppExec scripts are written by
hand.

## License

MIT. See [LICENSE](LICENSE).

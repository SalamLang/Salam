# Salam for Visual Studio Code

Language support for [Salam](https://github.com/SalamLang/Salam): syntax highlighting, snippets, and commands for building and running `.salam` files.

## Features

- **Syntax highlighting** for `.salam`, covering all three keyword packs (English, Persian, Arabic)
- **Snippets** for the constructs you type most
- **Run and build commands** that shell out to the `salam` compiler
- **Language configuration**: comment toggling, bracket matching, auto-closing pairs

## Requirements

The extension calls the `salam` compiler for its run and build commands. Install it from [the releases page](https://github.com/SalamLang/Salam/releases), or with:

```sh
# Linux / macOS
curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/main/install.sh | sh

# Windows (PowerShell)
irm https://raw.githubusercontent.com/SalamLang/Salam/main/install.ps1 | iex
```

If `salam` is not on your `PATH`, set `salam.compilerPath` to its absolute location.

## Settings

| Setting              | Default | Description                                                |
| -------------------- | ------- | ---------------------------------------------------------- |
| `salam.compilerPath` | `salam` | Path to the `salam` executable                             |
| `salam.languagePack` | `auto`  | Keyword pack passed as `--lang` (`auto`, `en`, `fa`, `ar`) |
| `salam.run.command`  | `run`   | Subcommand used by the run command                         |

## The grammar is generated

`syntaxes/salam.tmLanguage.json` is not edited by hand. It is produced from the compiler's own keyword and type tables, so it cannot drift from the language:

```sh
salam exec scripts/gen_grammar.salam > syntaxes/salam.tmLanguage.json
```

Change `scripts/gen_grammar.salam` and regenerate; a direct edit to the JSON will be overwritten.

## Licence

**This directory is MIT licensed.** See [LICENSE](LICENSE).

The rest of the repository is copyleft; see the licence file at the repository root. This directory is deliberately the exception: editor tooling, and the TextMate grammar above all, is only useful if other people can vendor it. [GitHub Linguist](https://github.com/github-linguist/linguist) and similar projects accept grammar dependencies only under permissive licences, and nothing that merely embeds a grammar should take on a copyleft obligation to do so.

Everything the extension ships is MIT and carries no copyleft dependency:

| Shipped file                                           | Notes                            |
| ------------------------------------------------------ | -------------------------------- |
| `syntaxes/salam.tmLanguage.json`                       | the grammar; plain data, no code |
| `language-configuration.json`, `snippets/`, `icons/`   | editor data                      |
| `README.md`, `CHANGELOG.md`, `LICENSE`, `package.json` | metadata                         |

One thing worth stating plainly rather than leaving implied: `src/` and `scripts/` are written in Salam and import the standard library, which is copyleft. Both are excluded from the published extension by [`.vscodeignore`](.vscodeignore), so nothing derived from it is distributed in the `.vsix`. If you build them yourself, the result combines this MIT code with that library and the combination is governed accordingly.

So a project that wants the grammar, the snippets or the language configuration can take them under MIT with nothing further to consider. That is the case Linguist and other consumers care about.

## Notes for grammar consumers

Salam identifiers may contain Arabic and Persian letters, so the patterns use `\x{HHHH}` escapes over the Arabic, Arabic Supplement, Arabic Extended-A and Arabic Presentation Forms ranges, plus ZWNJ and ZWJ. That is Oniguruma syntax, which is what `vscode-textmate` and GitHub use. Engines that spell codepoints `\u{HHHH}` instead, such as Ruby's Onigmo, need the escapes translated before the patterns will compile.

The scope name is `source.salam` and the only file extension is `.salam`.

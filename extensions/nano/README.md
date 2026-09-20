# Salam for GNU nano

## Install

```sh
mkdir -p ~/.nano
cp salam.nanorc ~/.nano/
echo 'include "~/.nano/salam.nanorc"' >> ~/.nanorc
```

System-wide instead:

```sh
sudo cp salam.nanorc /usr/share/nano/
```

nano loads everything in `/usr/share/nano/` by default.

## What it covers

Keywords, types, built-in output statements, constants and contextual
keywords in all three languages, numbers in ASCII, Persian and Arabic-Indic
digits, string, raw-string and character literals, `//` and `/* */` comments,
TODO markers, and trailing whitespace.

`^T` (nano 4.0 and later) runs `salam format` on the buffer, and the `linter`
directive wires `salam inspect` to nano's linter view.

## Generated

`salam.nanorc` is generated. Edit the generator and rerun it from the
repository root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).

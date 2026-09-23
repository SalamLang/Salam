# Salam for Notepad++

A User Defined Language (UDL 2.1) definition.

## Install

`Language > User Defined Language > Define your language... > Import...` and
pick `salam.xml`, then restart Notepad++.

Or copy it straight into the UDL folder:

```
%APPDATA%\Notepad++\userDefineLangs\salam.xml
```

`.salam` files then open as Salam.

## What it covers

Keywords, types, built-in output statements, constants, intrinsic types and
intrinsic methods in English and Persian; `//` and `/* */` comments
with folding; `"..."`, `'c'` and `` `...` `` literals; hex, binary, octal and
decimal numbers; and the operator set.

The colours target the default light theme. To change them, open
`Language > User Defined Language > Define your language...` and edit the
styles there.

## Generated

`salam.xml` is generated. Edit the generator and rerun it from the repository
root:

```sh
salam exec extensions/tools/gen_syntaxes.salam --timeout=180000
```

## License

MIT. See [LICENSE](LICENSE).

# Changelog

## 0.3.9

Grammar corrected against the compiler's own keyword and type tables. Each item below was checked by compiling a program that uses the construct, not by reading the source.

### Added

- `match` is highlighted as a control keyword. It is `TK_KW_MATCH` in the langpack and has been a language feature for some time, but nothing in the grammar matched it.
- `while` is highlighted again. The generator had stopped emitting it; the langpack still maps it to `TK_KW_UNTIL`, the same loop under a second spelling.
- The primitive types `int`, `uint`, `size`, `float` and `uchar`, none of which were highlighted before.
- `?` is highlighted as `keyword.operator.ternary.salam`.

### Fixed

- `^^` and `^^=` are highlighted as the power operators again. The generator emitted `**`, which the language does not accept: `2 ** 3` is rejected and `2 ^^ 3` compiles. The lexer treats `^` doubled as power.

### Removed

- `isize` and `usize` are no longer highlighted as primitive types. They are not Salam types, and the compiler rejects both with `E001: unknown type`. They exist only inside the LLVM backend.

### Licensing

- Added an explicit MIT `LICENSE` to this directory. The extension's `package.json` had always declared MIT, but with no licence file next to it the claim was ambiguous inside a GPL-3.0 repository.

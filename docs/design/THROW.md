# One keyword for failing functions (W234)

A function that can fail is marked with `throw` after its return type (or after
the parameters when there is none), and fails with the statement `throw`:

```salam
func parse port(text: str): int throw:
    if text == "":
        throw Error.New(ErrorKind.InvalidArgument, "empty port")
    end
    ret text.ToInt()
end
```

In Persian code both are `پرتاب خطا`:

```salam
تابع درگاه(متن: رشته): صحیح پرتاب خطا:
    اگر متن برابر "":
        پرتاب خطا "درگاه خالی است"
    پایان
    برگشت متن.به صحیح()
پایان
```

The signature form is accepted wherever it was accepted before: functions,
methods, interface methods and function types.

## The old spelling

`throws` (Persian `خطادار`) used to mark the signature. It is not a keyword
any more, so it can be used as a name. In a signature it is the error E237,
with a quick fix that writes `throw`:

```
error[E237]: 'throws' is no longer a keyword; write 'throw'
 --> a.salam:1:21
  |
1 | func f(x: int): int throws:
  |                     ^
  |
  = help: write 'throw' after the signature, for example 'func f(): int throw:'
  = fix: Write 'throw'
```

## History

1. Stage 1 (#3640, in 0.6.18): the parser accepted `throw` in a signature,
   `throws` warned with W234, and all examples, tests, docs and editor
   grammars moved to `throw`.
2. Stage 2: the CI seed moved to 0.6.18, the first release that reads `throw`
   in a signature. `std/` and the scripts the released compiler runs were
   converted, and the `throws` keyword was removed.

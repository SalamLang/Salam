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

`throws` (Persian `خطادار`) used to mark the signature. It still compiles and
means the same, but reports W234 with a quick fix that writes `throw`:

```
warning[W234]: 'throws' is deprecated, use 'throw'
```

`salam format` rewrites `throws` to `throw`. Files under `std/` and `compiler/`
do not report W234 until they are converted.

## Removal plan

1. This change: the parser accepts `throw` in a signature, `throws` warns, all
   examples, tests, docs and editor grammars use `throw`.
2. After the next release, once the seed compiler knows `throw`, `std/` and
   `compiler/` are converted and the `throws` keyword is removed.

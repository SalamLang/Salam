from lang import Lang
from value import Value

referrerpolicies = [
    Value().set_generate_name("no-referrer").add_text(Lang.EN, "no-referrer").add_text(Lang.FA, "بدون ارجاع"),
    Value().set_generate_name("no-referrer-when-downgrade").add_text(Lang.EN, "no-referrer-when-downgrade").add_text(Lang.FA, "منبع"),
    Value().set_generate_name("origin").add_text(Lang.EN, "origin").add_text(Lang.FA, "منبع"),
    Value().set_generate_name("origin-when-cross-origin").add_text(Lang.EN, "origin-when-cross-origin").add_text(Lang.FA, "منبع در صورت عبور از منبع"),
    Value().set_generate_name("same-origin").add_text(Lang.EN, "same-origin").add_text(Lang.FA, "منبع یکسان"),
    Value().set_generate_name("strict-origin-when-cross-origin").add_text(Lang.EN, "strict-origin-when-cross-origin").add_text(Lang.FA, "حساس به منبع"),
    Value().set_generate_name("unsafe-url").add_text(Lang.EN, "unsafe-url").add_text(Lang.FA, "آدرس ناامن"),
]

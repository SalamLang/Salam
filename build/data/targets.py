from lang import Lang
from value import Value

targets = [
    Value().set_generate_name("_blank").add_text(Lang.EN, "blank").add_text(Lang.FA, "جدید"),
    Value().set_generate_name("_self").add_text(Lang.EN, "self").add_text(Lang.FA, "خود"),
    Value().set_generate_name("_parent").add_text(Lang.EN, "parent").add_text(Lang.FA, "والدین"),
    Value().set_generate_name("_top").add_text(Lang.EN, "top").add_text(Lang.FA, "بالا"),
]

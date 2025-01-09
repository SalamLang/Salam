from lang import Lang
from value import Value

dirs = [
    Value().set_generate_name("ltr").add_text(Lang.EN, "ltr").add_text(Lang.EN, "left").add_text(Lang.FA, "چپ به راست").add_text(Lang.FA, "چپ"),
    Value().set_generate_name("rtl").add_text(Lang.EN, "rtl").add_text(Lang.EN, "right").add_text(Lang.FA, "راست به چپ").add_text(Lang.FA, "راست"),
]

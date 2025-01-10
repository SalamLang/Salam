import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

rels = [
    Value()
    .set_generate_name("alternate")
    .add_text(Lang.EN, "alternate")
    .add_text(Lang.FA, "متناوب"),
    Value()
    .set_generate_name("author")
    .add_text(Lang.EN, "author")
    .add_text(Lang.FA, "نویسنده"),
    Value()
    .set_generate_name("bookmark")
    .add_text(Lang.EN, "bookmark")
    .add_text(Lang.FA, "بوکمارک"),
    Value()
    .set_generate_name("external")
    .add_text(Lang.EN, "external")
    .add_text(Lang.FA, "خارجی"),
    Value()
    .set_generate_name("help")
    .add_text(Lang.EN, "help")
    .add_text(Lang.FA, "کمک"),
    Value()
    .set_generate_name("license")
    .add_text(Lang.EN, "license")
    .add_text(Lang.FA, "مجوز"),
    Value()
    .set_generate_name("next")
    .add_text(Lang.EN, "next")
    .add_text(Lang.FA, "بعدی"),
    Value()
    .set_generate_name("nofollow")
    .add_text(Lang.EN, "nofollow")
    .add_text(Lang.FA, "بدون دنبال"),
    Value()
    .set_generate_name("noreferrer")
    .add_text(Lang.EN, "noreferrer")
    .add_text(Lang.FA, "بدون ارجاع"),
    Value()
    .set_generate_name("noopener")
    .add_text(Lang.EN, "noopener")
    .add_text(Lang.FA, "بدون بازکردن"),
    Value()
    .set_generate_name("prev")
    .add_text(Lang.EN, "prev")
    .add_text(Lang.FA, "قبلی"),
    Value()
    .set_generate_name("search")
    .add_text(Lang.EN, "search")
    .add_text(Lang.FA, "جستجو"),
    Value()
    .set_generate_name("tag")
    .add_text(Lang.EN, "tag")
    .add_text(Lang.FA, "برچسب"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string([value.to_dict() for value in rels])
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

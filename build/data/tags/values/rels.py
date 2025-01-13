import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Correctly accessing language codes from Lang.languages
rels = [
    Value()
    .set_generate_name("alternate")
    .add_text(Lang.languages["EN"]["code"], "alternate")
    .add_text(Lang.languages["FA"]["code"], "متناوب"),
    Value()
    .set_generate_name("author")
    .add_text(Lang.languages["EN"]["code"], "author")
    .add_text(Lang.languages["FA"]["code"], "نویسنده"),
    Value()
    .set_generate_name("bookmark")
    .add_text(Lang.languages["EN"]["code"], "bookmark")
    .add_text(Lang.languages["FA"]["code"], "بوکمارک"),
    Value()
    .set_generate_name("external")
    .add_text(Lang.languages["EN"]["code"], "external")
    .add_text(Lang.languages["FA"]["code"], "خارجی"),
    Value()
    .set_generate_name("help")
    .add_text(Lang.languages["EN"]["code"], "help")
    .add_text(Lang.languages["FA"]["code"], "کمک"),
    Value()
    .set_generate_name("license")
    .add_text(Lang.languages["EN"]["code"], "license")
    .add_text(Lang.languages["FA"]["code"], "مجوز"),
    Value()
    .set_generate_name("next")
    .add_text(Lang.languages["EN"]["code"], "next")
    .add_text(Lang.languages["FA"]["code"], "بعدی"),
    Value()
    .set_generate_name("nofollow")
    .add_text(Lang.languages["EN"]["code"], "nofollow")
    .add_text(Lang.languages["FA"]["code"], "بدون دنبال"),
    Value()
    .set_generate_name("noreferrer")
    .add_text(Lang.languages["EN"]["code"], "noreferrer")
    .add_text(Lang.languages["FA"]["code"], "بدون ارجاع"),
    Value()
    .set_generate_name("noopener")
    .add_text(Lang.languages["EN"]["code"], "noopener")
    .add_text(Lang.languages["FA"]["code"], "بدون بازکردن"),
    Value()
    .set_generate_name("prev")
    .add_text(Lang.languages["EN"]["code"], "prev")
    .add_text(Lang.languages["FA"]["code"], "قبلی"),
    Value()
    .set_generate_name("search")
    .add_text(Lang.languages["EN"]["code"], "search")
    .add_text(Lang.languages["FA"]["code"], "جستجو"),
    Value()
    .set_generate_name("tag")
    .add_text(Lang.languages["EN"]["code"], "tag")
    .add_text(Lang.languages["FA"]["code"], "برچسب"),
]

if __name__ == "__main__":
    # Correct the structure for Prebuild.to_string()
    rels_dict = {
        value.generate_name: value.to_dict() for value in rels if value.generate_name is not None
    }

    # Generate the string output
    strings = Prebuild.to_string(rels_dict)

    # Print and save the output
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

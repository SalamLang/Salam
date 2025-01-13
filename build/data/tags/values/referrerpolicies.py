import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Correcting the language access and constructing the list of referrer policies
referrerpolicies = [
    Value()
    .set_generate_name("no-referrer")
    .add_text(Lang.languages["EN"]["code"], "no-referrer")
    .add_text(Lang.languages["FA"]["code"], "بدون ارجاع"),
    Value()
    .set_generate_name("no-referrer-when-downgrade")
    .add_text(Lang.languages["EN"]["code"], "no-referrer-when-downgrade")
    .add_text(Lang.languages["FA"]["code"], "منبع"),
    Value()
    .set_generate_name("origin")
    .add_text(Lang.languages["EN"]["code"], "origin")
    .add_text(Lang.languages["FA"]["code"], "منبع"),
    Value()
    .set_generate_name("origin-when-cross-origin")
    .add_text(Lang.languages["EN"]["code"], "origin-when-cross-origin")
    .add_text(Lang.languages["FA"]["code"], "منبع در صورت عبور از منبع"),
    Value()
    .set_generate_name("same-origin")
    .add_text(Lang.languages["EN"]["code"], "same-origin")
    .add_text(Lang.languages["FA"]["code"], "منبع یکسان"),
    Value()
    .set_generate_name("strict-origin-when-cross-origin")
    .add_text(Lang.languages["EN"]["code"], "strict-origin-when-cross-origin")
    .add_text(Lang.languages["FA"]["code"], "حساس به منبع"),
    Value()
    .set_generate_name("unsafe-url")
    .add_text(Lang.languages["EN"]["code"], "unsafe-url")
    .add_text(Lang.languages["FA"]["code"], "آدرس ناامن"),
]

if __name__ == "__main__":
    # Creating a dictionary for the Prebuild.to_string() method
    referrerpolicies_dict = {
        value.generate_name: value.to_dict() for value in referrerpolicies if value.generate_name is not None
    }

    # Generate the string output
    strings = Prebuild.to_string(referrerpolicies_dict)

    # Print and save the output
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

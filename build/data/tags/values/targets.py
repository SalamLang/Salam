import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

targets = [
    Value()
    .set_generate_name("_blank")
    .add_text(Lang.languages["EN"]["code"], "blank")
    .add_text(Lang.languages["FA"]["code"], "جدید"),
    Value()
    .set_generate_name("_self")
    .add_text(Lang.languages["EN"]["code"], "self")
    .add_text(Lang.languages["FA"]["code"], "خود"),
    Value()
    .set_generate_name("_parent")
    .add_text(Lang.languages["EN"]["code"], "parent")
    .add_text(Lang.languages["FA"]["code"], "والدین"),
    Value()
    .set_generate_name("_top")
    .add_text(Lang.languages["EN"]["code"], "top")
    .add_text(Lang.languages["FA"]["code"], "بالا"),
]

if __name__ == "__main__":
    # Filter out values with None as generate_name
    strings = Prebuild.to_string(
        {
            value.generate_name: value.to_dict()
            for value in targets
            if value.generate_name is not None
        }
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

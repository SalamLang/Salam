import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

targets = [
    Value().set_generate_name("_blank").add_text(Lang.EN, "blank").add_text(Lang.FA, "جدید"),
    Value().set_generate_name("_self").add_text(Lang.EN, "self").add_text(Lang.FA, "خود"),
    Value().set_generate_name("_parent").add_text(Lang.EN, "parent").add_text(Lang.FA, "والدین"),
    Value().set_generate_name("_top").add_text(Lang.EN, "top").add_text(Lang.FA, "بالا"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in targets]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

methods = [
    Value().set_generate_name("get").add_text(Lang.EN, "get").add_text(Lang.FA, "گت"),
    Value().set_generate_name("post").add_text(Lang.EN, "post").add_text(Lang.FA, "پست"),
]

if __name__ == '__main__':
    strings = Prebuild.to_string(
        [value.to_dict() for value in methods]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Fixing the language access
methods = [
    Value()
    .set_generate_name("get")
    .add_text(Lang.languages["EN"]["code"], "get")
    .add_text(Lang.languages["FA"]["code"], "گت"),
    Value()
    .set_generate_name("post")
    .add_text(Lang.languages["EN"]["code"], "post")
    .add_text(Lang.languages["FA"]["code"], "پست"),
]

if __name__ == "__main__":
    # Creating a dictionary for Prebuild.to_string() method
    methods_dict = {
        value.generate_name: value.to_dict() for value in methods if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(methods_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

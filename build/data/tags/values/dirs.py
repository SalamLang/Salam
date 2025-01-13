import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Fixing language access and ensuring correct dictionary format for to_string()
dirs = [
    Value()
    .set_generate_name("ltr")
    .add_text(Lang.languages["EN"]["code"], "ltr")
    .add_text(Lang.languages["EN"]["code"], "left")
    .add_text(Lang.languages["FA"]["code"], "چپ به راست")
    .add_text(Lang.languages["FA"]["code"], "چپ"),
    Value()
    .set_generate_name("rtl")
    .add_text(Lang.languages["EN"]["code"], "rtl")
    .add_text(Lang.languages["EN"]["code"], "right")
    .add_text(Lang.languages["FA"]["code"], "راست به چپ")
    .add_text(Lang.languages["FA"]["code"], "راست"),
]

if __name__ == "__main__":
    # Creating a dictionary for Prebuild.to_string() method
    dirs_dict = {
        value.generate_name: value.to_dict()
        for value in dirs
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(dirs_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Fixing language access and ensuring correct dictionary format for to_string()
crossorigins = [
    Value()
    .set_generate_name("anonymous")
    .add_text(Lang.languages["EN"]["code"], "anonymous")
    .add_text(Lang.languages["FA"]["code"], "ناشناس"),
    Value()
    .set_generate_name("use-credentials")
    .add_text(Lang.languages["EN"]["code"], "use credentials")
    .add_text(Lang.languages["FA"]["code"], "یوز کرید"),
]

if __name__ == "__main__":
    # Creating a dictionary for Prebuild.to_string() method
    crossorigins_dict = {
        value.generate_name: value.to_dict() for value in crossorigins if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(crossorigins_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

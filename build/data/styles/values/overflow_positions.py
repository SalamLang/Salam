import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
overflow_positions = [
    Value()
    .set_generate_name("safe")
    .add_text(Lang.languages["EN"]["code"], "safe")
    .add_text(Lang.languages["FA"]["code"], "امن"),
    Value()
    .set_generate_name("unsafe")
    .add_text(Lang.languages["EN"]["code"], "unsafe")
    .add_text(Lang.languages["FA"]["code"], "نامناسب"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    overflow_positions_dict = {
        value.generate_name: value.to_dict() for value in overflow_positions if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(overflow_positions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
single_animation_directions = [
    Value()
    .set_generate_name("normal")
    .add_text(Lang.languages["EN"]["code"], "normal")
    .add_text(Lang.languages["FA"]["code"], "عادی"),
    Value()
    .set_generate_name("reverse")
    .add_text(Lang.languages["EN"]["code"], "reverse")
    .add_text(Lang.languages["FA"]["code"], "برعکس"),
    Value()
    .set_generate_name("alternate")
    .add_text(Lang.languages["EN"]["code"], "alternate")
    .add_text(Lang.languages["FA"]["code"], "متناوب"),
    Value()
    .set_generate_name("alternate-reverse")
    .add_text(Lang.languages["EN"]["code"], "alternate-reverse")
    .add_text(Lang.languages["FA"]["code"], "متناوب برعکس"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    single_animation_directions_dict = {
        value.generate_name: value.to_dict()
        for value in single_animation_directions
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(single_animation_directions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
self_positions = [
    Value()
    .set_generate_name("center")
    .add_text(Lang.languages["EN"]["code"], "center")
    .add_text(Lang.languages["FA"]["code"], "وسط"),
    Value()
    .set_generate_name("start")
    .add_text(Lang.languages["EN"]["code"], "start")
    .add_text(Lang.languages["FA"]["code"], "شروع"),
    Value()
    .set_generate_name("end")
    .add_text(Lang.languages["EN"]["code"], "end")
    .add_text(Lang.languages["FA"]["code"], "پایان"),
    Value()
    .set_generate_name("self-start")
    .add_text(Lang.languages["EN"]["code"], "self-start")
    .add_text(Lang.languages["FA"]["code"], "شروع خود"),
    Value()
    .set_generate_name("self-end")
    .add_text(Lang.languages["EN"]["code"], "self-end")
    .add_text(Lang.languages["FA"]["code"], "پایان خود"),
    Value()
    .set_generate_name("flex-start")
    .add_text(Lang.languages["EN"]["code"], "flex-start")
    .add_text(Lang.languages["FA"]["code"], "شروع فلکس"),
    Value()
    .set_generate_name("flex-end")
    .add_text(Lang.languages["EN"]["code"], "flex-end")
    .add_text(Lang.languages["FA"]["code"], "پایان فلکس"),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    self_positions_dict = {
        value.generate_name: value.to_dict()
        for value in self_positions
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(self_positions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/align-items
# <self-position> =
#   center      |
#   start       |
#   end         |
#   self-start  |
#   self-end    |
#   flex-start  |
#   flex-end

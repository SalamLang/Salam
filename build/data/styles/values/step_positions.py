import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Fixing language access and ensuring correct dictionary format for to_string()
single_animation_directions = [
    Value()
    .set_generate_name("jump-start")
    .add_text(Lang.languages["EN"]["code"], "jump-start")
    .add_text(Lang.languages["FA"]["code"], "پرش-شروع"),
    Value()
    .set_generate_name("jump-end")
    .add_text(Lang.languages["EN"]["code"], "jump-end")
    .add_text(Lang.languages["FA"]["code"], "پرش-پایان"),
    Value()
    .set_generate_name("jump-none")
    .add_text(Lang.languages["EN"]["code"], "jump-none")
    .add_text(Lang.languages["FA"]["code"], "بدون-پرش"),
    Value()
    .set_generate_name("jump-both")
    .add_text(Lang.languages["EN"]["code"], "jump-both")
    .add_text(Lang.languages["FA"]["code"], "پرش-هردو"),
    Value()
    .set_generate_name("start")
    .add_text(Lang.languages["EN"]["code"], "start")
    .add_text(Lang.languages["FA"]["code"], "شروع"),
    Value()
    .set_generate_name("end")
    .add_text(Lang.languages["EN"]["code"], "end")
    .add_text(Lang.languages["FA"]["code"], "پایان"),
]

if __name__ == "__main__":
    # Creating a dictionary for Prebuild.to_string() method
    single_animation_directions_dict = {
        value.generate_name: value.to_dict() for value in single_animation_directions if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(single_animation_directions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <step-position> = 
#   jump-start  |
#   jump-end    |
#   jump-none   |
#   jump-both   |
#   start       |
#   end         

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

single_animation_directions = [
    Value().set_generate_name("jump-start").add_text(Lang.EN, "jump-start").add_text(Lang.FA, "پرش-شروع"),
    Value().set_generate_name("jump-end").add_text(Lang.EN, "jump-end").add_text(Lang.FA, "پرش-پایان"),
    Value().set_generate_name("jump-none").add_text(Lang.EN, "jump-none").add_text(Lang.FA, "بدون-پرش"),
    Value().set_generate_name("jump-both").add_text(Lang.EN, "jump-both").add_text(Lang.FA, "پرش-هردو"),
    Value().set_generate_name("start").add_text(Lang.EN, "start").add_text(Lang.FA, "شروع"),
    Value().set_generate_name("end").add_text(Lang.EN, "end").add_text(Lang.FA, "پایان"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in single_animation_directions]
    )
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

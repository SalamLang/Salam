import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

single_animation_directions = [
    Value()
    .set_generate_name("normal")
    .add_text(Lang.EN, "normal")
    .add_text(Lang.FA, "عادی"),
    Value()
    .set_generate_name("reverse")
    .add_text(Lang.EN, "reverse")
    .add_text(Lang.FA, "برعکس"),
    Value()
    .set_generate_name("alternate")
    .add_text(Lang.EN, "alternate")
    .add_text(Lang.FA, "متناوب"),
    Value()
    .set_generate_name("alternate-reverse")
    .add_text(Lang.EN, "alternate-reverse")
    .add_text(Lang.FA, "متناوب برعکس"),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in single_animation_directions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-fill-mode
# <single-animation-fill-mode> =
#   none       |
#   forwards   |
#   backwards  |
#   both

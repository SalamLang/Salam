import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Fixing language access and ensuring correct dictionary format for to_string()
step_easing_functions = [
    Value()
    .set_generate_name("step-start")
    .add_text(Lang.languages["EN"]["code"], "step-start")
    .add_text(Lang.languages["FA"]["code"], "پرش-شروع"),
    Value()
    .set_generate_name("step-end")
    .add_text(Lang.languages["EN"]["code"], "step-end")
    .add_text(Lang.languages["FA"]["code"], "پرش-پایان"),
    Value()
    .set_generate_name("steps")
    .add_text(Lang.languages["EN"]["code"], "steps")
    .add_text(Lang.languages["FA"]["code"], "پرش")
    .set_is_function(True),
]

if __name__ == "__main__":
    # Creating a dictionary for Prebuild.to_string() method
    step_easing_functions_dict = {
        value.generate_name: value.to_dict()
        for value in step_easing_functions
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(step_easing_functions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <step-easing-function> =
#   step-start  |
#   step-end    |
#   <steps()>

# <steps()> =
#   steps( <integer> , <step-position>? )

import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
linear_easing_functions = [
    Value()
    .set_generate_name("linear")
    .add_text(Lang.languages["EN"]["code"], "linear")
    .add_text(Lang.languages["FA"]["code"], "خطی"),
    Value()
    .set_generate_name("linear")
    .add_text(Lang.languages["EN"]["code"], "linear")
    .add_text(Lang.languages["FA"]["code"], "خطی")
    .set_is_function(True),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    linear_easing_functions_dict = {
        value.generate_name: value.to_dict()
        for value in linear_easing_functions
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(linear_easing_functions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <linear-easing-function> =
#   linear      |
#   <linear()>

# <linear()> =
#   linear( [ <number> && <percentage>{0,2} ]# )

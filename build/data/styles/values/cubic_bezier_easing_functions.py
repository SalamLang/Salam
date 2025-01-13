import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

# Corrected access to language codes and Prebuild.to_string() argument type
cubic_bezier_easing_functions = [
    Value()
    .set_generate_name("ease")
    .add_text(Lang.languages["EN"]["code"], "ease")
    .add_text(Lang.languages["FA"]["code"], "آسان"),
    Value()
    .set_generate_name("ease-in")
    .add_text(Lang.languages["EN"]["code"], "ease-in")
    .add_text(Lang.languages["FA"]["code"], "آسان-ورودی"),
    Value()
    .set_generate_name("ease-out")
    .add_text(Lang.languages["EN"]["code"], "ease-out")
    .add_text(Lang.languages["FA"]["code"], "آسان-خروجی"),
    Value()
    .set_generate_name("ease-in-out")
    .add_text(Lang.languages["EN"]["code"], "ease-in-out")
    .add_text(Lang.languages["FA"]["code"], "آسان-ورودی-خروجی"),
    Value()
    .set_generate_name("cubic-bezier")
    .add_text(Lang.languages["EN"]["code"], "cubic-bezier")
    .add_text(Lang.languages["FA"]["code"], "بیزیر-مکعب")
    .set_is_function(True),
]

if __name__ == "__main__":
    # Creating a dictionary with generate_name as key and value.to_dict() as value
    cubic_bezier_easing_functions_dict = {
        value.generate_name: value.to_dict()
        for value in cubic_bezier_easing_functions
        if value.generate_name is not None
    }

    # Generating the string output
    strings = Prebuild.to_string(cubic_bezier_easing_functions_dict)
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)


# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <cubic-bezier-easing-function> =
#   ease              |
#   ease-in           |
#   ease-out          |
#   ease-in-out       |
#   <cubic-bezier()>

# <cubic-bezier()> =
#   cubic-bezier( [ <number [0,1]> , <number> ]#{2} )

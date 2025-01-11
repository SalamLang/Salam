import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

linear_easing_functions = [
    Value().set_generate_name("linear").add_text(Lang.EN, "linear").add_text(Lang.FA, "خطی"),
    Value().set_generate_name("linear").add_text(Lang.EN, "linear").add_text(Lang.FA, "خطی").set_is_function(True),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in linear_easing_functions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <linear-easing-function> = 
#   linear      |
#   <linear()>  

# <linear()> = 
#   linear( [ <number> && <percentage>{0,2} ]# )  

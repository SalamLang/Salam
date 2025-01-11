import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

cubic_bezier_easing_functions = [
    Value().set_generate_name("ease").add_text(Lang.EN, "ease").add_text(Lang.FA, "آسان"),
    Value().set_generate_name("ease-in").add_text(Lang.EN, "ease-in").add_text(Lang.FA, "آسان-ورودی"),
    Value().set_generate_name("ease-out").add_text(Lang.EN, "ease-out").add_text(Lang.FA, "آسان-خروجی"),
    Value().set_generate_name("ease-in-out").add_text(Lang.EN, "ease-in-out").add_text(Lang.FA, "آسان-ورودی-خروجی"),
    Value().set_generate_name("cubic-bezier").add_text(Lang.EN, "cubic-bezier").add_text(Lang.FA, "بیزیر-مکعب").set_is_function(True),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in cubic_bezier_easing_functions]
    )
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

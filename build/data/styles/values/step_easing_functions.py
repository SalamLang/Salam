import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

from lang import Lang
from value import Value
from prebuild import Prebuild

step_easing_functions = [
    Value().set_generate_name("step-start").add_text(Lang.EN, "step-start").add_text(Lang.FA, "پرش-شروع"),
    Value().set_generate_name("step-end").add_text(Lang.EN, "step-end").add_text(Lang.FA, "پرش-پایان"),
    Value().set_generate_name("steps").add_text(Lang.EN, "steps").add_text(Lang.FA, "پرش").set_is_function(True),
]

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in step_easing_functions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <step-easing-function> = 
#   step-start  |
#   step-end    |
#   <steps()>   

# <steps()> = 
#   steps( <integer> , <step-position>? )  

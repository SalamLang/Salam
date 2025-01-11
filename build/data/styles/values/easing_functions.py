import sys
from pathlib import Path

parent_dir = Path(__file__).resolve().parent.parent.parent.parent
sys.path.append(str(parent_dir))

import step_easing_functions
import linear_easing_functions
import cubic_bezier_easing_functions
from prebuild import Prebuild

easing_functions = linear_easing_functions.linear_easing_functions
easing_functions += cubic_bezier_easing_functions.cubic_bezier_easing_functions
easing_functions += step_easing_functions.step_easing_functions

if __name__ == "__main__":
    strings = Prebuild.to_string(
        [value.to_dict() for value in easing_functions]
    )
    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <easing-function> = 
#   <linear-easing-function>        |
#   <cubic-bezier-easing-function>  |
#   <step-easing-function>          

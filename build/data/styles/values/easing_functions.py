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
    # Wrap the list of dictionaries in a dictionary (adjust the key as needed)
    easing_functions_dict = {
        "easing_functions": [value.to_dict() for value in easing_functions]
    }

    # Now pass the dictionary to to_string
    strings = Prebuild.to_string(easing_functions_dict)

    Prebuild.print(strings)
    Prebuild.save(strings, __file__)

# https://developer.mozilla.org/en-US/docs/Web/CSS/animation-timing-function
# <easing-function> =
#   <linear-easing-function>        |
#   <cubic-bezier-easing-function>  |
#   <step-easing-function>
